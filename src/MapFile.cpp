#include "MapFile.h"

#include <XmlDocument.h>

#include <spatialite.h>
#include <fstream>
#include <boost/filesystem.hpp>

using namespace std;

class SpatialLiteSingleton
{
    public:

    static SpatialLiteSingleton instance_;

    static SpatialLiteSingleton& instance() {
            return instance_;
    }

private:

    SpatialLiteSingleton () {
        spatialite_init(false);
    };

    ~SpatialLiteSingleton () {
        spatialite_cleanup();
    }

    SpatialLiteSingleton( SpatialLiteSingleton const & );

    void operator = ( SpatialLiteSingleton const & );


};

SpatialLiteSingleton SpatialLiteSingleton::instance_ ;


MapFile::MapFile():  db_(0), min_zoom_(12), max_zoom_(14), geom_column_name_("geom") {}

MapFile::~MapFile() {
    delete db_ ;
}

bool MapFile::create(const std::string &name) {

    if ( boost::filesystem::exists(name) )
        boost::filesystem::remove(name);

    db_ = new SQLite::Database(name) ;

    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {
        con.exec("PRAGMA synchronous=NORMAL") ;
        con.exec("PRAGMA journal_mode=WAL") ;
        con.exec("SELECT InitSpatialMetadata(1);") ;

        fileName_ = name ;

        return true ;
    }
    catch ( SQLite::Exception & )
    {

        return false ;
    }

}

bool MapFile::hasLayer(const std::string &layerName)
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {
        string sql = "SELECT count(*) FROM sqlite_master WHERE sqlite_master.type='table' AND sqlite_master.name=?;" ;

        SQLite::Query q(con, sql) ;

        q.bind(layerName) ;

        SQLite::QueryResult res = q.exec() ;

        return ( res && res.get<int>(0) ) ;
    }
    catch ( SQLite::Exception & )
    {
        return false ;
    }

}

bool MapFile::createLayerTable(const std::string &layerName, const string &layerType, const string &layerSrid)
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {
        bool has_layer = hasLayer(layerName) ;
        if ( has_layer ) return false ;

        string sql ;

        sql = "CREATE TABLE ";
        sql +=  layerName + "(gid INTEGER PRIMARY KEY AUTOINCREMENT, tags TEXT)" ;

        SQLite::Command(con, sql).exec() ;

        // Add geometry column

        sql = "SELECT AddGeometryColumn( '"  ;
        sql += layerName ;
        sql += "', '" + geom_column_name_ +"', " + layerSrid + "," ;

        if ( layerType == "points" )
            sql += "'POINT', 2);" ;
        else if ( layerType == "lines" )
            sql +="'LINESTRING', 2);" ;
        else if ( layerType == "polygons" )
            sql += "'POLYGON', 2);" ;

        SQLite::Command(con, sql).exec() ;

        // create spatial index

        con.exec("SELECT CreateSpatialIndex('%q', '%q');", layerName.c_str(), geom_column_name_.c_str()) ;

        return true ;
    }
    catch ( SQLite::Exception &e)
    {
        cerr << e.what() << endl ;
        return false ;
    }
}


string MapFile::insertFeatureSQL(const string &layerName, const string &geomCmd )
{
    string sql ;

    sql = "INSERT INTO " ;
    sql += layerName ;
    sql += "(" + geom_column_name_ ;
    sql += ",tags" ;

    sql += ") VALUES (" + geomCmd + ",?)";
    return sql ;
}



