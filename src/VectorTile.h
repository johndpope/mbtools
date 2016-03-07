#ifndef __VECTOR_TILE_WRITTER_H__
#define __VECTOR_TILE_WRITTER_H__

#include <Dictionary.h>

#include <vector>
#include <string>
#include <map>

#include <sqlite3.h>
#include <spatialite.h>

#include "GeomHelpers.h"
#include "vector_tile.pb.h"

class VectorTileWriter {
public:

    VectorTileWriter(uint tx, uint ty, uint tz, uint te):
        current_layer_(nullptr), current_feature_(nullptr), tile_extent_(te), zoom_(tz) {
        tms::tileToPixels(tx, ty, tile_origin_x_, tile_origin_y_) ;
    }

    void beginLayer(const std::string &name) ;
    void endLayer() ;
    void encodeFeatures(const gaiaGeomCollPtr &geom, const Dictionary &attr);

    std::string toString(bool compress = true) ;

private:

    vector_tile::Tile tile_msg_ ;
    vector_tile::Tile_Layer *current_layer_ ;
    vector_tile::Tile_Feature *current_feature_ ;

    double tile_origin_x_, tile_origin_y_ ;
    uint tile_extent_, zoom_ ;

    std::map<std::string, uint> key_map_, value_map_ ;

    void tile_coords(double x, double y, int &tx, int &ty) {
        double px, py ;
        tms::metersToPixels(x, y, zoom_, px, py) ;
        tx = ( tile_origin_x_ - px )/tile_extent_ ;
        ty = ( tile_origin_y_ - px )/tile_extent_ ;
    }

    void addPoint(int x, int y) ;
    void addGeomCmd(uint cmd, uint count=1) ;
    void encodePointGeometry(const gaiaGeomCollPtr &geom, const Dictionary &attr) ;
    void encodeLineGeometry(const gaiaGeomCollPtr &geom, const Dictionary &attr) ;
    void encodePolygonGeometry(const gaiaGeomCollPtr &geom, const Dictionary &attr) ;
    void setFeatureAttributes(const Dictionary &attr) ;
};











#endif
