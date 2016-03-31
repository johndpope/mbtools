#ifndef __MAP_SERVER_H__
#define __MAP_SERVER_H__

#include "HttpServer.h"
#include "Database.h"
#include "MeshTileRenderer.h"

// This creates the map server that will provide tiles and other assets required by mapbox client when displaying a map
// The server responds to the following endpoints:
//
// http://localhost:<port>/map/<mapname>/* (map specific assets such as style file)
// http://localhost:<port>/map/<mapname>/tiles/<tileset>/*/*/*.pbf (vector tiles)
// http://localhost:<port>/assets/* (global assets such as fonts, glyphs, js, css)

// On the local filesystem the server expects a fixed structure of file/folder under a root directory provided in the constructor
//
// <root>/maps/
// <root>/maps/<map1>/, <root>/maps/<map2>/ ...
// <root>/maps/<map1>/tiles/<tileset>/<z>/<x>/<y>.(pbf|png) or <root>/maps/<map1>/<tileset>.mbtiles
// <root>/maps/<map1>/assets/* or <root>/maps/<map1>/assets.sqlite
// <root>/assets/* or <root>/assets.sqlite

class OGLRenderingLoop ;

class MapServer: public HttpServer {
public:
    MapServer(const std::string &root, const std::string &ports) ;

private:
    std::shared_ptr<OGLRenderingLoop> gl_ ;
};

#endif
