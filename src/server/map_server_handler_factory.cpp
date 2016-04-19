#include "map_server_handler_factory.hpp"
#include "request.hpp"
#include "raster_request_handler.hpp"
#include "vector_tile_request_handler.hpp"
#include "gl_tile_request_handler.hpp"
#include "pugixml.hpp"
#include "shader_config.hpp"

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

using namespace std ;
namespace fs = boost::filesystem ;
using namespace http ;

MapServerHandlerFactory::MapServerHandlerFactory(const string &root_folders)
{
    // parse maps

    vector<string> tokens ;
    boost::split(tokens, root_folders, boost::is_any_of(";"),boost::algorithm::token_compress_on);

    glsl::ProgramList gl_programs ;

    gl_.reset(new GLRenderingLoop()) ;

    for( string &folder: tokens) {

        fs::path rp(folder) ;

        if ( !fs::exists(rp / "config.xml") ) continue ;

        string cfg_file = ( rp / "config.xml" ).native() ;

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(cfg_file.c_str());

        if ( !result ) {
            LOG_WARN_STREAM("XML [" << cfg_file << "] parsed with errors: " << result.description() ) ;
            continue ;
        }

        for( pugi::xml_node p: doc.children("assets") ) {
            string key = p.attribute("key").as_string() ;
            string src = p.attribute("src").as_string() ;

            if ( key.empty() ) {
                LOG_WARN_STREAM("XML [" << cfg_file << "] error: key attribute missing from assets element" ) ;
                continue ;
            }

            if ( src.empty() ) {
                LOG_WARN_STREAM("XML [" << cfg_file << "] error: src attribute missing from assets element" ) ;
                continue ;
            }


            try {
                fs::path src_path  = fs::canonical(rp / src) ;

                asset_request_handlers_[key] = make_shared<AssetRequestHandler>(key, src_path.native()) ;
            }
            catch ( fs::filesystem_error &e ) {
                LOG_WARN(e.what()) ;
                continue ;
            }

        }

        for( pugi::xml_node p: doc.children("tiles") ) {
            string key = p.attribute("key").as_string() ;
            string src = p.attribute("src").as_string() ;
            string type = p.attribute("type").as_string() ;

            if ( key.empty() ) {
                LOG_WARN_STREAM("XML [" << cfg_file << "] error: key attribute missing from tiles element" ) ;
                continue ;
            }

            if ( src.empty() ) {
                LOG_WARN_STREAM("XML [" << cfg_file << "] error: src attribute missing from tiles element" ) ;
                continue ;
            }

            if ( type.empty() ) {
                LOG_WARN_STREAM("XML [" << cfg_file << "] error: tiles attribute missing from tiles element" ) ;
                continue ;
            }

            try {
                fs::path src_path  = fs::canonical(rp / src) ;
                src = src_path.native() ;
            }
            catch ( fs::filesystem_error &e ) {
                LOG_WARN(e.what()) ;
                continue ;
            }

            if ( type == "raster" )
                tile_request_handlers_[key] = make_shared<RasterRequestHandler>(key, src) ;
            else if ( type == "vector" )
                tile_request_handlers_[key] = make_shared<VectorTileRequestHandler>(key, src) ;
            else if ( type == "gl" ) {
                string prog = p.attribute("config").as_string() ;

                try {
                    prog  = fs::canonical(rp / prog).native() ;

                    if ( prog.empty() || !gl_programs.load(prog) ) {
                        LOG_WARN_STREAM("XML [" << cfg_file << "] error: cannot load OpenGL render programs" ) ;
                        continue ;
                    }
                }
                catch ( fs::filesystem_error &e ) {
                    LOG_WARN(e.what()) ;
                    continue ;
                }

                tile_request_handlers_[key] = make_shared<GLTileRequestHandler>(key, src, gl_) ;
            }

        }

  }

    if ( gl_ )
    gl_thread_.reset(new std::thread(&GLRenderingLoop::run, gl_, gl_programs)) ;

}

#if 0
MapServerHandlerFactory::MapServerHandlerFactory(const string &root_folders)
{
    // parse maps

    vector<string> tokens ;
    boost::split(tokens, root_folders, boost::is_any_of(";"),boost::algorithm::token_compress_on);

    for( string &folder: tokens) {

        fs::path rp(folder) ;

        // start the rendering loop ( a configuration file config.xml is expected
        if ( with_gl && !gl_ && fs::exists(rp / "config.xml") )
            gl_.reset(new GLRenderingLoop(fs::path(rp) / "config.xml")) ;

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
                            string extension = ts.path().extension().string() ;
                            string map_source = ts.path().native() ;

                            if ( fs::is_regular_file(ts.path()) ) {
                                if ( extension == ".jp2" )
                                    tile_request_handlers_[name + '_' + tileset_name] = make_shared<RasterRequestHandler>(name, map_source) ;
                                else if ( extension == ".mbtiles" )
                                    tile_request_handlers_[name + '_' + tileset_name] = make_shared<TileRequestHandler>(name, map_source, gl_) ;
                            }
                            else
                                tile_request_handlers_[name + '_' + tileset_name] = make_shared<TileRequestHandler>(name, map_source, gl_) ;
                        }
                    }

                    // map assets

                    if ( fs::exists(de.path() / "assets.sqlite"))
                        assets_source = ( de.path() / "assets.sqlite" ).native() ;
                    else if ( fs::is_directory(de.path() / "assets") )
                        assets_source = ( de.path() / "assets" ).native() ;

                    if ( !assets_source.empty() ) {
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

    }

    if ( gl_ )
    gl_thread_.reset(new std::thread(&GLRenderingLoop::run, gl_)) ;
}
#endif

std::shared_ptr<RequestHandler> MapServerHandlerFactory::create(const Request &req) {

    boost::smatch m ;

    for( auto handler: tile_request_handlers_ ) {
        if ( handler.second->matches(req.path_) ) return handler.second ;
    }
    for( auto handler: asset_request_handlers_ ) {
        if ( handler.second->matches(req.path_) ) return handler.second ;
    }

    return nullptr ;
}
