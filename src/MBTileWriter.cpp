#include "MBTileWriter.h"
#include "base64.h"

#include <boost/filesystem.hpp>

using namespace std ;

MBTileWriter::MBTileWriter(const std::string &fileName)
{
    if ( boost::filesystem::exists(fileName) )
        boost::filesystem::remove(fileName);

    db_.reset(new SQLite::Database(fileName)) ;

    SQLite::Session session(db_.get()) ;
    SQLite::Connection &con = session.handle() ;

    con.exec("CREATE TABLE metadata (name text, value text, UNIQUE(name));") ;
    con.exec("CREATE TABLE tiles (zoom_level integer, tile_column integer, tile_row integer, tile_data blob, UNIQUE (zoom_level, tile_column, tile_row));") ;
}


bool MBTileWriter::writeMetaData(const std::string &name, const std::string &val)
{
    assert(db_) ;

    SQLite::Session session(db_.get()) ;
    SQLite::Connection &con = session.handle() ;

    try {
        SQLite::Command cmd(con, "REPLACE INTO metadata VALUES (?, ?)") ;
        cmd.bind(name) ;
        cmd.bind(val) ;
        return true ;
    }
    catch ( SQLite::Exception &e ) {
        cerr << e.what() << endl ;
        return false ;
    }
}


bool MBTileWriter::writeTiles(const MapFile &map, MapConfig &cfg)
{
    assert(db_) ;

    SQLite::Session session(db_.get()) ;
    SQLite::Connection &con = session.handle() ;

    // write metadata

    try {
        SQLite::Transaction trans(con) ;
        SQLite::Command cmd(con, "REPLACE INTO tiles (zoom_level, tile_column, tile_row, tile_data) VALUES (?,?,?,?);") ;

        for( uint32_t z = cfg.minz_ ; z <= cfg.maxz_ ; z++ )
        {
            uint32_t x0, y0, x1, y1 ;
            tms::metersToTile(cfg.bbox_.minx_, cfg.bbox_.miny_, z, x0, y0) ;
            tms::metersToTile(cfg.bbox_.maxx_, cfg.bbox_.maxy_, z, x1, y1) ;

            for( uint32_t x = x0 ; x<=x1 ; x++ )
                for(uint32_t y = y0 ; y<=y1 ; y++ )
                {
                    if ( z == 12 && x == 2302 && y == pow(2, z)-1-1535 )
                        cout << "ok here" << endl ;
                    VectorTileWriter vt(x, y, z) ;

                    if ( map.queryTile(cfg, vt) ) {

                        string data = vt.toString() ;

                        cmd.bind((int)z) ;
                        cmd.bind((int)x) ;
                        cmd.bind((int)y) ;
                        cmd.bind(data.data(), data.size()) ;

                        cmd.exec() ;
                        cmd.clear() ;
                    }
 /*                   else {
                        string data = vt.toString(true) ;
                        string b64 = base64_encode((const unsigned char *)data.data(), data.size()) ;
                        cout << b64 << endl ;

                    }
                    */
                }
        }

        trans.commit() ;

        return true ;
    }
    catch ( SQLite::Exception &e )
    {
        cerr << e.what() << endl ;
        return false ;
    }
}
