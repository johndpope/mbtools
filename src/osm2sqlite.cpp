#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include <iomanip>

#include <MapFile.h>

#include <OsmRuleParser.h>
#include <OsmDocument.h>

#include <spatialite.h>

using namespace std ;


struct Rule {
   OSM::Rule::ExpressionNode *condition_ ;
   OSM::Rule::Command *actions_ ;
};

struct Layer {
    string name_ ;
    string geom_ ;
    string srid_ ;
    vector<string> tags_, types_ ;
    vector<Rule> rules_ ;
};

struct MapConfig {
   vector<Layer> layers_ ;
   Dictionary pragmas_, resources_ ;
};

struct Action {
    string column_ ;
    OSM::Rule::Literal val_ ;
};

struct NodeRuleMap {
    int node_idx_ ;
    vector<int> matched_rules_ ;
};



static void split_line(const string &src, vector<string> &tokens) {

    using boost::tokenizer;
    using boost::escaped_list_separator;

    typedef tokenizer<escaped_list_separator<char> > stokenizer;

    stokenizer tok(src, escaped_list_separator<char>("\\", " ,\t",   "\"'"));

    for( stokenizer::iterator beg = tok.begin(); beg!=tok.end(); ++beg)
        if ( !beg->empty() ) tokens.push_back(*beg) ;
}

static void split_at_colon(const string &src, string &arg1, string &arg2, const string &def = string())
{
    vector<string> sub_tokens ;
    boost::split( sub_tokens, src , boost::is_any_of(" :"), boost::token_compress_on);

    arg1 = sub_tokens[0] ;
    if ( sub_tokens.size() == 2 ) arg2 = sub_tokens[1]  ;
    else arg2 = def ;
}

bool parseConfigFile(const string &fileName, MapConfig &cfg)
{
    ifstream strm(fileName.c_str()) ;
    if ( !strm ) {
        cerr << "Cannot open config file: " << fileName << endl ;
        return false ;
    }

    int count = 0 ;
    while ( !strm.eof() )
    {
        string line ;
        getline(strm, line) ;

        ++count ;

        if ( line.empty() || line.at(0)== '#' ) continue ;
        else if ( line.at(0) == '@')
        {
            vector<string> tokens ;

            split_line(line, tokens) ;

            if ( tokens[0] == "@layer") {

                if ( tokens.size() < 3 )
                {
                    cerr << "Error parsing layer definition in " << fileName << " not enough arguments (line: " << count << ")" ;
                    return false ;
                }

                // parse layer info

                Layer layer ;

                layer.name_ = boost::trim_copy(tokens[1]) ; // name

                // geometry type (points, lines, polygons) and srid

                split_at_colon(tokens[2], layer.geom_, layer.srid_, "4326") ;

                // parse columns (in the form <column_name>:<column_type>)

                for(int j=3 ; j<tokens.size() ; j++ )
                {
                    string tag, type ;
                    split_at_colon(tokens[j], tag, type, "text") ;

                    layer.tags_.push_back(tag) ;
                    layer.types_.push_back(type) ;
                }

                cfg.layers_.push_back(layer) ;

            }
            else if ( tokens[0] == "@pragma" )
            {
                if ( tokens.size() < 3 )
                {
                    cerr << "Error parsing pragma declaration in " << fileName << " not enough arguments (line: " << count << ")" ;
                    return false ;
                }

                string key = tokens[1] ;
                string val = tokens[2] ;

                cfg.pragmas_.add(key, val) ;
            }
            else if ( tokens[0] == "@resource" )
            {
                if ( tokens.size() < 2 )
                {
                    cerr << "Error parsing resource declaration in " << fileName << "(line: " << count << ")" ;
                    return false ;
                }

                string rpath = tokens[1] ;
                string prefix ;

                if ( tokens.size() == 3 ) prefix = tokens[2] ;

                cfg.resources_.add(rpath, prefix) ;

            }


        }
        else // rules
        {
            if ( cfg.layers_.empty() )
            {
                cerr << "Error parsing " << fileName << " :rule found before any layer definition (line: " << count << ")" ;
                return false ;
            }

            Layer &layer = cfg.layers_.back() ;

            istringstream sstrm(line) ;
            OSM::Rule::Parser parser(sstrm) ;

            if ( ! parser.parse() )
            {
                cerr << "Error parsing " << fileName << " : error in rule (line: " << count << ")" << endl ;
                cerr << parser.errorString << endl ;

                return false ;
            }

            Rule rule ;

            rule.condition_ = parser.node ;
            rule.actions_ = parser.actions ;

            layer.rules_.push_back(rule) ;
        }

    }

    return true ;
}

