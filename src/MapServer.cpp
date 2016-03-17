#include "MapServer.h"

#include "HttpServer.h"
#include "Database.h"
#include "base64.h"

#include <boost/regex.hpp>
#include <boost/range/iterator_range.hpp>

#include <time.h>

#include <fstream>

using namespace std ;
namespace fs = boost::filesystem ;

extern void gmt_time_string(char *buf, size_t buf_len, time_t *t) ;

/////////////////////////////////////////////////////////////////////

class TileRequestHandler: public HttpRequestHandler {
public:

    TileRequestHandler(const string &map_id, const string &tileSet) ;
    void respond(const HttpServerRequest &request, HttpServerResponse &resp) ;

private:

    std::unique_ptr<SQLite::Database> db_ ;
    boost::filesystem::path tileset_ ;
    string map_id_ ;
};


TileRequestHandler::TileRequestHandler(const string &id, const string &tileSet): tileset_(tileSet), map_id_(id)
{
    if ( !fs::is_directory(tileset_) )
        db_.reset(new SQLite::Database(tileset_.native())) ;
}

void TileRequestHandler::respond(const HttpServerRequest &request, HttpServerResponse &resp) {

    boost::regex r(R"(/map/[^/]+/tiles/(\d+)/(\d+)/(\d+)\.pbf)") ;

    boost::smatch m ;
    boost::regex_match(request._SERVER.get("REQUEST_URI"), m, r) ;

    int zoom = stoi(m.str(1)) ;
    int tx = stoi(m.str(2)) ;
    int ty = stoi(m.str(3)) ;

    ty = pow(2, zoom) - 1 - ty ;

    if ( !fs::is_directory(tileset_) ) {

        SQLite::Session session(db_.get()) ;
        SQLite::Connection &con = session.handle() ;

        try {
            SQLite::Query q(con, "SELECT tile_data FROM tiles WHERE zoom_level=? AND tile_column=? AND tile_row=?") ;

            q.bind(zoom) ;
            q.bind(tx) ;
            q.bind(ty) ;

            SQLite::QueryResult res = q.exec() ;

            resp.setHeader("Content-Encoding", "gzip") ;
            resp.setHeader("Content-Type", "application/x-protobuf") ;
            resp.setHeader("Connection", "keep-alive") ;
            resp.setHeader("Access-Control-Allow-Origin", "*") ;

            char ctime_buf[64], mtime_buf[64] ;
            time_t curtime = time(NULL), modtime = boost::filesystem::last_write_time(tileset_);

            gmt_time_string(ctime_buf, sizeof(ctime_buf), &curtime);
            gmt_time_string(mtime_buf, sizeof(mtime_buf), &modtime);

            resp.setHeader("Date", ctime_buf) ;
            resp.setHeader("Last-Modified", mtime_buf) ;

            ostringstream etag ;
            etag << modtime  ;
            resp.setHeader("Etag", etag.str()) ;

            if ( res ) {
                int blobsize ;
                const char *data = res.getBlob(0, blobsize) ;

                ostringstream cl ;
                cl << blobsize ;
                resp.setHeader("Content-Length", cl.str()) ;

                resp.write(data, blobsize) ;
            }
            else {
                string empty_tile = base64_decode("H4sIAAAAAAAAAwMAAAAAAAAAAAA=") ;

                ostringstream cl ;
                cl << empty_tile.size() ;
                resp.setHeader("Content-Length", cl.str()) ;

                resp.write(empty_tile.data(), empty_tile.size()) ;
            }

        }
        catch ( SQLite::Exception &e )
        {
            resp.setCode(500) ;
            cerr << e.what() << endl ;
        }
    }
    else {
        fs::path tile_path(tileset_) ;
        tile_path /= to_string(zoom) ;
        tile_path /= to_string(tx) ;
        tile_path /= to_string(ty) + ".pbf" ;

        resp.setHeader("Content-Encoding", "gzip") ;
        resp.setHeader("Content-Type", "application/x-protobuf") ;
        resp.setHeader("Connection", "keep-alive") ;
        resp.setHeader("Access-Control-Allow-Origin", "*") ;

        if ( fs::exists( tile_path ) )
        {
            char ctime_buf[64], mtime_buf[64] ;
            time_t curtime = time(NULL), modtime = boost::filesystem::last_write_time(tile_path.native());

            gmt_time_string(ctime_buf, sizeof(ctime_buf), &curtime);
            gmt_time_string(mtime_buf, sizeof(mtime_buf), &modtime);

            resp.setHeader("Date", ctime_buf) ;
            resp.setHeader("Last-Modified", mtime_buf) ;

            ostringstream etag ;
            etag << modtime  ;
            resp.setHeader("Etag", etag.str()) ;

            ifstream istr(tile_path.native().c_str(), ios::binary) ;
            string data = string(std::istreambuf_iterator<char>(istr), std::istreambuf_iterator<char>());

            ostringstream cl ;
            cl << data.size() ;
            resp.setHeader("Content-Length", cl.str()) ;

            resp.write(data.data(), data.size()) ;
        }
        else {
            string empty_tile = base64_decode("H4sIAAAAAAAAAwMAAAAAAAAAAAA=") ;

            ostringstream cl ;
            cl << empty_tile.size() ;
            resp.setHeader("Content-Length", cl.str()) ;

            resp.write(empty_tile.data(), empty_tile.size()) ;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////


class ResourceHandler: public HttpRequestHandler {
public:

    ResourceHandler(const string &url_prefix, const std::string &rsdb) ;
    void respond(const HttpServerRequest &request, HttpServerResponse &resp) ;

private:

    std::unique_ptr<SQLite::Database> db_ ;
    boost::filesystem::path rsdb_ ;
    string url_prefix_ ;
};


ResourceHandler::ResourceHandler(const string &url_prefix, const std::string &rs): rsdb_(rs), url_prefix_(url_prefix)
{
    if ( !fs::is_directory(rsdb_) )
        db_.reset(new SQLite::Database(rsdb_.native())) ;
}

void ResourceHandler::respond(const HttpServerRequest &request, HttpServerResponse &resp) {

    boost::regex r("/" + url_prefix_ + "/(.*)") ;

    boost::smatch m ;
    boost::regex_match(request._SERVER.get("REQUEST_URI"), m, r) ;

    string key = m.str(1) ;

    if ( db_ ) {

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

                resp.setHeader("Content-Encoding", "gzip") ;
                resp.setHeader("Connection", "keep-alive") ;
                resp.setHeader("Access-Control-Allow-Origin", "*") ;

                ostringstream clen ;
                clen << bs  ;
                resp.setHeader("Content-Length", clen.str()) ;

                resp.write(blob, bs) ;
            }
            else {
                resp.setCode(404) ;
            }
        }
        catch ( SQLite::Exception &e )
        {
            resp.setCode(500) ;
            cerr << e.what() << endl ;
        }
    }
    else {
        fs::path file(rsdb_ / key) ;

        if ( fs::exists(file) ) {

//            resp.setHeader("Content-Encoding", "gzip") ;
//            resp.setHeader("Content-Type", "application/x-protobuf") ;
            resp.setHeader("Connection", "keep-alive") ;
            resp.setHeader("Access-Control-Allow-Origin", "*") ;

                char ctime_buf[64], mtime_buf[64] ;
                time_t curtime = time(NULL), modtime = boost::filesystem::last_write_time(file.native());

                gmt_time_string(ctime_buf, sizeof(ctime_buf), &curtime);
                gmt_time_string(mtime_buf, sizeof(mtime_buf), &modtime);

                resp.setHeader("Date", ctime_buf) ;
                resp.setHeader("Last-Modified", mtime_buf) ;

                ostringstream etag ;
                etag << modtime  ;
                resp.setHeader("Etag", etag.str()) ;

                ifstream istr(file.native().c_str(), ios::binary) ;
                string data = string(std::istreambuf_iterator<char>(istr), std::istreambuf_iterator<char>());

                ostringstream cl ;
                cl << data.size() ;
                resp.setHeader("Content-Length", cl.str()) ;

                resp.write(data.data(), data.size()) ;
        }
        else
        {
            resp.setCode(404) ;
        }
    }
}

MapServer::MapServer(const string &rootFolder, const string &ports): HttpServer(ports.c_str()) {

    fs::path rp(rootFolder) ;

    // parse maps

    if ( fs::exists(rp) && fs::exists(rp / "maps") && fs::is_directory(rp / "maps") ) {

        for( auto &de: boost::make_iterator_range(fs::directory_iterator(rp / "maps"), {} ) )
        {
            if ( fs::is_directory(de.path()) ) {
                string name = de.path().stem().string() ;

                string map_source, assets_source ;

                // tiles

                if ( fs::exists( de.path() / "tiles.mbtiles") )
                    map_source = ( de.path() / "tiles.mbtiles" ).native() ;
                else if ( fs::is_directory(de.path() / "tiles") )
                    map_source = ( de.path() / "tiles" ).native() ;

                if ( !map_source.empty() ) {
                    string rx =  "/map/" + name + R"(/tiles/\d+/\d+/\d+\.pbf)" ;
                    addHandler(std::shared_ptr<HttpRequestHandler>(new TileRequestHandler(name, map_source)), boost::regex(rx)) ;
                }

                // map assets

                if ( fs::exists(de.path() / "assets.sqlite"))
                    assets_source = ( de.path() / "assets.sqlite" ).native() ;
                else if ( fs::is_directory(de.path() / "assets") )
                    assets_source = ( de.path() / "assets" ).native() ;

                if ( !assets_source.empty() ) {
                    string rx =  "/map/" + name + "/(.*)" ;
                    addHandler(std::shared_ptr<HttpRequestHandler>(new ResourceHandler("map/" + name, assets_source)), boost::regex(rx)) ;
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
        string rx =  "/assets/(.*)" ;
        addHandler(std::shared_ptr<HttpRequestHandler>(new ResourceHandler("assets", global_assets_source)), boost::regex(rx)) ;
    }
}
