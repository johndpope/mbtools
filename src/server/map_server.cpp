#include "map_server.hpp"
#include "map_server_handler_factory.hpp"

using namespace std ;

MapServer::MapServer(const string &root_folders, const string &ports, bool withgl):
    http::Server(std::make_shared<MapServerHandlerFactory>(root_folders, withgl), "127.0.0.1", ports, 4) {
}