bool processSetTagActions(const Rule &r, OSM::Rule::Context &ctx, OSM::Feature *node)
{
    OSM::Rule::Command *action = r.actions_ ;

    bool cont = false ;

    while ( action )
    {
        if ( action->cmd_ ==  OSM::Rule::Command::Add )
        {
            node->tags.add(action->tag_, action->expression_->eval(ctx).toString()) ;
        }
        else if ( action->cmd_ == OSM::Rule::Command::Set )
        {
            if ( node->tags.contains(action->tag_) )
                node->tags[action->tag_] = action->expression_->eval(ctx).toString() ;
            else
                node->tags.add(action->tag_, action->expression_->eval(ctx).toString()) ;
        }
        else if ( action->cmd_ == OSM::Rule::Command::Continue )
        {
            cont = true ;
        }
        else if ( action->cmd_ == OSM::Rule::Command::Delete )
        {
            node->tags.remove(action->tag_) ;
        }

        action = action->next_ ;

    }

    return cont ;

}

bool processStoreActions(const Rule &r, OSM::Rule::Context &ctx, OSM::Feature *node, vector<Action> &actions)
{
    OSM::Rule::Command *action = r.actions_ ;

    bool cont = false ;

    while ( action )
    {
        if ( action->cmd_ == OSM::Rule::Command::Continue )
        {
            cont = true ;
        }
        else if ( action->cmd_ == OSM::Rule::Command::Store )
        {
            Action act ;

            act.column_ = action->tag_ ;
            act.val_ = action->expression_->eval(ctx) ;

            actions.push_back(act) ;


        }

        action = action->next_ ;

    }

    return cont ;

}

void bindColumns(const vector<Action> &actions, const Layer &layer, SQLite::Command &cmd, SQLite::Command &cmd_dict)
{
    bool is_poi_layer = ( layer.geom_ == "pois" ) ;

    for( int i=0 ; i<actions.size() ; i++ )
    {
        const Action &act = actions[i] ;

        if ( is_poi_layer && act.column_ == "content") continue ;

        const OSM::Rule::Literal &res = act.val_ ;
        int column_idx = std::find(layer.tags_.begin(), layer.tags_.end(), act.column_) - layer.tags_.begin();

        if ( res.isNull() )
            cmd.bind(column_idx + 2, SQLite::Nil) ;
        else if ( layer.types_[column_idx] == "real" )
            cmd.bind(column_idx + 2, res.toNumber()) ;
        else if ( layer.types_[column_idx] == "text" )
            cmd.bind(column_idx + 2, res.toString()) ;
        else if ( layer.types_[column_idx] == "integer" )
            cmd.bind(column_idx + 2, (int)res.toNumber()) ;
        else if ( layer.types_[column_idx] == "dict" )
        {
            cmd_dict.bind(1, res.toString()) ;
            cmd_dict.exec() ;

            cmd_dict.clear() ;

            cmd.bind(column_idx + 2, res.toString()) ;
        }
    }
}

