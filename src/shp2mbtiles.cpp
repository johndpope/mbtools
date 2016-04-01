#include <fstream>

#include "map_config.hpp"
#include "map_file.hpp"

#include "MBTileWriter.h"

#include <boost/filesystem.hpp>

using namespace std ;
namespace fs = boost::filesystem ;

void printUsageAndExit()
{
    cerr << "Usage: shp2mbtiles --options <options_file> --out <tileset> [--srid <srid>] [--enc <char encoding>] [--layer <name>] <shape_file>" << endl ;
    exit(1) ;
}

int main(int argc, char *argv[])
{
    string map_file, map_config_file, tile_set ;
    int srid = 3857 ;
    string shp_file, layer_name, encoding = "WINDOWS-1252";

    for( int i=1 ; i<argc ; i++ )
    {
        string arg = argv[i] ;

        if ( arg == "--options" ) {
            if ( i++ == argc ) printUsageAndExit() ;
            map_config_file = argv[i] ;
        }
        else if ( arg == "--out" ) {
            if ( i++ == argc ) printUsageAndExit() ;
            tile_set = argv[i] ;
        }
        else if ( arg == "--srid" ) {
            if ( i++ == argc ) printUsageAndExit() ;
            srid = atoi(argv[i]) ;
        }
        else if ( arg == "--enc" ) {
            if ( i++ == argc ) printUsageAndExit() ;
            encoding = argv[i] ;
        }
        else if ( arg == "--layer" ) {
            if ( i++ == argc ) printUsageAndExit() ;
            layer_name = argv[i] ;
        }

        else
            shp_file = arg ;
    }

    if ( map_config_file.empty() || shp_file.empty() )
        printUsageAndExit() ;

    if ( layer_name.empty() )
        layer_name = fs::path(shp_file).stem().string() ;

    boost::filesystem::path tmp_dir = boost::filesystem::temp_directory_path() ;
    boost::filesystem::path tmp_file = boost::filesystem::unique_path("%%%%%.sqlite");

    map_file = ( tmp_dir / tmp_file ).native() ;

    cout << map_file << endl ;

    MapFile gfile ;

    if ( !gfile.create(map_file) ) {
        cerr << "can't open map file: " << map_file << endl ;
        exit(1) ;
    }

    MapConfig mcfg ;
    if ( !mcfg.parse(map_config_file) ) {
        cerr << "Error parsing map configuration file: " << map_config_file << endl ;
        return 0 ;
    }

    if ( !gfile.processShpFile(shp_file, layer_name, srid, encoding) ) {
        cerr << "Error while creating temporary spatialite database" << endl ;
        return 0 ;
    }

    MBTileWriter twriter(tile_set) ;

    if ( boost::filesystem::is_directory(tile_set) )
        twriter.writeTilesFolder(gfile, mcfg) ;
    else
        twriter.writeTilesDB(gfile, mcfg) ;


//    boost::filesystem::remove(mapFile) ;

    return 1 ;

}
