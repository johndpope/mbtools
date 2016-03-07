#ifndef __MAP_FILE_H__
#define __MAP_FILE_H__

#include "Database.h"
#include "Dictionary.h"
#include "ImportConfig.h"
#include "MapConfig.h"
#include "VectorTile.h"
#include "OsmDocument.h"

#include <map>

// The map file is a spatialite database. It is used as temporary storage for OSM data structured per layer and filtered using the configuration file.

using std::string ;
using std::vector ;
using std::map ;


class MapFile {
public:
    MapFile() ;
    ~MapFile() ;

    // Create a new database deleting any old file if it exists.

    bool create(const string &filePath) ;

    // Get handle to database

    SQLite::Database &handle() const { return *db_ ; }

    bool hasLayer(const std::string &layerName);

    bool createLayerTable(const string &layerName, const string &layerType,
                          const string &layerSrid);

    std::string insertFeatureSQL(const std::string &layerName,
                                 const std::string &geomCmd = "?") ;

    bool processOsmFiles(const vector<string> &files, const ImportConfig &cfg) ;


    bool queryTile(const MapConfig &cfg, uint tx, uint ty, uint tz, VectorTileWriter &tile) ;

private:

    bool addOSMLayerPoints(OSM::Document &doc, const ImportLayer &layer,
                           const vector<NodeRuleMap > &node_idxs) ;

    bool addOSMLayerLines(OSM::Document &doc, const ImportLayer &layer,
                          const vector<NodeRuleMap> &way_idxs,
                          vector<OSM::Way> &chunk_list,
                          const vector<NodeRuleMap > &rule_map) ;

    bool addOSMLayerPolygons(const OSM::Document &doc, const ImportLayer &layer,
                             vector<OSM::Polygon> &polygons, const vector<NodeRuleMap > &poly_idxs) ;


    SQLite::Database *db_ ;

public:

    std::string geom_column_name_ ;

};





#endif