bool addOSMLayerPoints(MapFile &mf, OSM::Document &doc, const Layer &layer,
                         const vector<NodeRuleMap > &node_idxs)
{
    SQLite::Database &db = mf.handle() ;

    SQLite::Session session(&db) ;
    SQLite::Connection &con = session.handle() ;

    unsigned char *blob;
    int blob_size;

    OSM::Rule::Context ctx(&doc) ;

    if ( layer.geom_ != "points" ) return false ;

    SQLite::Transaction trans(con) ;

    string geoCmd = "Transform(?," + layer.srid_ + ")" ;
    SQLite::Command cmd(con, mf.insertFeatureSQL(layer.tags_, layer.types_,  layer.name_, geoCmd)) ;

    string sql = "INSERT OR IGNORE INTO " + mf.dictionary_table_name_ + " (key) VALUES (?)" ;
    SQLite::Command cmd_dict(con, sql) ;

    for(int i=0 ; i<node_idxs.size() ; i++ )
    {
        vector<Action> actions ;

        const NodeRuleMap &nr = node_idxs[i] ;

        int node_idx = nr.node_idx_ ;
        OSM::Node &node = doc.nodes[node_idx] ;

        ctx.set(&node) ;

        for(int j=0 ; j<nr.matched_rules_.size() ; j++ )
        {
            int rule_idx = nr.matched_rules_[j] ;

            const Rule &r = layer.rules_[rule_idx] ;

            if ( ! processStoreActions(r, ctx, &node, actions) ) break ;
        }

        cmd.clear() ;

        bindColumns(actions, layer, cmd, cmd_dict) ;

        gaiaGeomCollPtr geo_pt = gaiaAllocGeomColl();

        geo_pt->Srid = 4326;

        gaiaAddPointToGeomColl (geo_pt, node.lon, node.lat);

        gaiaToSpatiaLiteBlobWkb (geo_pt, &blob, &blob_size);

        gaiaFreeGeomColl (geo_pt);

        cmd.bind(1, blob, blob_size) ;

        cmd.exec() ;
        cmd.clear() ;
        free(blob);

    }

    trans.commit() ;

    return true ;
}


bool addOSMLayerPOIs(MapFile &mf, OSM::Document &doc, const Layer &layer,
                         const vector<pair<int, int> > &node_idxs)
{
    SQLite::Database &db = mf.handle() ;

    SQLite::Session session(&db) ;
    SQLite::Connection &con = session.handle() ;

    unsigned char *blob;
    int blob_size;

    OSM::Rule::Context ctx(&doc) ;

    if ( layer.geom_ != "pois" ) return false ;

    SQLite::Transaction trans(con) ;

    string sql = "INSERT INTO " ;
    sql += layer.name_ + "_text (docid, content) VALUES(?, ?)" ;

    string geoCmd = "Transform(?," + layer.srid_ + ")" ;
    SQLite::Command cmd1(con, mf.insertFeatureSQL(layer.tags_, layer.types_, layer.name_, geoCmd)) ;
    SQLite::Command cmd2(con, sql) ;
    sql = "INSERT OR IGNORE INTO " + mf.dictionary_table_name_ + " (key) VALUES (?)" ;
    SQLite::Command cmd_dict(con, sql) ;

    bool cont ;

    for(int i=0 ; i<node_idxs.size() ; i++ )
    {
        int node_idx = node_idxs[i].first ;
        int rule_idx = node_idxs[i].second ;

        OSM::Node &node = doc.nodes[node_idx] ;

        if ( node.visited_ ) continue ;

        ctx.set(&doc.nodes[node_idx]) ;

        if ( !node.tags.empty() )
        {
            const Rule &r = layer.rules_[rule_idx] ;

            vector<Action> actions ;

            cont = processStoreActions(r, ctx, &node, actions) ;

            if ( !cont ) node.visited_ = true ;

            bindColumns(actions, layer, cmd1, cmd_dict) ;

            gaiaGeomCollPtr geo_pt = gaiaAllocGeomColl();

            geo_pt->Srid = 4326;

            gaiaAddPointToGeomColl (geo_pt, node.lon, node.lat);

            gaiaToSpatiaLiteBlobWkb (geo_pt, &blob, &blob_size);

            gaiaFreeGeomColl (geo_pt);

            cmd1.bind(1, blob, blob_size) ;
            cmd1.exec() ;

            sqlite3_int64 rid = con.last_insert_rowid() ;

            for(int j=0 ; j<actions.size() ; j++ )
            {
                 if ( actions[j].column_ == "content" )
                 {
                     cmd2.bind(1, rid) ;
                     cmd2.bind(2, actions[j].val_.toString()) ;
                     cmd2.exec() ;
                 }
            }

            cmd1.clear() ;
            cmd2.clear() ;
            free(blob);
        }
    }

    trans.commit() ;

    return true ;
}


