#ifndef __MAP_FILE_H__
#define __MAP_FILE_H__

#include <Database.h>
#include <Dictionary.h>

#include <map>


// The map file is a spatialite database. It contains various tables contaning metadata such as available layers and geometry tables containing the actual data.

class ConnectionPool ;
class ProjectionCache ;
class RasterTile ;

enum LayerType { Points, Lines, Polygons } ;

using std::string ;
using std::vector ;
using std::map ;

struct POI {
    double lat_, lon_ ; // coordinates
    Dictionary tags_ ; // returned tags
    double distance_ ;  // distance from search center
};

class BBox {

public:

    BBox() {}

    BBox(double minx_, double miny_, double maxx_, double maxy_, int srid_ = 4326):
        minx(minx_), miny(miny_), maxx(maxx_), maxy(maxy_), srid(srid_) {
    }

    double width() const { return maxx - minx ; }
    double height() const { return maxy - miny ; }

    bool transform(int target_srid, BBox &q, ProjectionCache &prj) const ;

    double minx, miny, maxx, maxy ;
    int srid ;
};


class MapFile {
public:
    MapFile() ;
    ~MapFile() ;

    // Open for reading

    bool open(const string &filePath) ;

    // Create a new database deleting any old file if it exists.

    bool create(const string &filePath) ;


    // Delete layer and all associated indexes and metadata

    bool deleteLayer(const std::string &layerName);

    // Get the assigned geometry column for this layer

    string getLayerGeometryColumn(const string &) const ;

    // Get all geometry columns in the layer

    void getLayerGeometryColumns(const string &layerName, vector<string> &names) const ;

    // Get proj4 string associated with the given srid

    string getProj4SRID(int srid) const;

    // Get handle to database

    SQLite::Database &handle() const { return *db_ ; }

    // various data source may be inserted in the database using ogr/gdal
    // ogr2ogr -update -overwrite -nln corine_hellas -nlt POLYGON -f SQLite -select level1,level2,level3 -s_srs EPSG:4326 -lco FORMAT=SPATIALITE -spat 23.590000 40.435000 23.830000 40.675000 map ~/GPS/mapping/corine_hellas.shp

    // Equivalently rasters are provided using rasterlite tools
    // rasterlite_load -d ~/tmp/map -v -T hillshade -f maps/stratoniko/images/hillshade.tif -i PNG
    // rasterlite_pyramid -d ~/tmp/map -T hillshade -v
    // rasterlite_topmost -d ~/tmp/map -T hillshade -v

    // The metadata for the external layer should be provided with the following function.

    bool hasLayer(const std::string &layerName);

    bool createLayerTable(const string &layerName, const string &layerType,
                          const string &geomColumn,
                          const std::vector<string> &tags,
                          const std::vector<string> &tag_types,
                          bool ovewrite,
                          bool append);

    bool createPOITable(const string &layerName,
                          bool ovewrite,
                          bool append);

    bool createDictionary(bool overwrite) ;

private:

    bool hasTable(const std::string &tableName);

private:

    SQLite::Database *db_ ;

    string fileName ;

};


std::string insertFeatureSQL(const std::vector<string> &tags, const std::vector<string> &types, const std::string &layerName, const std::string &geomCmd = "?") ;



#endif
