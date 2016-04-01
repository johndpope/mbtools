#include "MapServer.h"

#include "HttpServer.h"
#include "Database.h"
#include "base64.h"

#include <boost/regex.hpp>
#include <boost/range/iterator_range.hpp>

#include <time.h>

#include <fstream>

#include <thread>
#include <future>

#include "Queue.h"

using namespace std ;
namespace fs = boost::filesystem ;
using namespace http::server ;

////////////////////////////////////////////////////////////////////

class OGLRenderingLoop ;

class TileRequestHandler: public request_handler {
public:

    TileRequestHandler(const string &map_id, const string &tileSet, std::shared_ptr<OGLRenderingLoop> &renderer) ;
    void handle_request(const request &request, reply &resp) ;

private:

    friend class OGLRenderingLoop ;
    std::unique_ptr<SQLite::Database> db_ ;
    boost::filesystem::path tileset_ ;
    string map_id_ ;
    std::shared_ptr<OGLRenderingLoop> gl_ ;

public:
    static const boost::regex uri_pattern_ ;
};

const boost::regex TileRequestHandler::uri_pattern_(R"(/map/([^/]+)/tiles/([^/]+)/(\d+)/(\d+)/(\d+)\.([^/]+))") ;

/////////////////////////////////////////////////////////////////////

struct RenderTileJob {
    uint32_t x_, y_, z_ ;
    string bytes_ ;
    std::shared_ptr<std::promise<string>> result_ ;
};


class OGLRenderingLoop {

public:
    OGLRenderingLoop(): is_running_(true) {}

    void run() {

        gl_.reset(new MeshTileRenderer()) ;

        while (is_running_) {
            RenderTileJob job ;
            try {
                job_queue_.pop(job) ;
                string bytes = gl_->render(job.x_, job.y_, job.z_, job.bytes_, "hillshade") ;
                job.result_->set_value(bytes) ;
            }
            catch ( std::exception &) {
                return ;
            }
        }
    }

    shared_ptr<std::promise<string>> addJob(uint32_t x, uint32_t y, uint32_t z, const string &bytes) {

        std::shared_ptr<std::promise<string>> res(new std::promise<string>) ;
        job_queue_.push({x, y, z, bytes, res}) ;
        return res ;
    }

    void stop() {
        is_running_ = false ;
        job_queue_.stop() ;
    }

    Queue<RenderTileJob> job_queue_ ;
    std::shared_ptr<MeshTileRenderer> gl_ ;
    std::atomic<bool> is_running_ ;
};



TileRequestHandler::TileRequestHandler(const string &id, const string &tileSet, std::shared_ptr<OGLRenderingLoop> &renderer):
    tileset_(tileSet), map_id_(id), gl_(renderer)
{
    if ( !fs::is_directory(tileset_) )
        db_.reset(new SQLite::Database(tileset_.native())) ;
}


static bool is_png(const char *bytes, uint32_t len) {
    if ( len < 8 ) return false ;
    return ( bytes[0] == 0x89 ) && ( bytes[1] == 'P' ) && ( bytes[2] == 'N' ) && ( bytes[3] == 'G' ) && ( bytes[4] == 0x0d ) &&
            ( bytes[5] == 0x0a ) && ( bytes[6] == 0x1a ) && ( bytes[7] == 0x0a ) ;
}

static const char *g_empty_transparent_png_256 =
"iVBORw0KGgoAAAANSUhEUgAAAQAAAAEACAYAAABccqhmAAAABGdBTUEAALGPC/xhBQAAAAFzUkdC\
AK7OHOkAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAAAAZiS0dE\
AP8A/wD/oL2nkwAAAAlwSFlzAAAASAAAAEgARslrPgAAARVJREFUeNrtwTEBAAAAwqD1T+1rCKAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHgDATwAAdgpQwQAAAAldEVYdGRhdGU6Y3JlYXRlADIw\
MTYtMDQtMDFUMDk6MTE6MjMrMDM6MDCHKZUkAAAAJXRFWHRkYXRlOm1vZGlmeQAyMDE2LTA0LTAx\
VDA5OjExOjIzKzAzOjAw9nQtmAAAAABJRU5ErkJggg==" ;

