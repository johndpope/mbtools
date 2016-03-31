#ifndef HTTP_SERVER_REQUEST_HANDLER_FACTORY
#define HTTP_SERVER_REQUEST_HANDLER_FACTORY

#include "request_handler.hpp"

// modeled after POCO Net library

namespace http {
namespace server {

class request_handler_factory {
public:
    request_handler_factory() = default ;

    virtual request_handler * create(const request &req) = 0;
};

}
}













#endif