bool addOSMLayerLines(MapFile &mf, OSM::Document &doc, const Layer &layer,
                         const vector<NodeRuleMap> &way_idxs,
                         vector<OSM::Way> &chunk_list,
                         const vector<NodeRuleMap > &rule_map
                      )
{

    SQLite::Database &db = mf.handle() ;

    SQLite::Session session(&db) ;
    SQLite::Connection &con = session.handle() ;

    unsigned char *blob;
    int blob_size;

    OSM::Rule::Context ctx(&doc) ;

    if ( layer.geom_ != "lines" ) return false ;

    SQLite::Transaction trans(con) ;

    string geoCmd = "CompressGeometry(Transform(?," + layer.srid_ + "))" ;
    SQLite::Command cmd(con, mf.insertFeatureSQL(layer.tags_, layer.types_, layer.name_, geoCmd)) ;
    string sql = "INSERT OR IGNORE INTO " + mf.dictionary_table_name_ + " (key) VALUES (?)" ;
    SQLite::Command cmd_dict(con, sql) ;

    int k=0 ;
    bool cont ;

    for(int i=0 ; i<way_idxs.size() ; i++ )
    {
        vector<Action> actions ;

        const NodeRuleMap &nr = way_idxs[i] ;

        int node_idx = nr.node_idx_ ;
        OSM::Way &way = doc.ways[node_idx] ;

        ctx.set(&way) ;

        for(int j=0 ; j<nr.matched_rules_.size() ; j++ )
        {
            int rule_idx = nr.matched_rules_[j] ;

            const Rule &r = layer.rules_[rule_idx] ;

            if ( ! processStoreActions(r, ctx, &way, actions) ) break ;
        }

        cmd.clear() ;

        bindColumns(actions, layer, cmd, cmd_dict) ;

        gaiaGeomCollPtr geo_line = gaiaAllocGeomColl();
        geo_line->Srid = 4326;

        gaiaLinestringPtr ls = gaiaAddLinestringToGeomColl (geo_line, way.nodes.size());

        for(int j=0 ; j<way.nodes.size() ; j++)
        {
            const OSM::Node &node = doc.nodes[way.nodes[j]] ;

            gaiaSetPoint (ls->Coords, j, node.lon, node.lat);
        }

        gaiaToSpatiaLiteBlobWkb (geo_line, &blob, &blob_size);

        gaiaFreeGeomColl (geo_line);

        cmd.bind(1, blob, blob_size) ;

        cmd.exec() ;

        free(blob) ;

    }

    for( int i=0 ; i<rule_map.size() ; i++ )
    {
        vector<Action> actions ;

        const NodeRuleMap &nr = rule_map[i] ;

        int node_idx = nr.node_idx_ ;
        OSM::Way &way = chunk_list[i] ;

        ctx.set(&way) ;

        for(int j=0 ; j<nr.matched_rules_.size() ; j++ )
        {
            int rule_idx = nr.matched_rules_[j] ;

            const Rule &r = layer.rules_[rule_idx] ;

            if ( ! processStoreActions(r, ctx, &way, actions) ) break ;
        }

        cmd.clear() ;

        bindColumns(actions, layer, cmd, cmd_dict) ;

        gaiaGeomCollPtr geo_line = gaiaAllocGeomColl();
        geo_line->Srid = 4326;

        gaiaLinestringPtr ls = gaiaAddLinestringToGeomColl (geo_line, way.nodes.size());

        for(int j=0 ; j<way.nodes.size() ; j++)
        {
            const OSM::Node &node = doc.nodes[way.nodes[j]] ;

            gaiaSetPoint (ls->Coords, j, node.lon, node.lat);
        }

        gaiaToSpatiaLiteBlobWkb (geo_line, &blob, &blob_size);

        gaiaFreeGeomColl (geo_line);

        cmd.bind(1, blob, blob_size) ;

        cmd.exec() ;

        free(blob) ;

    }

    trans.commit() ;

    return true ;
}

