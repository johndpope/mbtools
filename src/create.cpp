#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <iomanip>

#include <MapFile.h>
#include <ParseUtil.h>

using namespace std ;

MapFile gfile ;
string gpath ;

void printUsageAndExit()
{
    cerr << "Usage: map_create [--ignore] ( commands )+ file_name" << endl ;
    cerr << "Create new map:\t --create --bbox minx miny maxx maxy" << endl ;
    cerr << "Add/Update OSM layers:\t --osm name -type (points|lines|polygons) (--file osm_file)+ [--filter expression] [--tags tags] [--overwrite] [--geom column]" << endl ;
    cerr << "Create metadata for external layer (e.g. created with gdal):\t --import name -type (points|lines|polygons) [--geom <column_name>]" << endl ;
    cerr << "Create POI index from OSM data:\t --index --file <osm_file> (--category poi_category_tag --filter expression --tags tags)+ [--overwrite] [--append]" << endl ;

    exit(1) ;
}

// tags are of the form: tag1=integer,tag2, tag3=real, tag4, tag5 = text
bool parseTags(const string &tags, vector<string> &names, vector<string> &types)
{
    boost::sregex_token_iterator it( tags.begin(), tags.end(), boost::regex("[\\s]*,[\\s]*"), -1 ), end;

    while ( it != end )
    {
        string tag_ = *it ;
        vector<string> tokens ;
        boost::split( tokens, tag_ , boost::is_any_of("\t= "), boost::token_compress_on);

        names.push_back(tokens[0]) ;

        if ( tokens.size() == 2 )
            types.push_back(tokens[1])  ;
        else
            types.push_back("text") ;

        ++it ;

    }

    return true ;
}

bool parseCreateArgs(int argc, char *argv[], int &i)
{
    double minX, minY, maxX, maxY ;
    bool has_bbox = false ;
    string fileName ;

    ++i ;

    for( ; i<argc-1 ; i++ )
    {
        string arg = argv[i] ;

        if ( arg == "--bbox" )
        {
            if ( i++ == argc || !parseNumber(argv[i], minX) ) printUsageAndExit() ;
            if ( i++ == argc || !parseNumber(argv[i], minY) ) printUsageAndExit() ;
            if ( i++ == argc || !parseNumber(argv[i], maxX) ) printUsageAndExit() ;
            if ( i++ == argc || !parseNumber(argv[i], maxY) ) printUsageAndExit() ;
            has_bbox = true ;
        }
        else break ;
    }

    if ( !gfile.create(gpath) )
    {
        cerr << "Unable to create map file at: " << fileName << endl ;
        return false ;

    }
    else if ( has_bbox ) {
        if ( !gfile.setBoundingBox(minX, minY, maxX, maxY) )
        {
            cerr << "Unable to set bounding box" << endl ;
            return false ;
        }
    }

    return true ;
}

bool parseCreateLayerOsmArgs(int argc, char *argv[], int &i)
{
    string layerName, tags, filter, geom_column, style, layerType ;
    vector<string> src_files ;
    bool overwrite = true, append = false ;

    for(  ; i<argc ; i++ )
    {
        string arg = argv[i] ;

        if ( arg == "--osm" && layerName.empty() )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            layerName = argv[i] ;
        }
        else if ( arg == "--type" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            layerType = argv[i] ;
        }
        else if ( arg == "--style" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            style = argv[i] ;
        }
        else if ( arg == "--filter" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            filter = argv[i] ;
        }
        else if ( arg == "--tags" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            tags = argv[i] ;
        }
        else if ( arg == "--geom" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            geom_column = argv[i] ;
        }
        else if ( arg == "--file" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            src_files.push_back(argv[i]) ;
        }
        else if ( arg == "--overwrite" ) // if table exists and overwrite is true it will drop the table and make a new one otherwise it will try to write in the existing table
            overwrite = true ;
        else if ( arg == "--append" )   // if false it will delete contents of the table before inserting new data
            append = true ;
        else break ;
    }

    if ( layerType.empty() ||  layerName.empty() ||  src_files.empty() )
        printUsageAndExit() ;

    vector<string> tag_names, tag_types ;
    if ( !tags.empty() && !parseTags(tags, tag_names, tag_types) )
        printUsageAndExit() ;

    if ( !gfile.open(gpath) ) {
        cerr << "Can't open map file: " << gpath << endl ;
        return false ;
    }

    if ( !gfile.createOSMLayer(layerName, layerType, src_files, filter, geom_column, tag_names, tag_types, overwrite, append) )
    {
        cerr << "Unable to create OSM layer: " << layerName << endl ;
        return false ;
    }

    return true ;
}