static bool processSetTagActions(const Rule &r, OSM::Rule::Context &ctx, OSM::Feature *node)
{
    OSM::Rule::Command *action = r.actions_ ;

    bool cont = false ;

    while ( action )
    {
        if ( action->cmd_ ==  OSM::Rule::Command::Add )
        {
            node->tags_.add(action->tag_, action->expression_->eval(ctx).toString()) ;
        }
        else if ( action->cmd_ == OSM::Rule::Command::Set )
        {
            if ( node->tags_.contains(action->tag_) )
                node->tags_[action->tag_] = action->expression_->eval(ctx).toString() ;
            else
                node->tags_.add(action->tag_, action->expression_->eval(ctx).toString()) ;
        }
        else if ( action->cmd_ == OSM::Rule::Command::Continue )
        {
            cont = true ;
        }
        else if ( action->cmd_ == OSM::Rule::Command::Delete )
        {
            node->tags_.remove(action->tag_) ;
        }

        action = action->next_ ;

    }

    return cont ;

}

static bool processStoreActions(const Rule &r, OSM::Rule::Context &ctx, OSM::Feature *node, vector<Action> &actions)
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

            act.key_ = action->tag_ ;
            act.val_ = action->expression_->eval(ctx) ;

            actions.push_back(act) ;


        }

        action = action->next_ ;

    }

    return cont ;

}

void bindActions(const vector<Action> &actions, SQLite::Command &cmd)
{
    string kvl ;

    for( int i=0 ; i<actions.size() ; i++ )
    {
        const Action &act = actions[i] ;

        string val = act.val_.toString() ;
        string key = act.key_ ;

        kvl += key + '@' + val + ';' ;
    }

    if ( kvl.empty() ) cmd.bind(2, SQLite::Nil) ;
    else cmd.bind(2, kvl) ;
}

