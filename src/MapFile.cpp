#include "MapFile.h"

#include <XmlDocument.h>

#include <spatialite.h>
#include <fstream>
#include <boost/filesystem.hpp>

using namespace std;

class SpatialLiteSingleton
{
    public:

    static SpatialLiteSingleton instance_;

    static SpatialLiteSingleton& instance() {
            return instance_;
    }

private:

    SpatialLiteSingleton () {
        spatialite_init(true);
    };

    ~SpatialLiteSingleton () {
        spatialite_cleanup();
    }

    SpatialLiteSingleton( SpatialLiteSingleton const & );

    void operator = ( SpatialLiteSingleton const & );


};

SpatialLiteSingleton SpatialLiteSingleton::instance_ ;


MapFile::MapFile():  db_(0) {}

MapFile::~MapFile() {
    delete db_ ;

}

bool MapFile::open(const string &filePath)
{
     if ( !boost::filesystem::exists(filePath) ) return false ;

     if ( !db_ ) {
         db_ = new SQLite::Database(filePath) ;
        fileName = filePath ;
     }

     return true ;
}


bool MapFile::create(const std::string &name) {

    if ( boost::filesystem::exists(name) )
        boost::filesystem::remove(name);

    db_ = new SQLite::Database(name) ;

    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {
        con.exec("SELECT InitSpatialMetadata(1);") ;
        con.exec("CREATE TABLE layers (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, geom TEXT NOT NULL, type TEXT NOT NULL);") ;
        con.exec("CREATE TABLE info (key TEXT NOT NULL, value TEXT NULL);") ;

        fileName = name ;

        return true ;
    }
    catch ( SQLite::Exception & )
    {

        return false ;
    }

}



string MapFile::getProj4SRID(int srid) const
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {

        string sql = "SELECT proj4text FROM spatial_ref_sys WHERE srid=? LIMIT 1;" ;

        SQLite::Query q(con, sql) ;

        q.bind(srid) ;

        SQLite::QueryResult res = q.exec() ;

        if ( res )
            return res.get<string>(0) ;

    }
    catch ( SQLite::Exception &e )
    {
        return string() ;
    }

}


bool MapFile::hasTable(const std::string &tableName)
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {
        string sql = "SELECT count(*) FROM sqlite_master WHERE sqlite_master.type='table' AND sqlite_master.name=?;" ;

        SQLite::Query q(con, sql) ;

        q.bind(tableName) ;

        SQLite::QueryResult res = q.exec() ;

        return ( res && res.get<int>(0) ) ;
    }
    catch ( SQLite::Exception & )
    {
        return false ;
    }

}

bool MapFile::hasLayer(const std::string &layerName)
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {
        string sql = "SELECT count(*) FROM sqlite_master WHERE sqlite_master.type='table' AND sqlite_master.name=?;" ;

        SQLite::Query q(con, sql) ;

        q.bind(layerName) ;

        SQLite::QueryResult res = q.exec() ;

        return ( res && res.get<int>(0) ) ;
    }
    catch ( SQLite::Exception & )
    {
        return false ;
    }

}

bool MapFile::deleteLayer(const std::string &layerName)
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {
        string geomName = getLayerGeometryColumn(layerName) ;

        con.exec("BEGIN TRANSACTION") ;
        con.exec("SELECT DiscardGeometryColumn('%q', '%q')", layerName.c_str(), geomName.c_str()) ;
        con.exec("DROP TABLE 'idx_%q_%q'", layerName.c_str(), geomName.c_str() ) ;
        con.exec("DROP TABLE %q", layerName.c_str()) ;
        con.exec("DELETE FROM layers WHERE name='%q'", layerName.c_str()) ;
        con.exec("COMMIT TRANSACTION") ;

        return true ;
    }
    catch ( SQLite::Exception &e )
    {
        return false ;
    }
}


string MapFile::getLayerGeometryColumn(const string &layerName) const
{
    vector<string> columns ;
    getLayerGeometryColumns(layerName, columns) ;

    if ( columns.empty() ) return string() ;
    else return columns[0] ;
}

void MapFile::getLayerGeometryColumns(const string &layerName, vector<string> &names) const
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {
        string sql = "SELECT f_geometry_column FROM geometry_columns WHERE f_table_name=?" ;

        SQLite::Query q(con, sql) ;
        q.bind(layerName) ;

        SQLite::QueryResult res = q.exec() ;

        while ( res ) {
            names.push_back(res.get<string>(0)) ;
            res.next() ;
        }
    }
    catch ( SQLite::Exception &e )
    {

    }

}