bool parseIndexArgs(int argc, char *argv[], int &i)
{
    vector<string> categories, tags, filters ;
    vector<string> src_files ;

    bool overwrite = true, append = false ;

    for(  ; i<argc ; i++ )
    {
        string arg = argv[i] ;

        if ( arg == "--index") ;
        else if ( arg == "--filter" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            filters.push_back(argv[i]) ;
        }
        else if ( arg == "--category" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            categories.push_back(argv[i]) ;
        }
        else if ( arg == "--tags" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            tags.push_back(argv[i]) ;
        }
        else if ( arg == "--file" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            src_files.push_back(argv[i]) ;
        }
        else break ;
    }

    if ( categories.empty() || tags.empty() ||  filters.empty() || src_files.empty() )
        printUsageAndExit() ;

    if ( categories.size() != tags.size() || tags.size() != filters.size()  )
        printUsageAndExit() ;

    if ( !gfile.open(gpath) ) {
        cerr << "Can't open map file: " << gpath << endl ;
        return false ;
    }

    if ( !gfile.createIndex(src_files, categories, filters, tags) )
    {
        cerr << "Unable to create POI index layer"  << endl ;
        return false ;
    }

    return true ;
}

bool parseBBoxArgs(int argc, char *argv[], int &i)
{
    string srid, format ;

    for(  ; i<argc ; i++ )
    {
        string arg = argv[i] ;

        if ( arg == "--bbox" ) ;
        else if ( arg == "--srid" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            srid = argv[i] ;
        }
        else if ( arg == "--format" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            format = argv[i] ;
        }
        else break ;
    }

    if ( !gfile.open(gpath) ) {
        cerr << "Can't open map file: " << gpath << endl ;
        return false ;
    }

    BBox box ;
    gfile.getBoundingBox(box, srid.empty() ? 4326:atoi(srid.c_str()));

    cout << fixed << setw( 10 ) << setprecision( 2 ) << box.minx << ' ' << box.miny << ' ' << box.maxx << ' ' << box.maxy << endl ;

    return true ;


}

bool parseImportLayerArgs(int argc, char *argv[], int &i)
{
    string layerName,  geom_column, layerType ;

    for(  ; i<argc ; i++ )
    {
        string arg = argv[i] ;

        if ( arg == "--import" && layerName.empty() )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            layerName = argv[i] ;
        }
        else if ( arg == "--type" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            layerType = argv[i] ;
        }
        else if ( arg == "--geom" )
        {
            if ( i++ == argc ) printUsageAndExit() ;
            geom_column = argv[i] ;
        }
        else break ;
    }

    if ( layerType.empty() ||  layerName.empty() )
        printUsageAndExit() ;

    if ( !gfile.open(gpath) ) {
        cerr << "Can't open map file: " << gpath << endl ;
        return false ;
    }

    if ( !gfile.updateLayerMetadata(layerName, layerType, geom_column) )
    {
        cerr << "Unable to update layer metadata: " << layerName << endl ;
        return false ;
    }

    return true ;
}

int main(int argc, char *argv[])
{
    bool continue_ = false ; // continue if a command returns and error

    // the last argument should be the map database path

    if ( argc < 3  )
    {
        printUsageAndExit() ;
        return 1 ;
    }

    gpath = argv[argc-1] ;

    int i ;
    for( i=1 ; i<argc-1 ; )
    {
        string arg = argv[i] ;

        if ( arg == "--continue" ) {
            continue_ = true ;
            ++i ;
        }
        else if ( arg == "--create" && !parseCreateArgs(argc, argv, i) && !continue_ ) return 1 ;
        else if ( arg == "--osm" && !parseCreateLayerOsmArgs(argc, argv, i) && !continue_ ) return 1 ;
        else if ( arg == "--import" && !parseImportLayerArgs(argc, argv, i) && !continue_ ) return 1 ;
        else if ( arg == "--bbox" && !parseBBoxArgs(argc, argv, i) && !continue_ ) return 1 ;
        else if ( arg == "--index" && !parseIndexArgs(argc, argv, i) && !continue_ ) return 1 ;

    }


    return 1 ;

}
