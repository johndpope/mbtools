#include <fstream>

#include "MapConfig.h"
#include "MapFile.h"

#include <boost/filesystem.hpp>

using namespace std ;

void printUsageAndExit()
{
    cerr << "Usage: osm2mbtiles --config <config_file> <file_name>+" << endl ;
    exit(1) ;
}

int main(int argc, char *argv[])
{
    string mapFile, configFile ;
    vector<string> osmFiles ;

    for( int i=1 ; i<argc ; i++ )
    {
        string arg = argv[i] ;

        if ( arg == "--config" ) {
            if ( i++ == argc ) printUsageAndExit() ;
            configFile = argv[i] ;
        }
        else
            osmFiles.push_back(argv[i]) ;
    }

    if ( configFile.empty() ||  osmFiles.empty() )
        printUsageAndExit() ;

    boost::filesystem::path tmp_dir = boost::filesystem::temp_directory_path() ;
    boost::filesystem::path tmp_file = boost::filesystem::unique_path("%%%%%.sqlite");

    mapFile = ( tmp_dir / tmp_file ).native() ;


    MapFile gfile ;

    if ( !gfile.create(mapFile) ) {
        cerr << "can't open map file: " << mapFile << endl ;
        exit(1) ;
    }

    MapConfig cfg ;
    if ( !cfg.parse(configFile) ) {
        cerr << "Error parsing configuration file: " << configFile << endl ;
        return 0 ;
    }

    for( const Layer &layer: cfg.layers_)
    {
        if ( ! gfile.createLayerTable(layer.name_, layer.geom_, layer.srid_ ) ) {
            cerr << "Failed to create layer " << layer.name_ << ", skipping" ;
           continue ;
        }
    }

    if ( !gfile.processOsmFiles(osmFiles, cfg) ) {
        cerr << "Error while creating temporary spatialite database" << endl ;
        return 0 ;
    }

    cout << mapFile << endl ;

//    boost::filesystem::remove(mapFile) ;

    return 1 ;

}