bool addOSMLayerPolygons(MapFile &mf, const OSM::Document &doc, const Layer &layer,
                         vector<OSM::Polygon> &polygons, const vector<NodeRuleMap > &poly_idxs)
{
    SQLite::Database &db = mf.handle() ;

    SQLite::Session session(&db) ;
    SQLite::Connection &con = session.handle() ;

    unsigned char *blob;
    int blob_size;

    OSM::Rule::Context ctx(&doc) ;

    if ( layer.geom_ != "polygons" ) return false ;

    // look for ways with same start and end point

    SQLite::Transaction trans(con) ;

    string geoCmd = "CompressGeometry(Transform(ST_BuildArea(?)," + layer.srid_ + "))" ;
    SQLite::Command cmd(con, mf.insertFeatureSQL(layer.tags_, layer.types_, layer.name_,  geoCmd)) ;
    string sql = "INSERT OR IGNORE INTO " + mf.dictionary_table_name_ + " (key) VALUES (?)" ;
    SQLite::Command cmd_dict(con, sql) ;

    for( int i=0 ; i< poly_idxs.size() ; i++ )
    {
        vector<Action> actions ;

        const NodeRuleMap &nr = poly_idxs[i] ;

        int poly_idx = nr.node_idx_ ;
        OSM::Polygon &poly = polygons[poly_idx] ;

        ctx.set(&poly) ;

        for(int j=0 ; j<nr.matched_rules_.size() ; j++ )
        {
            int rule_idx = nr.matched_rules_[j] ;

            const Rule &r = layer.rules_[rule_idx] ;

            if ( ! processStoreActions(r, ctx, &poly, actions) ) break ;
        }

        cmd.clear() ;


        bindColumns(actions, layer, cmd, cmd_dict) ;

        gaiaGeomCollPtr geo_poly = gaiaAllocGeomColl();
        geo_poly->Srid = 4326;

        for(int j=0 ; j<poly.rings.size() ; j++)
        {
            const OSM::Ring &ring = poly.rings[j] ;
            gaiaLinestringPtr gpoly = gaiaAddLinestringToGeomColl(geo_poly,ring.nodes.size());

            for(int k=0 ; k<ring.nodes.size() ; k++)
            {
                const OSM::Node &node = doc.nodes[ring.nodes[k]] ;

                gaiaSetPoint (gpoly->Coords, k, node.lon, node.lat);
            }
        }

        gaiaToSpatiaLiteBlobWkb (geo_poly, &blob, &blob_size);

        gaiaFreeGeomColl (geo_poly);

        cmd.bind(1, blob, blob_size) ;

        cmd.exec() ;

        free(blob) ;
    }

    trans.commit() ;

    return true ;
}

