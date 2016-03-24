#include <fstream>

#include "MeshTilesetWriter.h"

#include <boost/filesystem.hpp>

using namespace std ;
namespace fs = boost::filesystem ;

void printUsageAndExit()
{
    cerr << "Usage: mesh2mbtiles --config <config_file> <tileset> <mesh_file>" << endl ;
    exit(1) ;
}

int main(int argc, char *argv[])
{
    string tile_set, mesh_file, map_config_file ;

    for( int i=1 ; i<argc ; i++ )
    {
        string arg = argv[i] ;

        if ( arg == "--config" ) {
            if ( i++ == argc ) printUsageAndExit() ;
            map_config_file = argv[i] ;
        }
        else if ( tile_set.empty() )
            tile_set = arg ;
        else if ( mesh_file.empty() )
            mesh_file = arg ;
    }

    if ( tile_set.empty() || mesh_file.empty() || map_config_file.empty() )
        printUsageAndExit() ;

    MapConfig cfg ;

    if ( !cfg.parse(map_config_file) ) {
        cerr << "Error parsing map configuration file: " << map_config_file << endl ;
        return 0 ;
    }

    MeshTilesetWriter twriter(tile_set) ;

    if ( boost::filesystem::is_directory(tile_set) )
        twriter.writeTilesFolder(mesh_file, cfg) ;
    else
        twriter.writeTilesDB(mesh_file, cfg) ;

    return 1 ;

}
