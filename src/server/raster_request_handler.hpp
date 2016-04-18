#ifndef __RASTER_REQUEST_HANDLER_HPP__
#define __RASTER_REQUEST_HANDLER_HPP__

#include "request_handler.hpp"
#include "jp2_decoder.hpp"
#include "raster_tile_cache.hpp"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

class RasterRequestHandler: public http::RequestHandler {
public:

    RasterRequestHandler(const std::string &map_id, const std::string &tileSet) ;
    void handle_request(const http::Request &request, http::Response &resp) ;

private:

    boost::filesystem::path tileset_ ;
    std::string map_id_ ;

public:
    JP2Decoder provider_ ;
    static const boost::regex uri_pattern_ ;
    static RasterTileCache tile_cache_ ;
};

#endif
