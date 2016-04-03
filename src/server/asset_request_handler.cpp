#include "asset_request_handler.hpp"
#include "request.hpp"
#include "reply.hpp"
#include <boost/regex.hpp>

using namespace std ;
namespace fs = boost::filesystem ;
using namespace http ;

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

        if ( fs::exists(file) )
            resp.encode_file(file.native(), string(), string()) ;
        else
            resp = Response::stock_reply(Response::not_found) ;

    }
}

