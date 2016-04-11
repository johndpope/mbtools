#include "asset_request_handler.hpp"
#include "request.hpp"
#include "reply.hpp"
#include "gpx_reader.hpp"
#include "kml_reader.hpp"
#include "logger.hpp"

#include <zlib.h>
#include <boost/regex.hpp>

using namespace std ;
namespace fs = boost::filesystem ;
using namespace http ;

#define windowBits 15
#define GZIP_ENCODING 16

static string compress(const string &bytes)
{
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if ( deflateInit2 (&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                       windowBits | GZIP_ENCODING, 8,
                       Z_DEFAULT_STRATEGY) != Z_OK )
        return string() ;

    zs.next_in = (Bytef*)bytes.data() ;
    zs.avail_in = bytes.size() ;

    int ret;
    char outbuffer[32768];
    std::string outstring;

    // retrieve the compressed bytes blockwise
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            // append the block to the output string
            outstring.append(outbuffer,
                             zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    return outstring ;
}

static string uncompress(const string &str)
{
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK)
        return string() ;

    zs.next_in = (Bytef*)str.data();
    zs.avail_in = str.size();

    int ret;
    char outbuffer[32768];

    std::string outstring;

    // get the decompressed bytes blockwise using repeated calls to inflate

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) return string() ;

    return outstring;
}


AssetRequestHandler::AssetRequestHandler(const string &url_prefix, const std::string &rs): rsdb_(rs), url_prefix_(url_prefix)
{
    if ( !fs::is_directory(rsdb_) )
        db_.reset(new SQLite::Database(rsdb_.native())) ;
}

void AssetRequestHandler::handle_request(const Request &req, Response &resp) {

    boost::regex r("/" + url_prefix_ + "/(.*)") ;

    boost::smatch m ;
    boost::regex_match(req.path_, m, r) ;

    string key = m.str(1) ;

    string cnv = req.GET_["cnv"] ;

    LOG_INFO_STREAM("Recieved request for " << req.path_) ;

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

                if ( cnv.empty() )
                    resp.encode_file_data(content, "gzip", string(), mod_time) ;
                else if ( cnv == "geojson" ) {
                    string uc = uncompress(content) ;
                    geojson::FeatureCollection col ;
                    if ( GPXReader::load_from_string(uc, col) ) {
                        resp.encode_file_data(compress(col.toGeoJSON()), "gzip", string(), mod_time) ;
                    }
                    else if ( KMLReader::load_from_string(uc, col) ) {
                        resp.encode_file_data(compress(col.toGeoJSON()), "gzip", string(), mod_time) ;
                    }
                    else
                        resp = Response::stock_reply(Response::not_found) ;

                }
            }
            else {
                resp = Response::stock_reply(Response::not_found) ;
            }
        }
        catch ( SQLite::Exception &e )
        {
            resp = Response::stock_reply(Response::internal_server_error) ;
            cerr << e.what() << endl ;
        }
    }
    else {
        fs::path file(rsdb_ / key) ;

        if ( fs::exists(file) ) {

            time_t mod_time = boost::filesystem::last_write_time(file.native());

            if ( cnv.empty() )
               resp.encode_file(file.native(), string(), string()) ;
            else if ( cnv == "geojson" ) {
                geojson::FeatureCollection col ;
                if ( GPXReader::load_from_file(file.native(), col) ) {
                    resp.encode_file_data(compress(col.toGeoJSON()), "gzip", string(), mod_time) ;
                }
                else if ( KMLReader::load_from_file(file.native(), col) ) {
                    resp.encode_file_data(compress(col.toGeoJSON()), "gzip", string(), mod_time) ;
                }
                else
                    resp = Response::stock_reply(Response::not_found) ;
            }
        }
        else
            resp = Response::stock_reply(Response::not_found) ;

    }
}