void TileRequestHandler::handle_request(const request &request, reply &resp)
{
    boost::smatch m ;
    boost::regex_match(request.path_, m, uri_pattern_) ;

    int zoom = stoi(m.str(3)) ;
    int tx = stoi(m.str(4)) ;
    int ty = stoi(m.str(5)) ;
    string extension = m.str(6) ;

    ty = pow(2, zoom) - 1 - ty ;

    string encoding, mime ;

    if ( extension == "pbf" ) {
        encoding = "gzip" ;
        mime = "application/x-protobuf" ;
    }
    else if ( extension == "png" )
        mime = "image/png" ;

    // since we do not store timestamps per tile we use the modification time of the tileset
    time_t mod_time = boost::filesystem::last_write_time(tileset_.native());

    if ( !fs::is_directory(tileset_) ) {

        SQLite::Session session(db_.get()) ;
        SQLite::Connection &con = session.handle() ;

        try {
            SQLite::Query q(con, "SELECT tile_data FROM tiles WHERE zoom_level=? AND tile_column=? AND tile_row=?") ;

            q.bind(zoom) ;
            q.bind(tx) ;
            q.bind(ty) ;

            SQLite::QueryResult res = q.exec() ;

            //            resp.setHeader("Connection", "keep-alive") ;


            if ( res ) {
                int blobsize ;
                const char *data = res.getBlob(0, blobsize) ;

                string content ;

                if ( extension == "pbf" || is_png(data, blobsize) )
                    content.assign(data, data + blobsize) ;
                else {
                    // this is probably a mesh tile and we need to render it to PNG
                    // we have to add a job to the OpenGL rendering loop and wait for it to become ready
                    content = gl_->addJob(tx, ty, zoom, std::string(data, blobsize))->get_future().get() ;
                }

                resp.encode_file_data(content, encoding, mime, mod_time) ;
            }
            else {
                string empty_tile ;

                if ( extension == "pbf") // empty vector tile payload
                    empty_tile = base64_decode("H4sIAAAAAAAAAwMAAAAAAAAAAAA=") ;
                else if ( extension == "png" ) // empty png image payload
                    empty_tile = base64_decode(g_empty_transparent_png_256) ;

                if ( empty_tile.empty() )
                    resp = reply::stock_reply(reply::not_implemented) ;
                else
                    resp.encode_file_data(empty_tile, encoding, mime, mod_time) ;
            }

        }
        catch ( SQLite::Exception &e )
        {
            resp = reply::stock_reply(reply::internal_server_error) ;
            cerr << e.what() << endl ;
        }
    }
    else { // tiles are stored int the filesystem as seperate files

        fs::path tile_path(tileset_) ;
        tile_path /= to_string(zoom) ;
        tile_path /= to_string(tx) ;
        tile_path /= to_string(ty) + "." + extension ;

//        resp.setHeader("Connection", "keep-alive") ;
//        resp.setHeader("Access-Control-Allow-Origin", "*") ;

        if ( fs::exists( tile_path ) )
            resp.encode_file(tile_path.native(), encoding, mime) ;
        else {
            string empty_tile ;

            if ( extension == "pbf") // empty vector tile payload
                empty_tile = base64_decode("H4sIAAAAAAAAAwMAAAAAAAAAAAA=") ;
            else if ( extension == "png" ) // empty png image payload
                empty_tile = base64_decode(g_empty_transparent_png_256) ;

            if ( empty_tile.empty() )
                resp = reply::stock_reply(reply::not_implemented) ;
            else
                resp.encode_file_data(empty_tile, encoding, mime, mod_time) ;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////


class AssetRequestHandler: public request_handler {
public:

    AssetRequestHandler(const string &url_prefix, const std::string &rsdb) ;
    void handle_request(const request &request, reply &resp) ;

private:

    std::unique_ptr<SQLite::Database> db_ ;
    boost::filesystem::path rsdb_ ;
    string url_prefix_ ;
};


AssetRequestHandler::AssetRequestHandler(const string &url_prefix, const std::string &rs): rsdb_(rs), url_prefix_(url_prefix)
{
    if ( !fs::is_directory(rsdb_) )
        db_.reset(new SQLite::Database(rsdb_.native())) ;
}

void AssetRequestHandler::handle_request(const request &req, reply &resp) {

    boost::regex r("/" + url_prefix_ + "/(.*)") ;

    boost::smatch m ;
    boost::regex_match(req.path_, m, r) ;

    string key = m.str(1) ;

    if ( db_ ) {

        // since we do not store timestamps per tile we use the modification time of the tileset
        time_t mod_time = boost::filesystem::last_write_time(rsdb_.native());

        SQLite::Session session(db_.get()) ;
        SQLite::Connection &con = session.handle() ;

        try {
            SQLite::Query stmt(con, "SELECT data FROM resources WHERE name=?") ;
            stmt.bind(key) ;

            SQLite::QueryResult res = stmt.exec() ;

            if ( res )
            {
                int bs ;
                const char *blob = res.getBlob(0, bs) ;

                string content(blob, blob + bs) ;
                resp.encode_file_data(content, "gzip", string(), mod_time) ;
            }
            else {
                resp = reply::stock_reply(reply::not_found) ;
            }
        }
        catch ( SQLite::Exception &e )
        {
            resp = reply::stock_reply(reply::internal_server_error) ;
            cerr << e.what() << endl ;
        }
    }
    else {
        fs::path file(rsdb_ / key) ;

        if ( fs::exists(file) )
            resp.encode_file(file.native(), string(), string()) ;
        else
            resp = reply::stock_reply(reply::not_found) ;

    }
}



class MapServerHandlerFactory: public request_handler_factory {
public:

    MapServerHandlerFactory(const string &rootFolder)  ;
    ~MapServerHandlerFactory() {
        shutdown_gl_loop() ;
    }
    std::shared_ptr<request_handler> create(const request &req) ;

private:

    void shutdown_gl_loop() {
        gl_->stop() ;
        gl_thread_->join() ;
    }

    std::map<std::string, std::shared_ptr<TileRequestHandler> > tile_request_handlers_ ;
    std::map<std::string, std::shared_ptr<AssetRequestHandler> > asset_request_handlers_ ;
    std::shared_ptr<OGLRenderingLoop> gl_ ;
    std::unique_ptr<std::thread> gl_thread_ ;

};


MapServerHandlerFactory::MapServerHandlerFactory(const string &rootFolder)
{
    fs::path rp(rootFolder) ;

    gl_.reset(new OGLRenderingLoop()) ;

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
        //     string rx =  "/assets/(.*)" ;
        asset_request_handlers_["_global_"] = make_shared<AssetRequestHandler>("assets", global_assets_source) ;
    }


    gl_thread_.reset(new std::thread(&OGLRenderingLoop::run, gl_)) ;
}


std::shared_ptr<request_handler> MapServerHandlerFactory::create(const request &req) {

    boost::smatch m ;

    if ( boost::regex_match(req.path_, m, TileRequestHandler::uri_pattern_) ) {
        cout << m.str(1) + '_' + m.str(2) << endl ;
        auto handler = tile_request_handlers_.find(m.str(1) + '_' + m.str(2)) ;
        if ( handler == tile_request_handlers_.end() ) return nullptr ;
        else return handler->second ;
    }
    else if ( boost::regex_match(req.path_, m, boost::regex(R"(/map/([^/]+)/assets/.*)") ) ) {
        cout << m.str(1) << endl ;
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


MapServer::MapServer(const string &rootFolder, const string &ports):
    http::server::server(std::make_shared<MapServerHandlerFactory>(rootFolder), "127.0.0.1", ports, 4) {
}
