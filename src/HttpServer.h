#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include "Dictionary.h"

#include <iostream>
#include <map>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

class HttpServerRequest
{
public:

	// Dump an html table of all variables for debug purposes

    void dump(std::ostream &strm) const ;

	// Information about files uploaded 

	struct UploadedFile {
        std::string origName ;	// The original filename
        std::string serverPath ; // The path of a local temporary copy of the uploaded file
        std::string mime ;		// MIME information of the uploaded file
	} ;

    Dictionary _SERVER ; // Server variables
    Dictionary _GET ;	 // Query variables for GET requests
    Dictionary _POST ;   // Post variables for POST requests
    Dictionary _COOKIE ; // Cookies

    std::map<std::string, UploadedFile> _FILE ;	// Uploaded files

	// This is content sent using POST with Content-Type other than 
	// x-www-form-urlencoded or multipart-form-data e.g. text/xml
    std::string content ;
    std::string contentType ;
	
} ;

// The response to be filled by the user

class HttpServerResponse: public std::ostringstream
{
public:

    HttpServerResponse();

    // set a header key/value pair
    void setHeader(const std::string &key, const std::string &value);

    // set a cookie
    void setCookie(const std::string &key, const std::string &value);

    // set returned HTTP code
    void setCode(int code);

    // Sends the contents of a file to the client. The request headers are parsed to determine if compression is acceptable
    void sendFileContents(const HttpServerRequest &req, const boost::filesystem::path &p, const std::string &mime = "" ) ;

    // outputs the full message to the client socket (called by the server)
    void dumb(std::ostream &strm) ;

protected:

    int code_;
    Dictionary  headers_;
} ;

// Override your custom controller to respond to the request

class HttpRequestHandler {

public:

    // to be ovveriden by actual server implementations
    virtual void respond(const HttpServerRequest &request, HttpServerResponse &resp) = 0 ;
};

class HttpServer
{
	public:

    // create a server listening on the designated list of ports (a comma separated list of ports).
    // For SSL ports you should append an 's' after the port number.

    HttpServer(const char *ports) ;
    ~HttpServer() ;

    void setNumThreads(unsigned int num_threads) ;

    // essential for SSL encryption
    void setSSLCertificate(const boost::filesystem::path &pemFilePath) ;

    // if authentication is needed
    // use htdigest -c <password file> <realm> <user> to add a new user

    void setPasswordsFile(const boost::filesystem::path &gpass);

    // if not set it defaults to mydomain.com

    void setAuthRealm(const std::string &realm);

    // add a handler for the request path that may be a regular expression
    void addHandler(const std::shared_ptr<HttpRequestHandler> &h, const boost::regex &req_filter, const std::string &req_method = "GET" ) ;

	// start the server
	bool start() ;

	// stop the server
	void stop() ;


private:
	
    friend class HttpServerImpl ;
    std::unique_ptr<class HttpServerImpl> impl_ ;

    Dictionary options_ ;
} ;


#endif
