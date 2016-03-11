#include "HttpServer.h"
#include "Database.h"
#include "base64.h"
#include <boost/regex.hpp>

#include <time.h>

using namespace std ;

extern void gmt_time_string(char *buf, size_t buf_len, time_t *t) ;

class TileRequestHandler: public HttpRequestHandler {
public:

    TileRequestHandler(const std::string &tileSet) ;
    void respond(const HttpServerRequest &request, HttpServerResponse &resp) ;

private:

    std::unique_ptr<SQLite::Database> db_ ;
    boost::filesystem::path tileset_ ;
};


TileRequestHandler::TileRequestHandler(const std::string &tileSet): tileset_(tileSet)
{
    db_.reset(new SQLite::Database(tileSet)) ;
}

void TileRequestHandler::respond(const HttpServerRequest &request, HttpServerResponse &resp) {

    SQLite::Session session(db_.get()) ;
    SQLite::Connection &con = session.handle() ;

    try {
        boost::regex r(R"(/tiles/(\d+)/(\d+)/(\d+)\.vector\.pbf)") ;

        boost::smatch m ;
        boost::regex_match(request._SERVER.get("REQUEST_URI"), m, r) ;

        int zoom = stoi(m.str(1)) ;
        int tx = stoi(m.str(2)) ;
        int ty = stoi(m.str(3)) ;

        ty = pow(2, zoom) - 1 - ty ;

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

int main(int argc, char *argv[]) {

    HttpServer server("5000") ;

    TileRequestHandler tileHandler("/home/malasiot/tmp/oo.mbtiles") ;

    boost::regex r(R"(/tiles/\d+/\d+/\d+\.vector\.pbf)") ;
    server.addHandler(tileHandler, r) ;

    server.start() ;

    while (1) ;

}
