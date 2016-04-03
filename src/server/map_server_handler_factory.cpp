#include "map_server_handler_factory.hpp"
#include "request.hpp"

#include <boost/range/iterator_range.hpp>

using namespace std ;
namespace fs = boost::filesystem ;
using namespace http ;

MapServerHandlerFactory::MapServerHandlerFactory(const string &rootFolder)
{
    fs::path rp(rootFolder) ;

    // start the rendering loop ( a configuration file config.xml is expected
    gl_.reset(new GLRenderingLoop(fs::path(rootFolder) / "config.xml")) ;

    // parse maps

    if ( fs::exists(rp) && fs::exists(rp / "maps") && fs::is_directory(rp / "maps") ) {

        for( auto &de: boost::make_iterator_range(fs::directory_iterator(rp / "maps"), {} ) )
        {
            if ( fs::is_directory(de.path()) ) {
                string name = de.path().stem().string() ;

                string assets_source ;

                // tilesets

                fs::path tiles_folder = de.path() / "tiles" ;

                if ( fs::is_directory(tiles_folder) ) {
                    for( auto &ts: boost::make_iterator_range(fs::directory_iterator(tiles_folder), {} ) ) {

                        string tileset_name = ts.path().stem().string() ;
                        string map_source = ts.path().native() ;

                        //                   string rx =  "/map/" + name + "/tiles/" + tileset_name + R"(/\d+/\d+/\d+\.[^/]+)" ;

                        tile_request_handlers_[name + '_' + tileset_name] = make_shared<TileRequestHandler>(name, map_source, gl_) ;
                    }
                }

                // map assets

                if ( fs::exists(de.path() / "assets.sqlite"))
                    assets_source = ( de.path() / "assets.sqlite" ).native() ;
                else if ( fs::is_directory(de.path() / "assets") )
                    assets_source = ( de.path() / "assets" ).native() ;

                if ( !assets_source.empty() ) {
                    //                    string rx =  "/map/" + name + "/(.*)" ;
                    asset_request_handlers_[name] = make_shared<AssetRequestHandler>("map/" + name, assets_source) ;
                }
            }
        }
    }

    // global assets

    string global_assets_source ;
    if ( fs::exists(rp / "assets.sqlite"))
        global_assets_source = ( rp / "assets.sqlite" ).native() ;
    else if ( fs::is_directory(rp / "assets") )
        global_assets_source = ( rp / "assets" ).native() ;

    if ( !global_assets_source.empty() ) {
        asset_request_handlers_["_global_"] = make_shared<AssetRequestHandler>("assets", global_assets_source) ;
    }


    gl_thread_.reset(new std::thread(&GLRenderingLoop::run, gl_)) ;
}


std::shared_ptr<RequestHandler> MapServerHandlerFactory::create(const Request &req) {

    boost::smatch m ;

    if ( boost::regex_match(req.path_, m, TileRequestHandler::uri_pattern_) ) {
        auto handler = tile_request_handlers_.find(m.str(1) + '_' + m.str(2)) ;
        if ( handler == tile_request_handlers_.end() ) return nullptr ;
        else return handler->second ;
    }
    else if ( boost::regex_match(req.path_, m, boost::regex(R"(/map/([^/]+)/assets/.*)") ) ) {
        auto handler = asset_request_handlers_.find(m.str(1)) ;
        if ( handler == asset_request_handlers_.end() ) return nullptr ;
        else return handler->second ;
    }
    else if ( boost::regex_match(req.path_, m, boost::regex(R"(/assets/.*)") ) ) {
        auto handler = asset_request_handlers_.find("_global_") ;
        if ( handler == asset_request_handlers_.end() ) return nullptr ;
        else return handler->second ;
    }
    else return nullptr ;
}
