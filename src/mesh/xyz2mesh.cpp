#include <string>
#include <vector>
#include <iostream>

#include "Mesh.h"

using namespace std ;

void printUsageAndExit()
{
    cerr << "Usage: xyz2mesh [--nodata <value>] [--maxarea <value>] [--vtx <vertices>] [--error <error>] <out> <files>+" << endl ;
    exit(1) ;
}

int main(int argc, char *argv[])
{
    vector<string> input_files ;
    string out_file ;
    int no_data = -32768, target_vertices = 0 ;
    float max_area = 0, target_error = 1.0e-3 ;

    for( int i=1 ; i<argc ; i++ )
    {
        string arg = argv[i] ;

        if ( arg == "--nodata" ) {
            if ( ++i < argc )
                no_data = stoi(argv[i]) ;
        }
        else if ( arg == "--maxarea" ) {
            if ( ++i < argc )
                max_area = stof(argv[i]) ;
        }
        else if ( arg == "--vertices" ) {
            if ( ++i < argc )
                target_vertices = stoi(argv[i]) ;
        }
        else if ( arg == "--error" ) {
            if ( ++i < argc )
                target_error = stof(argv[i]) ;
        }
        else if ( !out_file.empty() )
            input_files.push_back(arg) ;
        else
            out_file = arg ;
    }

    if ( input_files.empty() || out_file.empty() )
        printUsageAndExit() ;


    tin::Mesh m ;

    for(auto file: input_files)
       m.loadXYZ(file, no_data) ;

    m.triangulateCoords(max_area) ;
    m.decimate(target_vertices, target_error) ;
    m.save(out_file) ;
}
