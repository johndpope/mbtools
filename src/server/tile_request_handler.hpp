#ifndef __TILE_REQUEST_HANDLER_HPP__
#define __TILE_REQUEST_HANDLER_HPP__

#include "request_handler.hpp"
#include "database.hpp"
#include "gl_rendering_loop.hpp"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

class TileRequestHandler: public http::RequestHandler {
public:

    TileRequestHandler(const std::string &map_id, const std::string &tileSet, std::shared_ptr<GLRenderingLoop> &renderer) ;
    void handle_request(const http::Request &request, http::Response &resp) ;

private:

    friend class GLRenderingLoop ;
    std::unique_ptr<SQLite::Database> db_ ;
    boost::filesystem::path tileset_ ;
    std::string map_id_ ;
    std::shared_ptr<GLRenderingLoop> gl_ ;

public:
    static const boost::regex uri_pattern_ ;
};

#endif
