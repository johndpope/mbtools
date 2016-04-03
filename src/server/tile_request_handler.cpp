#include "tile_request_handler.hpp"
#include "request.hpp"
#include "reply.hpp"
#include "base64.hpp"

using namespace std ;
namespace fs = boost::filesystem ;
using namespace http ;

const boost::regex TileRequestHandler::uri_pattern_(R"(/map/([^/]+)/tiles/([^/]+)/(\d+)/(\d+)/(\d+)\.([^/]+))") ;

TileRequestHandler::TileRequestHandler(const string &id, const string &tileSet, std::shared_ptr<GLRenderingLoop> &renderer):
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

void TileRequestHandler::handle_request(const Request &request, Response &resp)
{
    boost::smatch m ;
    boost::regex_match(request.path_, m, uri_pattern_) ;

    int zoom = stoi(m.str(3)) ;
    int tx = stoi(m.str(4)) ;
    int ty = stoi(m.str(5)) ;
    string extension = m.str(6) ;

    Dictionary options ;
    if ( request.method_ == "GET" )
        options = request.GET_ ;
    else if ( request.method_ == "POST" )
        options = request.POST_ ;

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
                    content = gl_->addJob(tx, ty, zoom,
                                          std::string(data, blobsize), options)->get_future().get() ;
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
                    resp = Response::stock_reply(Response::not_implemented) ;
                else
                    resp.encode_file_data(empty_tile, encoding, mime, mod_time) ;
            }

        }
        catch ( SQLite::Exception &e )
        {
            resp = Response::stock_reply(Response::internal_server_error) ;
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
                resp = Response::stock_reply(Response::not_implemented) ;
            else
                resp.encode_file_data(empty_tile, encoding, mime, mod_time) ;
        }
    }
}
