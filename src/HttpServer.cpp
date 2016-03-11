#include "HttpServer.h"
#include "Dictionary.h"

#include "mongoose/mongoose.h"

#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>
#include <boost/iostreams/stream.hpp>


#include <time.h>

using namespace std ;
namespace fs = boost::filesystem ;

fs::path getTemporaryPath(const std::string &dir, const std::string &prefix, const std::string &ext)
{
    std::string retVal ;

    fs::path directory ;

    if ( ! dir.empty() ) directory = dir;
    else directory = boost::filesystem::temp_directory_path() ;

    std::string varname ="%%%%-%%%%-%%%%-%%%%";

    if ( !prefix.empty() )
        directory /= prefix + '-' + varname + '.' + ext ;
    else
        directory /= "tmp-" + varname + '.' + ext ;

    boost::filesystem::path temp = boost::filesystem::unique_path(directory);

    return temp;
}

/////////////////////////////////////////////////////////////////////////////

static int hex_decode(char c)
{
    char ch = tolower(c) ;

    if ( ch >= 'a' && ch <= 'f' ) return 10 + ch - 'a' ;
    else if ( ch >= '0' && ch <= '9' ) return ch - '0' ;
    else return 0 ;
}

std::string url_decode(const char *str)
{
    const char *p = str ;

    std::string ret ;
    while ( *p )
    {
        if( *p == '+' ) ret += ' ' ;
        else if ( *p == '%' )
        {
            ++p ;
            char tmp[4];
            unsigned char val = 16 * ( hex_decode(*p++) )  ;
            val += hex_decode(*p) ;
            sprintf(tmp,"%c", val);
            ret += tmp ;
        } else ret += *p ;
        ++p ;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////
/// \brief The mg_source struct
///
struct mg_source {
    typedef char char_type;
    typedef boost::iostreams::source_tag category;

    mg_source(mg_connection *cnx): cnx_(cnx) {}

    std::streamsize read(char_type * s, std::streamsize n) {
        return mg_read(cnx_, s, n) ;
    }

private:
    mg_connection *cnx_ ;
};

struct mg_sink {
    typedef char char_type;
    typedef boost::iostreams::sink_tag category;

    mg_sink(mg_connection *cnx): cnx_(cnx) {}

    std::streamsize write(const char_type * s, std::streamsize n) {
        return mg_write(cnx_, s, n) ;
    }
private:
    mg_connection *cnx_ ;
};

class HttpServerRequestStream : public boost::iostreams::stream<mg_source>
{
public:
    HttpServerRequestStream(mg_connection *conn):
        boost::iostreams::stream<mg_source>(conn) {}
} ;

class HttpServerResponseStream : public boost::iostreams::stream<mg_sink>
{
public:
    HttpServerResponseStream(mg_connection *conn):
        boost::iostreams::stream<mg_sink>(conn) {}
} ;


/////////////////////////////////////////////////////////////////////////////

HttpServerResponse::HttpServerResponse(): code_(200)
{

}

void HttpServerResponse::setHeader(const string &key, const string &value)
{
    headers_.add(key, value) ;
}

void HttpServerResponse::setCode(int code)
{
    code_ = code ;
}

const char *well_known_mime_types[] = {
    "pdf", "application/pdf",
    "ps",  "application/postscript",
    "xml", "application/xml",
    "zip", "application/zip",
    "gz",  "application/gzip",
    "gif", "image/gif",
    "jpg", "image/jpeg",
    "jpeg", "image/jpeg",
    "png", "image/png",
    "svg", "image/svg+xml",
    "tif", "image/tiff",
    "tiff", "image/riff",
    "txt",  "text/plain",
    "kml",  "application/vnd.google-earth.kml+xml",
    "kmz",  "application/vnd.google-earth.kmz",
    "gpx",  "application/gpx+xml"
};

#ifndef _WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

static string get_file_mime(const string &mime,  const boost::filesystem::path &p)
{
    if ( !mime.empty() ) return mime ;

    string extension = p.extension().string() ;

    if ( extension == ".gz" )
        extension = p.stem().extension().string() ;

    if ( !extension.empty() )
    {
        for( int i=0 ; i<sizeof(well_known_mime_types)/sizeof(char *) ; i+=2 )
        {
            if ( stricmp(extension.c_str() + 1, well_known_mime_types[i]) == 0 )
                return well_known_mime_types[i+1] ;
        }
    }

    return "application/octet-stream" ;
}
/*
static void construct_etag(char *buf, size_t buf_len,
                           const struct file *filep) {
  snprintf(buf, buf_len, "\"%lx.%" INT64_FMT "\"",
           (unsigned long) filep->modification_time, filep->size);
}
*/

// range headers requests are not supported

extern void gmt_time_string(char *buf, size_t buf_len, time_t *t) {
    strftime(buf, buf_len, "%a, %d %b %Y %H:%M:%S GMT", gmtime(t));
}

void HttpServerResponse::sendFileContents(const HttpServerRequest &req, const boost::filesystem::path &p, const string &mime)
{
    string fmime = get_file_mime(mime, p) ;

    if ( p.extension() == ".gz" )
        setHeader("Content-Encoding", "gzip") ;

    setHeader("Content-Type", fmime) ;
    setHeader("Accept-Ranges", "bytes") ;
    setHeader("Connection", "close") ;

    char ctime_buf[64], mtime_buf[64] ;
    time_t curtime = time(NULL), modtime = fs::last_write_time(p);
    uint64_t filesize = fs::file_size(p) ;

    gmt_time_string(ctime_buf, sizeof(ctime_buf), &curtime);
    gmt_time_string(mtime_buf, sizeof(mtime_buf), &modtime);

    setHeader("Date", ctime_buf) ;
    setHeader("Last-Modified", mtime_buf) ;

    ostringstream etag ;
    etag << modtime  ;
    setHeader("Etag", etag.str()) ;

    if ( req._SERVER["REQUEST_METHOD"] != "HEAD" )
    {
        ifstream src(p.native().c_str(), ios::binary) ;

        (*this) << src.rdbuf();
    }
}

void HttpServerResponse::dumb(std::ostream &strm)
{
    string body = this->str() ;

    if ( ! headers_.contains("Content-Length") ) {
        ostringstream length;
        length << body.size();
        setHeader("Content-Length", length.str());
    }

    // write headers

    strm << "HTTP/1.1 " ;

    switch ( code_ )
    {
        case 200:
            strm << "200 OK" ;
            break ;
        case 400:
            strm << "400 Bad HttpServerRequest" ;
            break ;
        case 401:
            strm << "401 Unauthorized" ;
            break ;
        case 402:
            strm << "403 Forbidden" ;
            break ;
        case 403:
            strm << "404 Not Found" ;
            break ;
        default:
            strm << code_ ;
    }

    strm << "\r\n" ;

    DictionaryIterator it(headers_) ;

    while ( it )
    {
        strm << it.key() << ": " << it.value() << "\r\n" ;
        ++it ;
    }

    strm << "\r\n" ;

    strm.write(body.c_str(), body.size());
}

void HttpServerResponse::setCookie(const string &key, const string &value)
{
   ostringstream definition;
   definition << key << "=" << value << "; path=/";

   setHeader("Set-cookie", definition.str());
}


/////////////////////////////////////////////////////////////////////////////

struct HandlerInfo {
    string req_method_ ;
    boost::regex req_uri_filter_ ;
    std::shared_ptr<HttpRequestHandler> handler_ ;
};

class HttpServerImpl
{
public:

    HttpServerImpl()
    {

    }

    ~HttpServerImpl() {
        ctx = 0 ;
    }

    static int mg_begin_request(mg_connection *conn);

    bool start(HttpServer *srv, const Dictionary &serverOptions) {
        char *options[100] ;
        char buf[8164] ;

        int c = 0 ;
        char *p = buf ;
        DictionaryIterator it(serverOptions) ;

        while ( it )
        {
            string k = it.key(), v = it.value() ;
            strcpy(p, k.c_str()) ;
            options[c++] = p ;
            p += k.length() ; *p++ = 0 ;
            strcpy(p, v.c_str()) ;
            options[c++] = p ;
            p += v.length() ; *p++ = 0 ;
            ++it ;
        }
        options[c] = 0 ;

        mg_callbacks callbacks ;
        memset(&callbacks, 0, sizeof(callbacks));

        callbacks.begin_request = &mg_begin_request ;

        ctx = mg_start(&callbacks, srv, (const char **) options);


        return ( ctx != NULL ) ;
    }

    void stop() {
        if ( ctx ) mg_stop(ctx) ;
    }

    struct mg_context *ctx ;
    char ports[100] ;
    vector<HandlerInfo> handlers_ ;
} ;

static bool parseVariables(const mg_request_info *request_info, HttpServerRequest &session)
{

    for(int i=0 ; i<request_info->num_headers ; i++ )
    {
        const char *name = request_info->http_headers[i].name ;
        const char *value = request_info->http_headers[i].value ;

        session._SERVER[name] = value ;
    }

    session._SERVER["REQUEST_METHOD"] =	request_info->request_method ;
    session._SERVER["REQUEST_URI"] = request_info->uri ;
    if ( request_info->query_string )
        session._SERVER["QUERY_STRING"] = request_info->query_string ;

    return true ;

}

static bool parseQueryString(const mg_request_info *request_info, HttpServerRequest &session)
{
    std::string queryStr ;

    if ( request_info->query_string )
        queryStr = request_info->query_string ;


    if ( !queryStr.empty() )
    {
        std::vector<std::string> args ;

        boost::split(args, queryStr, boost::is_any_of("&"), boost::algorithm::token_compress_on);

        for(int i=0 ; i<args.size() ; i++ )
        {
            std::string &arg = args[i] ;

            int pos = arg.find('=') ;

            if ( pos > 0 )
            {
                std::string key = arg.substr((int)0, (int)pos) ;
                std::string val = arg.substr((int)pos+1, -1) ;

                session._GET[url_decode(key.c_str())] = ( val.empty() ) ? "" : url_decode(val.c_str()) ;
            }
            else if ( pos == -1 )
                session._GET[url_decode(arg.c_str())] = "" ;
        }
    }

    return true ;

}

static bool parseCookie(HttpServerRequest &session, const std::string &data)
{
    int pos = data.find("=") ;

    if ( pos == -1 ) return false ;


    boost::regex rx("[ \n\r\t\f\v]*") ;
    boost::smatch rm ;

    boost::regex_search(data, rm, rx) ;
    int wscount = rm[0].length() ;

    std::string name = data.substr(wscount, pos - wscount);
    std::string value = data.substr(++pos);

    session._COOKIE[name] = value ;

    return true ;
}


static bool parseCookies(HttpServerRequest &session)
{
    const char *ck = "Cookie" ;

    if ( session._SERVER.count(ck) == 0 ) return true ;

    std::string data = session._SERVER[ck] ;

    if ( data.empty() ) return false ;

    int pos, oldPos = 0 ;

    while (1)
    {
        // find the ';' terminating a name=value pair
        int pos = data.find(";", oldPos);

        // if no ';' was found, the rest of the string is a single cookie

        if ( pos == -1 ) {
            bool res = parseCookie(session, data.substr(oldPos, (int)-1));
            return res ;
        }

        // otherwise, the string contains multiple cookies
        // extract it and add the cookie to the list
        if ( !parseCookie(session, data.substr(oldPos, pos - oldPos)) ) return false ;

        // update pos (+1 to skip ';')
        oldPos = pos + 1;
    }

    return true ;
}


static std::string get_next_line(std::istream &strm, int maxc = 1000)
{
    std::string res ;
    char b0, b1 ;
    int count = 0 ;

    while ( count < maxc && !strm.eof() )
    {
        b0 = strm.get() ;
        count ++ ;

        if ( b0 == '\r' )
        {
            b1 = strm.get() ;
            count ++ ;

            if ( b1 == '\n' ) return res ;
            else {
                res += b0 ;
                res += b1 ;
            }
        }
        else res += b0 ;

    }

    return res ;
}


static bool parseMimeData(HttpServerRequest &session, istream &strm, const char *fld, const char *fileName,
                          const char *contentType,
                          const char *transEncoding,
                          const char *bnd)
{
    std::string data ;

    while ( 1 )
    {
        char b0 = strm.get() ;

        if ( b0 == '\r' )
        {
            char b1 = strm.get() ;

            if ( b1 == '\n' )
            {
                char b2 = strm.get() ;

                if ( b2 == '-' )
                {
                    char b3 = strm.get() ;

                    if ( b3 == '-' )
                    {
                        int bndlen = strlen(bnd) ;
                        char *buf = new char [bndlen] ;
                        strm.read(buf, bndlen) ;

                        if ( strncmp(buf, bnd, bndlen ) == 0 ) {
                            strm.get() ; strm.get() ;

                            delete buf ;
                            break ;
                        }
                        delete buf ;
                    }
                    else
                    {
                        data += b0 ;
                        data += b1 ;
                        data += b2 ;
                        data += b3 ;
                    }
                }
                else {
                    data += b0 ;
                    data += b1 ;
                    data += b2 ;
                }
            }
            else
            {
                data += b0 ;
                data += b1 ;
            }
        }
        else data += b0 ;
    }

    if ( fileName == 0 ) session._POST[fld] = data ;
    else
    {
        std::string uploadFolder ; //?
        HttpServerRequest::UploadedFile fileInfo ;

        fileInfo.mime = contentType ;
        fileInfo.origName = fileName ;

        boost::filesystem::path serverPath = getTemporaryPath(string(), "up", "tmp") ;

        FILE *file = fopen(serverPath.string().c_str(), "wb") ;
        fwrite(data.data(), data.size(), 1, file) ;
        fclose(file) ;

        fileInfo.serverPath = serverPath.string() ;
        session._FILE[fld] = fileInfo ;
    }

    return true ;
}


static bool parseMultipartData(HttpServerRequest &session, istream &strm, const char *bnd)
{
    std::string s = get_next_line(strm) ;
    if ( s.empty() ) return false ;

    // Parse boundary header
    const char *p = s.c_str() ;
    if ( *p++ != '-' || *p++ != '-' || strncmp(p, bnd, s.length() ) != 0 ) return  false ;

    while ( 1 )
    {

        std::string formField, fileName, contentType, transEncoding;

        while ( 1 )
        {
            s = get_next_line(strm) ;
            if ( s.empty() ) break ;

            const char *p = s.c_str() ;
            const char *q = strchr(p, ':') ;

            if ( q )
            {
                std::string key, val ;
                key.assign(p, (int)(q - p)) ;
                boost::trim(key) ;
                val.assign(q+1) ;
                boost::trim(val) ;

                if ( strncmp(key.c_str(), "Content-Disposition", 20) == 0 )
                {
                    if ( strncmp(val.c_str(), "form-data", 9) == 0 )
                    {
                        const char *a = strchr((const char *)val.c_str() + 9, ';') ;
                        while ( a && *a )
                        {
                            ++a ;
                            std::string key_, val_ ;

                            while ( *a && *a != '=' ) key_ += *a++ ;
                            if ( *a == 0 ) return false ; ++a ;
                            while ( *a && *a != ';' ) val_ += *a++ ;
                            boost::trim(key_);
                            boost::trim_if(val_, boost::is_any_of(" \"")) ;

                            if ( key_ == "name" ) formField = val_ ;
                            else if ( key_ == "filename" ) fileName = val_ ;


                        }
                    }
                }
                else if ( strncmp(key.c_str(), "Content-Type", 11) == 0 )
                    contentType = val ;
                else if ( strncmp(key.c_str(), "Content-Transfer-Encoding", 25) == 0 )
                    transEncoding = val ;

            }
        }

        if ( formField.empty() ) break;

        // Parse content

        if ( ! parseMimeData(session, strm, formField.c_str(),
                             ((fileName.empty()) ? (const char *)NULL : fileName.c_str()),
                             contentType.c_str(), transEncoding.c_str(), bnd) ) return false ;


    }

    return true ;
}



static bool parseFormData(HttpServerRequest &session, istream &strm)
{
    const char *cl = "Content-Length" ;

    unsigned int contentLength = 1000 ;
    bool hasContentLength = false ;

    if ( session._SERVER.count(cl) == 1 )
    {
        contentLength = atoi(session._SERVER[cl].c_str()) ;
        hasContentLength = true ;
    }

    const char *ct = "Content-Type" ;

    if ( session._SERVER.count(ct) == 0 ) return false ;

    std::string contentType = session._SERVER[ct] ;

    if ( strncmp(contentType.c_str(), "application/x-www-form-urlencoded", 33) == 0 )
    {
        // parse name value pairs

        std::string s = get_next_line(strm, contentLength) ;

        typedef boost::tokenizer<boost::char_separator<char> >   tokenizer;

        boost::char_separator<char> sep("&");
        tokenizer tokens(s, sep);

        for (tokenizer::iterator it = tokens.begin(); it != tokens.end(); ++it)
        {
            std::string str = (*it) ;

            int pos = str.find('=') ;
            if ( pos < 0 ) return false ;

            std::string key, val ;
            key = str.substr(0, pos) ;
            val = str.substr(pos+1) ;
            session._POST[url_decode(key.c_str())] = url_decode(val.c_str()) ;
        }
    }
    else if ( strncmp(contentType.c_str(),"multipart/form-data", 19) == 0 )
    {
        std::string boundary ;

        const char *p = strstr((const char *)contentType.c_str() + 19, "boundary") ;
        if ( !p ) return false ;
        ++p ;
        p = strchr(p, '=') ;
        if ( !p ) return false ;
        ++p ;
        while ( *p == ' ' || *p == '"' ) ++p ;
        while ( *p != 0 && *p != '"' && *p != '\r' && *p != ';' && *p != ' ')
            boundary += *p++ ;


        return parseMultipartData(session, strm, boundary.c_str()) ;
    }
    else if ( hasContentLength )
    {
        char *data = new char [contentLength+1] ;

        strm.read(data, contentLength) ;
        data[contentLength] = 0 ;

        session.content = data ;
        session.contentType = contentType ;

        delete data ;

    }


    return true ;
}

int HttpServerImpl::mg_begin_request(struct mg_connection *conn)
{
    struct mg_request_info *request_info = mg_get_request_info(conn);

    HttpServer *srv = (HttpServer *)request_info->user_data ;

    HttpServerResponseStream rstrm(conn) ;

    HttpServerRequest session ;

    if ( !parseVariables(request_info, session) ||
         !parseQueryString(request_info, session) ||
         !parseCookies(session) )
    {

        rstrm << "HTTP/1.1 400 Malformed HttpServerRequest\r\n" ;
        return 1 ;
    }

    if ( session._SERVER["REQUEST_METHOD"] == "POST" )
    {
        HttpServerRequestStream reqstr(conn) ;

        if ( !parseFormData(session, reqstr) ) {
            rstrm << "HTTP/1.1 400 Malformed HttpServerRequest\r\n" ;
            return 1 ;

        }
    }

    vector<HandlerInfo>::const_iterator it = srv->impl_->handlers_.begin() ;

    for( ; it != srv->impl_->handlers_.end() ; ++it )
    {
        const boost::regex &filter = it->req_uri_filter_ ;
        const string &method = it->req_method_ ;
        std::shared_ptr<HttpRequestHandler> handler = it->handler_ ;

        if ( session._SERVER["REQUEST_METHOD"] == method &&
             boost::regex_match(session._SERVER.get("REQUEST_URI"),filter) )
        {
            HttpServerResponse resp ;

            handler->respond(session, resp) ;

            resp.dumb(rstrm) ;

            return 1 ;
        }
    }

    return 0 ;
}

HttpServer::HttpServer(const char *ports)
{
    options_.add("listening_ports", ports) ;

    setNumThreads(10) ;

    impl_.reset(new HttpServerImpl()) ;
}

bool HttpServer::start() {
    if ( impl_ ) return impl_->start(this, options_) ;
    else return false ;
}

void HttpServer::stop() {
    if ( impl_ ) impl_->stop() ;
}

void HttpServer::addHandler(const std::shared_ptr<HttpRequestHandler> &h, const boost::regex &filter, const std::string &method )
{
    HandlerInfo info ;
    info.handler_ = h ;
    info.req_method_ = method ;
    info.req_uri_filter_ = filter ;
    impl_->handlers_.push_back(info) ;
/*
    if ( !options_.contains("protect_uri") )
            options_.add("protect_uri", filter) ;
        else
            options_["protect_uri"] += ',' + filter ;
*/
}

void HttpServer::setNumThreads(unsigned int num_threads)
{
    ostringstream strm ;
    strm << num_threads ;
    options_.add("num_threads", strm.str()) ;
}

void HttpServer::setSSLCertificate(const fs::path &pemFilePath)
{
    options_.add("ssl_certificate", pemFilePath.string()) ;
}

void HttpServer::setPasswordsFile(const fs::path &gpass)
{
    options_.add("global_auth_file", gpass.string()) ;
}

void HttpServer::setAuthRealm(const string &realm)
{
    options_.add("authentication_domain", realm) ;
}

HttpServer::~HttpServer() {}
