#include "jp2_decoder.hpp"
#include "logger.hpp"

#include <memory>

using namespace std ;

class DefaultLogger: public Logger
{
public:
    DefaultLogger() {
        addAppender(make_shared<LogStreamAppender>(Trace, make_shared<LogPatternFormatter>("%In function %c, %F:%l: %m"), std::cerr)) ;
    }
};


Logger &get_current_logger() {
    static DefaultLogger g_server_logger_ ;
    return g_server_logger_ ;
}

int main(int argc, char *argv[]) {

    JP2Decoder decoder ;

    decoder.open("/home/malasiot/GPS/mapping/hillshade.jp2") ;

    char *data = new char [256*256] ;
    decoder.read(20, data) ;

    return 0;


}
