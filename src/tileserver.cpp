#include "MapServer.h"
#include "rapidjson/rapidjson.h"

int main(int argc, char *argv[]) {
    MapServer srv("/home/malasiot/source/mbtools/build/data/", "5000") ;
    srv.start() ;

    getchar() ;

}
