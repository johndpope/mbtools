#ifndef __GEOM_HELPERS_H__
#define __GEOM_HELPERS_H__

#include <boost/geometry.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/index/rtree.hpp>

typedef boost::geometry::model::d2::point_xy<double> Point;
typedef boost::geometry::model::linestring<Point> Linestring;
typedef boost::geometry::model::polygon<Point> Polygon;
typedef boost::geometry::model::multi_polygon<Polygon> MultiPolygon;
typedef boost::geometry::model::multi_linestring<Linestring> MultiLinestring;
typedef boost::geometry::model::box<Point> Box;
typedef boost::geometry::model::ring<Point> Ring;
typedef boost::variant<Point,Linestring,MultiLinestring,MultiPolygon> Geometry;
typedef std::pair<Box, uint> IndexValue;
typedef boost::geometry::index::rtree< IndexValue, boost::geometry::index::quadratic<16> > RTree;



namespace tms {

// Resolution (meters/pixel) for given zoom level (measured at Equator)
double resolution(uint zoom) ;

// Converts given lat/lon in WGS84 Datum to XY in Spherical Mercator EPSG:900913
void latlonToMeters(double lat, double lon, double &mx, double &my)  ;

// Converts XY point from Spherical Mercator EPSG:900913 to lat/lon in WGS84 Datum
void metersToLatLon(double mx, double my, double &lat, double &lon) ;

//Converts pixel coordinates in given zoom level of pyramid to EPSG:900913
void pixelsToMeters(double px, double py, uint zoom, double &mx, double &my) ;

//Converts EPSG:900913 to pyramid pixel coordinates in given zoom level
void metersToPixels(double mx, double my, uint zoom, double &px, double &py) ;

//Returns a tile covering region in given pixel coordinates
void pixelsToTile(double px, double py, uint &tx, uint &ty) ;

void tileToPixels(uint tx, uint ty, double &px, double &py) ;

// Returns tile for given mercator coordinates
void metersToTile(double mx, double my, uint zoom, uint &tx, uint &ty) ;

// Returns bounds of the given tile in EPSG:900913 coordinates
void tileBounds(uint tx, uint ty, uint zoom, double &minx, double &miny, double &maxx, double &maxy) ;

// Returns bounds of the given tile in latutude/longitude using WGS84 datum
void tileLatLonBounds(uint tx, uint ty, uint zoom, double &minLat, double &minLon, double &maxLat, double &maxLon) ;

}




















#endif