bool processOsmFiles(MapFile &m, const string &configFile, const vector<string> &osmFiles, bool overwrite, bool append)
{
    MapConfig cfg ;
    if ( !parseConfigFile(configFile, cfg) ) return false ;

    m.dictionary_table_name_ = cfg.pragmas_.get("dictionary_table_name", m.dictionary_table_name_);
    m.view_suffix_ = cfg.pragmas_.get("view_table_suffix", m.view_suffix_);
    m.geometry_column_name_ = cfg.pragmas_.get("geometry_column", m.geometry_column_name_);
    string table_prefix = cfg.pragmas_.get("table_prefix", "") ;

    m.createDictionary(overwrite) ;
    m.createResourcesTable(append) ;

    DictionaryIterator it(cfg.resources_) ;

    while ( it )
    {
        m.addResource(it.value(), it.key()) ;
        ++it ;
    }

    for(int j=0 ; j<cfg.layers_.size() ; j++)
    {
        const Layer &layer = cfg.layers_[j] ;

        if ( ! m.createLayerTable(table_prefix + layer.name_,
                                  layer.geom_,
                                  layer.srid_,
                                  m.geometry_column_name_,
                                  layer.tags_, layer.types_, overwrite, append) ) {
            cerr << "Failed to create layer " << layer.name_ << ", skipping" ;
            continue ;
        }

        if ( layer.geom_ == "pois" && ! m.createPOITable(cfg.pragmas_.get("table_prefix", "") + layer.name_, overwrite, append) ) {
            cerr << "Failed to create POI layer " << layer.name_ << ", skipping" ;
            continue ;
        }
    }

    for(int i=0 ; i<osmFiles.size() ; i++ )
    {
        OSM::Document doc ;

        cout << "Reading file: " << osmFiles[i] << endl ;

        if ( !doc.read(osmFiles[i]) )
        {
            cerr << "Error reading from " << osmFiles[i] << endl ;
            continue ;
        }

        OSM::Rule::Context ctx(&doc) ;

        cout << "Importing to database\n";

        for(int j=0 ; j<cfg.layers_.size() ; j++)
        {
            const Layer &layer = cfg.layers_[j] ;

            std::vector<NodeRuleMap> passFilterNodes, passFilterWays, passFilterPoly, passFilterRel ;

            if ( layer.geom_ == "points" )
            {
                for(int k=0 ; k<doc.nodes.size() ; k++ )
                {
                    OSM::Node &node = doc.nodes[k] ;

                    ctx.set(&node) ;

                    NodeRuleMap nr ;

                    nr.node_idx_ = k ;

                    for( int r = 0 ; r < layer.rules_.size() ; r++ )
                    {
                        if ( !layer.rules_[r].condition_->eval(ctx).toBoolean() ) continue ;

                        processSetTagActions(layer.rules_[r], ctx, &node) ;

                        nr.matched_rules_.push_back(r) ;
                    }

                    if ( !nr.matched_rules_.empty() ) passFilterNodes.push_back(nr) ;

                }

                addOSMLayerPoints(m, doc, layer, passFilterNodes) ;
            }
            if ( layer.geom_ == "pois" )
            {
                for(int k=0 ; k<doc.nodes.size() ; k++ )
                {
                    ctx.set(&(doc.nodes[k])) ;
                    for( int r = 0 ; r < layer.rules_.size() ; r++ )
                    {
                        if ( !layer.rules_[r].condition_->eval(ctx).toBoolean() ) continue ;

 //                       passFilterNodes.push_back(make_pair(k, r)) ;
                    }
                }

                for(int k=0 ; k<doc.ways.size() ; k++ )
                {
                    ctx.set(&(doc.ways[k])) ;
                    for( int r = 0 ; r < layer.rules_.size() ; r++ )
                    {
                        if ( !layer.rules_[r].condition_->eval(ctx).toBoolean() ) continue ;

 //                       passFilterWays.push_back(make_pair(k, r)) ;
                    }
                }

                // for the moment we just deal with nodes
   //             addOSMLayerPOIs(m, doc, layer, passFilterNodes) ;
            }
            else if ( layer.geom_ == "lines" )
            {
                for(int k=0 ; k<doc.ways.size() ; k++ )
                {
                    OSM::Way &way = doc.ways[k] ;

                    // deal with closed ways

                    if ( way.nodes.front() == way.nodes.back() )
                    {
                        if ( way.tags.get("area") == "yes" ) continue ;
                        if ( !way.tags.contains("highway") && !way.tags.contains("barrier") && !way.tags.contains("contour") ) continue ;
                    }

                    ctx.set(&way) ;

                    NodeRuleMap nr ;

                    nr.node_idx_ = k ;

                    for( int r = 0 ; r < layer.rules_.size() ; r++ )
                    {

                        if ( !layer.rules_[r].condition_->eval(ctx).toBoolean() ) continue ;

                        processSetTagActions(layer.rules_[r], ctx, &way) ;

                        nr.matched_rules_.push_back(r) ;
                    }

                    if ( !nr.matched_rules_.empty() ) passFilterWays.push_back(nr) ;
                }

                // relations of type route, merge ways into chunks

                vector<OSM::Way> chunk_list ;

                for(int k=0 ; k<doc.relations.size() ; k++ )
                {
                    OSM::Relation &relation = doc.relations[k] ;

                    if ( relation.tags.get("type") != "route" ) continue ;

                    ctx.set(&relation) ;

                    vector<int> matched ;

                    for( int r = 0 ; r < layer.rules_.size() ; r++ )
                    {
                        if ( !layer.rules_[r].condition_->eval(ctx).toBoolean() ) continue ;

                        processSetTagActions(layer.rules_[r], ctx, &relation) ;

                        matched.push_back(r) ;
                    }

                    if ( matched.empty() ) continue ;

                    vector<OSM::Way> chunks ;
                    if ( !makeWaysFromRelation(doc, relation, chunks) ) continue ;

                    for(int c=0 ; c<chunks.size() ; c++)
                    {
                        NodeRuleMap nr ;

                        nr.node_idx_ = chunk_list.size() ;
                        nr.matched_rules_ = matched ;

                        chunk_list.push_back(chunks[i]) ;
                        passFilterRel.push_back(nr) ;
                    }

                }

                addOSMLayerLines(m, doc, layer, passFilterWays, chunk_list, passFilterRel) ;
            }
            else if ( layer.geom_ == "polygons" )
            {
                vector<OSM::Polygon> polygons ;

                // first look for multi-polygon relations

                for(int k=0 ; k<doc.relations.size() ; k++ )
                {
                    OSM::Relation &relation = doc.relations[k] ;

                    string rel_type = relation.tags.get("type") ;
                    if (  rel_type != "multipolygon" && rel_type != "boundary" ) continue ;

                    ctx.set(&relation) ;

                    vector<int> matched ;

                    for( int r = 0 ; r < layer.rules_.size() ; r++ )
                    {
                        if ( !layer.rules_[r].condition_->eval(ctx).toBoolean() ) continue ;

                        processSetTagActions(layer.rules_[r], ctx, &relation) ;

                        matched.push_back(r) ;
                    }

                    if ( matched.empty() ) continue ;

                    OSM::Polygon polygon ;
                    if ( !makePolygonsFromRelation(doc, relation, polygon) ) continue ;

                    NodeRuleMap nr ;

                    nr.node_idx_ = polygons.size() ;
                    nr.matched_rules_ = matched ;
                    polygons.push_back(polygon) ;

                    passFilterPoly.push_back(nr) ;

                }

                // check simple polygons

                for(int k=0 ; k<doc.ways.size() ; k++ )
                {
                    OSM::Way &way = doc.ways[k] ;

                    if ( way.nodes.front() != way.nodes.back() ) continue ;
                    if ( way.tags.get("area") == "no" ) continue ;
                    if ( way.tags.contains("highway") ) continue ;
                    if ( way.tags.contains("barrier") ) continue ;

                    ctx.set(&way) ;

                    NodeRuleMap nr ;

                    nr.node_idx_ = polygons.size()  ;

                    for( int r = 0 ; r < layer.rules_.size() ; r++ )
                    {
                        if ( !layer.rules_[r].condition_->eval(ctx).toBoolean() ) continue ;

                        processSetTagActions(layer.rules_[r], ctx, &way) ;

                        nr.matched_rules_.push_back(r) ;
                    }

                    if ( nr.matched_rules_.empty() ) continue ;

                    OSM::Polygon poly ;

                    OSM::Ring ring ;
                    ring.nodes.insert(ring.nodes.end(), way.nodes.begin(), way.nodes.end()) ;
                    poly.rings.push_back(ring) ;
                    poly.tags = way.tags ;
                    polygons.push_back(poly) ;
                    passFilterPoly.push_back(nr) ;

                }

                addOSMLayerPolygons(m, doc, layer, polygons, passFilterPoly) ;
            }



        }
    }


}