bool MapFile::addOSMLayerPoints(OSM::Document &doc, const Layer &layer,
                       const vector<NodeRuleMap > &node_idxs)
{
    SQLite::Database &db = handle() ;

    SQLite::Session session(&db) ;
    SQLite::Connection &con = session.handle() ;

    unsigned char *blob;
    int blob_size;

    if ( layer.geom_ != "points" ) return false ;

    SQLite::Transaction trans(con) ;

    string geoCmd = "Transform(?," + layer.srid_ + ")" ;
    SQLite::Command cmd(con, insertFeatureSQL(layer.name_, geoCmd)) ;

    for(int i=0 ; i<node_idxs.size() ; i++ )
    {
        vector<Action> actions ;

        const NodeRuleMap &nr = node_idxs[i] ;

        int node_idx = nr.node_idx_ ;
        OSM::Node &node = doc.nodes_[node_idx] ;

        OSM::Rule::Context ctx(&node) ;

        for(int j=0 ; j<nr.matched_rules_.size() ; j++ )
        {
            int rule_idx = nr.matched_rules_[j] ;

            const Rule &r = layer.rules_[rule_idx] ;

            if ( ! processStoreActions(r, ctx, &node, actions) ) break ;
        }

        cmd.clear() ;

        bindActions(actions, cmd) ;

        gaiaGeomCollPtr geo_pt = gaiaAllocGeomColl();

        geo_pt->Srid = 4326;

        gaiaAddPointToGeomColl (geo_pt, node.lon_, node.lat_);

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


bool MapFile::addOSMLayerLines(OSM::Document &doc, const Layer &layer,
                      const vector<NodeRuleMap> &way_idxs,
                      vector<OSM::Way> &chunk_list,
                      const vector<NodeRuleMap > &rule_map
                      )
{

    SQLite::Database &db = handle() ;

    SQLite::Session session(&db) ;
    SQLite::Connection &con = session.handle() ;

    unsigned char *blob;
    int blob_size;

    if ( layer.geom_ != "lines" ) return false ;

    SQLite::Transaction trans(con) ;

    string geoCmd = "CompressGeometry(Transform(?," + layer.srid_ + "))" ;
    SQLite::Command cmd(con, insertFeatureSQL(layer.name_, geoCmd)) ;

    for(int i=0 ; i<way_idxs.size() ; i++ )
    {
        vector<Action> actions ;

        const NodeRuleMap &nr = way_idxs[i] ;

        int node_idx = nr.node_idx_ ;
        OSM::Way &way = doc.ways_[node_idx] ;

        OSM::Rule::Context ctx(&way) ;

        for(int j=0 ; j<nr.matched_rules_.size() ; j++ )
        {
            int rule_idx = nr.matched_rules_[j] ;

            const Rule &r = layer.rules_[rule_idx] ;

            if ( ! processStoreActions(r, ctx, &way, actions) ) break ;
        }

        cmd.clear() ;

        bindActions(actions, cmd) ;

        gaiaGeomCollPtr geo_line = gaiaAllocGeomColl();
        geo_line->Srid = 4326;

        gaiaLinestringPtr ls = gaiaAddLinestringToGeomColl (geo_line, way.nodes_.size());

        for(int j=0 ; j<way.nodes_.size() ; j++)
        {
            const OSM::Node &node = doc.nodes_[way.nodes_[j]] ;

            gaiaSetPoint (ls->Coords, j, node.lon_, node.lat_);
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

        OSM::Rule::Context ctx(&way) ;

        for(int j=0 ; j<nr.matched_rules_.size() ; j++ )
        {
            int rule_idx = nr.matched_rules_[j] ;

            const Rule &r = layer.rules_[rule_idx] ;

            if ( ! processStoreActions(r, ctx, &way, actions) ) break ;
        }

        cmd.clear() ;

        bindActions(actions, cmd) ;

        gaiaGeomCollPtr geo_line = gaiaAllocGeomColl();
        geo_line->Srid = 4326;

        gaiaLinestringPtr ls = gaiaAddLinestringToGeomColl (geo_line, way.nodes_.size());

        for(int j=0 ; j<way.nodes_.size() ; j++)
        {
            const OSM::Node &node = doc.nodes_[way.nodes_[j]] ;

            gaiaSetPoint (ls->Coords, j, node.lon_, node.lat_);
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

bool MapFile::addOSMLayerPolygons(const OSM::Document &doc, const Layer &layer,
                         vector<OSM::Polygon> &polygons, const vector<NodeRuleMap > &poly_idxs)
{
    SQLite::Database &db = handle() ;

    SQLite::Session session(&db) ;
    SQLite::Connection &con = session.handle() ;

    unsigned char *blob;
    int blob_size;

    if ( layer.geom_ != "polygons" ) return false ;

    SQLite::Transaction trans(con) ;

    string geoCmd = "CompressGeometry(Transform(ST_BuildArea(?)," + layer.srid_ + "))" ;
    SQLite::Command cmd(con, insertFeatureSQL(layer.name_,  geoCmd)) ;

    for( int i=0 ; i< poly_idxs.size() ; i++ )
    {
        vector<Action> actions ;

        const NodeRuleMap &nr = poly_idxs[i] ;

        int poly_idx = nr.node_idx_ ;
        OSM::Polygon &poly = polygons[poly_idx] ;

        OSM::Rule::Context ctx(&poly) ;

        for(int j=0 ; j<nr.matched_rules_.size() ; j++ )
        {
            int rule_idx = nr.matched_rules_[j] ;

            const Rule &r = layer.rules_[rule_idx] ;

            if ( ! processStoreActions(r, ctx, &poly, actions) ) break ;
        }

        cmd.clear() ;

        bindActions(actions, cmd) ;

        gaiaGeomCollPtr geo_poly = gaiaAllocGeomColl();
        geo_poly->Srid = 4326;

        for(int j=0 ; j<poly.rings_.size() ; j++)
        {
            auto ring = poly.rings_[j] ;
            gaiaLinestringPtr gpoly = gaiaAddLinestringToGeomColl(geo_poly,ring.nodes_.size());

            for(int k=0 ; k<ring.nodes_.size() ; k++)
            {
                auto node = doc.nodes_[ring.nodes_[k]] ;

                gaiaSetPoint (gpoly->Coords, k, node.lon_, node.lat_);
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

bool MapFile::processOsmFiles(const vector<string> &osmFiles, const MapConfig &cfg)
{
    // read files from memory and write to spatialite database

    for(int i=0 ; i<osmFiles.size() ; i++ ) {

        OSM::Document doc ;

        cout << "Reading file: " << osmFiles[i] << endl ;

        if ( !doc.read(osmFiles[i]) )
        {
            cerr << "Error reading from " << osmFiles[i] << endl ;
            continue ;
        }

        for(uint j=0 ; j<cfg.layers_.size() ; j++)
        {
            const Layer &layer = cfg.layers_[j] ;

            std::vector<NodeRuleMap> passFilterNodes, passFilterWays, passFilterPoly, passFilterRel ;

            if ( layer.geom_ == "points" )
            {
                for(int k=0 ; k<doc.nodes_.size() ; k++ )
                {
                    auto node = doc.nodes_[k] ;

                    OSM::Rule::Context ctx(&node) ;

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

                addOSMLayerPoints(doc, layer, passFilterNodes) ;
            }
            else if ( layer.geom_ == "lines" )
            {
                for(int k=0 ; k<doc.ways_.size() ; k++ )
                {
                    auto way = doc.ways_[k] ;

                    // deal with closed ways

                    if ( way.nodes_.front() == way.nodes_.back() )
                    {
                        if ( way.tags_.get("area") == "yes" ) continue ;
                        if ( !way.tags_.contains("highway") && !way.tags_.contains("barrier") && !way.tags_.contains("contour") ) continue ;
                    }

                    OSM::Rule::Context ctx(&way) ;

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

                for(int k=0 ; k<doc.relations_.size() ; k++ )
                {
                    auto relation = doc.relations_[k] ;

                    if ( relation.tags_.get("type") != "route" ) continue ;

                    OSM::Rule::Context ctx(&relation) ;

                    vector<uint> matched ;

                    for( int r = 0 ; r < layer.rules_.size() ; r++ )
                    {
                        if ( !layer.rules_[r].condition_->eval(ctx).toBoolean() ) continue ;

                        processSetTagActions(layer.rules_[r], ctx, &relation) ;

                        matched.push_back(r) ;
                    }

                    if ( matched.empty() ) continue ;

                    vector<OSM::Way> chunks ;
                    if ( !OSM::Document::makeWaysFromRelation(doc, relation, chunks) ) continue ;

                    for(int c=0 ; c<chunks.size() ; c++)
                    {
                        NodeRuleMap nr ;

                        nr.node_idx_ = chunk_list.size() ;
                        nr.matched_rules_ = matched ;

                        chunk_list.push_back(chunks[i]) ;
                        passFilterRel.push_back(nr) ;
                    }

                }

                addOSMLayerLines(doc, layer, passFilterWays, chunk_list, passFilterRel) ;
            }
            else if ( layer.geom_ == "polygons" )
            {
                vector<OSM::Polygon> polygons ;

                // first look for multi-polygon relations

                for(int k=0 ; k<doc.relations_.size() ; k++ )
                {
                    OSM::Relation &relation = doc.relations_[k] ;

                    string rel_type = relation.tags_.get("type") ;
                    if (  rel_type != "multipolygon" && rel_type != "boundary" ) continue ;

                    OSM::Rule::Context ctx(&relation) ;

                    vector<uint> matched ;

                    for( uint r = 0 ; r < layer.rules_.size() ; r++ )
                    {
                        if ( !layer.rules_[r].condition_->eval(ctx).toBoolean() ) continue ;

                        processSetTagActions(layer.rules_[r], ctx, &relation) ;

                        matched.push_back(r) ;
                    }

                    if ( matched.empty() ) continue ;

                    OSM::Polygon polygon ;
                    if ( !OSM::Document::makePolygonsFromRelation(doc, relation, polygon) ) continue ;

                    NodeRuleMap nr ;

                    nr.node_idx_ = polygons.size() ;
                    nr.matched_rules_ = matched ;
                    polygons.push_back(polygon) ;

                    passFilterPoly.push_back(nr) ;

                }

                // check simple polygons

                for(int k=0 ; k<doc.ways_.size() ; k++ )
                {
                    auto way = doc.ways_[k] ;

                    if ( way.nodes_.front() != way.nodes_.back() ) continue ;
                    if ( way.tags_.get("area") == "no" ) continue ;
                    if ( way.tags_.contains("highway") ) continue ;
                    if ( way.tags_.contains("barrier") ) continue ;

                    OSM::Rule::Context ctx(&way) ;

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
                    ring.nodes_.insert(ring.nodes_.end(), way.nodes_.begin(), way.nodes_.end()) ;
                    poly.rings_.push_back(ring) ;
                    poly.tags_ = way.tags_ ;
                    polygons.push_back(poly) ;
                    passFilterPoly.push_back(nr) ;

                }

                addOSMLayerPolygons(doc, layer, polygons, passFilterPoly) ;
            }



        }
    }

    return true ;

}


static string makeBBoxQuery(const std::string &tableName, const std::string &geomColumn,
                              const std::string &condition, const BBox &bbox)
{
    stringstream sql ;

    sql.precision(16) ;

    sql << "SELECT tags," ;

    sql << "ST_Intersection(" << geomColumn << ",BuildMBR(" ;
    sql << bbox.minx_ << ',' << bbox.miny_ << ',' << bbox.maxx_ << ',' << bbox.maxy_ << "," << bbox.srid_ ;
    sql << ") AS _geom_ FROM " << tableName << " AS __table__";

    sql << " WHERE " << condition ;

    if ( !condition.empty() ) sql << " AND " ;

    sql << "__table__.ROWID IN ( SELECT ROWID FROM SpatialIndex WHERE f_table_name='" << tableName << "' AND search_frame = BuildMBR(" ;
    sql << bbox.minx_ << ',' << bbox.miny_ << ',' << bbox.maxx_ << ',' << bbox.maxy_ << "," << bbox.srid_ << "))" ;

    return sql.str() ;
}

static Dictionary parseTags(const string &tags)
{



}

static Geometry parseGeometryBlob(gaiaGeomCollPtr geom)
{

}

bool MapFile::queryTile(const MapConfig &cfg, const BBox &box, VectorTile &tile)
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    for ( const Layer &layer: cfg.layers_ ) {

        string sql = makeBBoxQuery(layer.name_, geom_column_name_, "", box) ;

 //   sql = "SELECT gid,type,state,notes, ST_Transform(geom,3857) AS _geom_ FROM tracks WHERE (\"type\"='primary') AND ROWID IN ( SELECT ROWID FROM SpatialIndex WHERE f_table_name='tracks' AND search_frame = ST_Transform(BuildMBR(2617203.848484434,4935997.538543543,2636771.727725439,4955565.417784547,3857),4326))" ;

        VectorTile::Layer vt_layer ;
        vt_layer.name_ = layer.name_ ;

        try {

            SQLite::Query q(con, sql) ;

            SQLite::QueryResult res = q.exec() ;

            while ( res )
            {
                int buf_size ;
                const char *data = res.getBlob("_geom_", buf_size) ;

                gaiaGeomCollPtr geom = gaiaFromSpatiaLiteBlobWkb ((const unsigned char *)data, buf_size);

                string tags = res.get<string>("tags") ;

                VectorTile::Feature ft ;
                ft.geom_ = parseGeometryBlob(geom) ;
                ft.tags_ = parseTags(tags) ;
                vt_layer.features_.push_back(ft) ;

                res.next() ;
            }
        }
        catch ( SQLite::Exception &e )
        {
            cout << e.what() << endl ;
            return false ;

        }

        if ( !vt_layer.features_.empty() )
            tile.layers_.push_back(vt_layer) ;
    }

    return true ;
}


