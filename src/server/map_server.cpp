#include "map_server.hpp"
#include "map_server_handler_factory.hpp"

using namespace std ;

MapServer::MapServer(const string &rootFolder, const string &ports):
    http::Server(std::make_shared<MapServerHandlerFactory>(rootFolder), "127.0.0.1", ports, 4) {
}