void printUsageAndExit()
{
    cerr << "Usage: osm2sqlite [--overwrite] [--append] --config <config_file> <map_file> <file_name>+" << endl ;
    exit(1) ;
}


int main(int argc, char *argv[])
{
    string mapFile, configFile ;
    vector<string> osmFiles ;
    bool overwrite = true, append = false ;

    for( int i=1 ; i<argc ; i++ )
    {
        string arg = argv[i] ;

        if ( arg == "--config" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            configFile = argv[i] ;
        }
        else if ( arg == "--overwrite" ) // if table exists and overwrite is true it will drop the table and make a new one otherwise it will try to write in the existing table
            overwrite = true ;
        else if ( arg == "--append" )   // if false it will delete contents of the table before inserting new data
            append = true ;
        else if ( mapFile.empty() )
        {
            mapFile = argv[i] ;
        }
        else
            osmFiles.push_back(argv[i]) ;
    }

    if ( configFile.empty() ||  mapFile.empty() ||  osmFiles.empty() )
        printUsageAndExit() ;

    MapFile gfile ;

    if ( !gfile.open(mapFile) && !gfile.create(mapFile) )
    {
        cerr << "can't open map file: " << mapFile << endl ;
        exit(1) ;
    }

    processOsmFiles(gfile, configFile, osmFiles, overwrite, append) ;


    return 1 ;

}
