#ifndef __MAP_SERVER_H__
#define __MAP_SERVER_H__

#include "server.hpp"

// This creates the map server that will provide tiles and other assets required by mapbox client when displaying a map.
// The server expects a file name config.xml in one or more of the specified root folders.
// The server provides two end-point types: assets and tiles
//
// assets are general files stored raw or inside an sqlite database,(global assets such as fonts, glyphs, js, css) or map specific assets such as styles
// http://localhost:<port>/<key>/* (map specific assets such as style file)
// these are specified using <assets src=<folder or sqlite file> key=<key>
// For GPX, KML assets the server supports conversion to GeoJson by appending ?cnv=geojson to the request URL.
//
// tiles are vector or raster tiles with end point:
// http://localhost:<port>/<key>/<z>/<x>/<y>.(pbf|png)
// the corresponding config is:
// <tiles type="raster" src=<jpeg2000 raster file> key=<key> />
// <tiles type="vector" src=<folder or mbtiles database> key=<key> />
// <tiles type="gl" src=<folder or mbtiles database> key=<key> config=<gl config path> />
// all paths are relative to the xml file


class MapServer: public http::Server {
public:
    MapServer(const std::string &root_list, const std::string &ports) ;
};

#endif
