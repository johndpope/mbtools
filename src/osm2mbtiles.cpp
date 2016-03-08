#include <fstream>

#include "MapConfig.h"
#include "MapFile.h"
#include "MBTileWriter.h"

#include <boost/filesystem.hpp>

using namespace std ;

void printUsageAndExit()
{
    cerr << "Usage: osm2mbtiles --import <config_file> --options <options_file> --out <tileset> <file_name>+" << endl ;
    exit(1) ;
}

int main(int argc, char *argv[])
{
    string mapFile, mapConfigFile, importConfigFile, tileSet ;
    vector<string> osmFiles ;

    for( int i=1 ; i<argc ; i++ )
    {
        string arg = argv[i] ;

        if ( arg == "--import" ) {
            if ( i++ == argc ) printUsageAndExit() ;
            importConfigFile = argv[i] ;
        }
        else if ( arg == "--options" ) {
            if ( i++ == argc ) printUsageAndExit() ;
            mapConfigFile = argv[i] ;
        }
        else if ( arg == "--out" ) {
            if ( i++ == argc ) printUsageAndExit() ;
            tileSet = argv[i] ;
        }

        else
            osmFiles.push_back(argv[i]) ;
    }

    if ( importConfigFile.empty() ||  mapConfigFile.empty() || osmFiles.empty() )
        printUsageAndExit() ;

    boost::filesystem::path tmp_dir = boost::filesystem::temp_directory_path() ;
    boost::filesystem::path tmp_file = boost::filesystem::unique_path("%%%%%.sqlite");

    mapFile = ( tmp_dir / tmp_file ).native() ;

    MapFile gfile ;

    if ( !gfile.create(mapFile) ) {
        cerr << "can't open map file: " << mapFile << endl ;
        exit(1) ;
    }

    ImportConfig icfg ;
    if ( !icfg.parse(importConfigFile) ) {
        cerr << "Error parsing OSM import configuration file: " << importConfigFile << endl ;
        return 0 ;
    }

    MapConfig mcfg ;
    if ( !mcfg.parse(mapConfigFile) ) {
        cerr << "Error parsing map configuration file: " << mapConfigFile << endl ;
        return 0 ;
    }

    for( const ImportLayer &layer: icfg.layers_)  {
        if ( ! gfile.createLayerTable(layer.name_, layer.geom_, layer.srid_ ) ) {
            cerr << "Failed to create layer " << layer.name_ << ", skipping" ;
           continue ;
        }
    }

    if ( !gfile.processOsmFiles(osmFiles, icfg) ) {
        cerr << "Error while creating temporary spatialite database" << endl ;
        return 0 ;
    }

    MBTileWriter twriter(tileSet) ;

    twriter.writeMetaData("name", mcfg.name_) ;
    twriter.writeMetaData("version", "1.1") ;
    twriter.writeMetaData("type", "baselayer") ;
    twriter.writeMetaData("version", "1.1") ;
    twriter.writeMetaData("description", mcfg.description_) ;
    twriter.writeMetaData("attribution", mcfg.attribution_) ;

    twriter.writeTiles(gfile, mcfg) ;


//    boost::filesystem::remove(mapFile) ;

    return 1 ;

}
