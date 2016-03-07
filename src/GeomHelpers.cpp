#include "GeomHelpers.h"

using namespace std ;
namespace tms {

const double tile_size = 256 ;
constexpr double gm_initial_resolution = 2 * M_PI * 6378137 / tile_size ;
constexpr double gm_origin_shift = 2 * M_PI * 6378137 / 2.0 ;

// Resolution (meters/pixel) for given zoom level (measured at Equator)
double resolution(uint zoom) {
    return gm_initial_resolution / pow(2, zoom) ;
}
// Converts given lat/lon in WGS84 Datum to XY in Spherical Mercator EPSG:900913
void latlonToMeters(double lat, double lon, double &mx, double &my) {
    mx = lon * gm_origin_shift / 180.0 ;
    my = log( tan((90 + lat) * M_PI / 360.0 )) / (M_PI / 180.0) ;

    my = my * gm_origin_shift / 180.0 ;
}

// Converts XY point from Spherical Mercator EPSG:900913 to lat/lon in WGS84 Datum
void metersToLatLon(double mx, double my, double &lat, double &lon) {
    lon = (mx / gm_origin_shift) * 180.0 ;
    lat = (my / gm_origin_shift) * 180.0 ;

    lat = 180 / M_PI * (2 * atan( exp( lat * M_PI / 180.0)) - M_PI / 2.0) ;
}

//Converts pixel coordinates in given zoom level of pyramid to EPSG:900913
void pixelsToMeters(double px, double py, uint zoom, double &mx, double &my) {
    double res = resolution( zoom ) ;
    mx = px * res - gm_origin_shift ;
    my = py * res - gm_origin_shift ;
}

//Converts EPSG:900913 to pyramid pixel coordinates in given zoom level
void metersToPixels(double mx, double my, uint zoom, double &px, double &py) {
    double res = resolution( zoom ) ;
    px = (mx + gm_origin_shift) / res ;
    py = (my + gm_origin_shift) / res ;
}

//Returns a tile covering region in given pixel coordinates
void pixelsToTile(double px, double py, uint &tx, uint &ty) {
    tx = int( ceil( px / float(tile_size) ) - 1 ) ;
    ty = int( ceil( py / float(tile_size) ) - 1 ) ;
}

void tileToPixels(uint tx, uint ty, double &px, double &py) {
    px = tx * tile_size ;
    py = ty * tile_size ;
}

// Returns tile for given mercator coordinates
void metersToTile(double mx, double my, uint zoom, uint &tx, uint &ty) {
    double px, py ;
    metersToPixels(mx, my, zoom, px, py) ;
    pixelsToTile(px, py, tx, ty) ;
}
// Returns bounds of the given tile in EPSG:900913 coordinates

void tileBounds(uint tx, uint ty, uint zoom, double &minx, double &miny, double &maxx, double &maxy) {
    pixelsToMeters( tx*tile_size, ty*tile_size, zoom, minx, miny ) ;
    pixelsToMeters( (tx+1)*tile_size, (ty+1)*tile_size, zoom, maxx, maxy ) ;
}

// Returns bounds of the given tile in latutude/longitude using WGS84 datum
void tileLatLonBounds(uint tx, uint ty, uint zoom, double &minLat, double &minLon, double &maxLat, double &maxLon) {

    double minx, miny, maxx, maxy ;
    tileBounds(tx, ty, zoom, minx, miny, maxx, maxy) ;
    metersToLatLon(minx, miny, minLat, minLon) ;
    metersToLatLon(maxx, maxy, maxLat, maxLon) ;
}


}