bool MapFile::createLayerTable(const std::string &layerName, const string &layerType, const string &geomColumn,
                               const vector<string> &tags, const vector<string> &tag_types,
                               bool overwrite, bool append)
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {
        bool has_layer = hasLayer(layerName) ;

        if ( has_layer && overwrite )
            deleteLayer(layerName) ;

        string sql ;

        vector<string> dict_columns ;

        if ( overwrite )
            sql = "CREATE TABLE ";
        else
            sql = "CREATE TABLE IF NOT EXISTS " ;

        sql +=  layerName + "(gid INTEGER PRIMARY KEY AUTOINCREMENT" ;
        for(int i=0 ; i<tags.size() ; i++)
        {
            sql += "," ;
            sql += tags[i] ;
            sql += " " ;

            if ( tag_types[i] == "text" )
                sql += "TEXT " ;
            else if ( tag_types[i] == "real" )
                sql += "REAL " ;
            else if ( tag_types[i] == "integer" )
                sql += "INTEGER " ;
            else if ( tag_types[i] == "dict" )
            {
                dict_columns.push_back(tags[i]) ;
                sql += "INTEGER " ;
            }
            else if ( tag_types[i].empty() )
                sql += " " ;

            sql += "NULL";
        }

        for(int i=0 ; i<tags.size() ; i++)
        {
            if ( tag_types[i] == "dict" )
            {
                sql += ", FOREIGN KEY(" ;
                sql += tags[i] ;
                sql += ") REFERENCES __dictionary__(id)" ;
            }
        }

        sql += ");" ;

        SQLite::Command(con, sql).exec() ;

        for(int i=0 ; i<dict_columns.size() ; i++ )
        {
            sql = "INSERT INTO dict_columns (table_name, column_name) VALUES ('" ;
            sql += layerName ;
            sql += "','" + dict_columns[i] + "');" ;

            SQLite::Command(con, sql).exec() ;

        }

        if ( overwrite || !has_layer )
        {
            // Add geometry column

            sql = "SELECT AddGeometryColumn( '"  ;
            sql += layerName ;
            sql += "', 'geom', 4326, " ;

            if ( layerType == "points" || layerType == "pois" )
                sql += "'POINT', 2);" ;
            else if ( layerType == "lines" )
                sql +="'LINESTRING', 2);" ;
            else if ( layerType == "polygons" )
                sql += "'POLYGON', 2);" ;

            SQLite::Command(con, sql).exec() ;

            // create spatial index

            con.exec("SELECT CreateSpatialIndex('%q', '%q');", layerName.c_str(), geomColumn.c_str()) ;
            con.exec("INSERT INTO layers (name, geom, type) VALUES ('%q', '%q', '%q')", layerName.c_str(), geomColumn.c_str(), layerType.c_str()) ;
        }

        if ( !append )
               con.exec("DELETE FROM %q", layerName.c_str()) ;

        return true ;
    }
    catch ( SQLite::Exception &e)
    {
        cerr << e.what() << endl ;
        return false ;
    }
}

bool MapFile::createPOITable(const std::string &layerName,
                               bool overwrite, bool append)
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    string poi_layer_name = layerName + "_text" ;

    try {
        bool has_layer = hasLayer(poi_layer_name) ;

        if ( has_layer && overwrite )
            con.exec("DROP TABLE %q", poi_layer_name.c_str()) ;

        string sql ;

        if ( overwrite )
            sql = "CREATE VIRTUAL TABLE "  ;
        else
            sql = "CREATE VIRTUAL TABLE IF NOT EXISTS " ;

        sql +=  poi_layer_name + " USING FTS4(content TEXT);" ;

        SQLite::Command(con, sql).exec() ;

        if ( !append )
               con.exec("DELETE FROM %q", layerName.c_str()) ;

        return true ;
    }
    catch ( SQLite::Exception &e)
    {
        cerr << e.what() << endl ;
        return false ;
    }
}

bool MapFile::createDictionary(bool overwrite)
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {
        string sql = "SELECT count(*) FROM sqlite_master WHERE sqlite_master.type='table' AND sqlite_master.name='__dictionary__';" ;

        SQLite::Query q(con, sql) ;

        SQLite::QueryResult res = q.exec() ;

        bool has_dictionary = ( res && res.get<int>(0) ) ;

        q.clear() ;

        if ( has_dictionary && overwrite )
        {
            con.exec("DROP TABLE __dictionary__") ;
            con.exec("DROP TABLE dict_columns") ;
        }

        if ( overwrite )
            sql = "CREATE TABLE __dictionary__ "  ;
        else
            sql = "CREATE TABLE IF NOT EXISTS __dictionary__" ;

        sql +=  "(id INTEGER PRIMARY KEY AUTOINCREMENT, key text NOT NULL UNIQUE);";

        SQLite::Command(con, sql).exec() ;

        if ( overwrite )
            sql = "CREATE TABLE dict_columns "  ;
        else
            sql = "CREATE TABLE IF NOT EXISTS dict_columns " ;

        sql +=  "(id INTEGER PRIMARY KEY AUTOINCREMENT, table_name text NOT NULL, column_name text NOT NULL);";

        SQLite::Command(con, sql).exec() ;

        return true ;
    }
    catch ( SQLite::Exception &e)
    {
        cerr << e.what() << endl ;
        return false ;
    }

}



