#include "MapServer.h"
#include "rapidjson/rapidjson.h"

#include <thread>

using namespace std ;

int main(int argc, char *argv[]) {
    std::shared_ptr<MapServer> srv(new MapServer("/home/malasiot/source/mbtools/build/data/", "5000")) ;

    std::thread t(&MapServer::run, srv.get()) ;


    this_thread::sleep_for(std::chrono::seconds(10)) ;

    srv->stop() ;

    t.join() ;


}
