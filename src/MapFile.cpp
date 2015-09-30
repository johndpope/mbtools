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


MapFile::MapFile():  db_(0), view_suffix_("_view"), dictionary_table_name_("__dictionary"), geometry_column_name_("geometry") {}

MapFile::~MapFile() {
    delete db_ ;
}

bool MapFile::open(const string &filePath)
{
     if ( !boost::filesystem::exists(filePath) ) return false ;

     if ( !db_ ) {
         db_ = new SQLite::Database(filePath) ;
        fileName_ = filePath ;
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
        con.exec("PRAGMA synchronous=NORMAL") ;
        con.exec("PRAGMA journal_mode=WAL") ;
        con.exec("SELECT InitSpatialMetadata(1);") ;

        con.exec("CREATE TABLE layers (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, geom TEXT NOT NULL, type TEXT NOT NULL);") ;
        con.exec("CREATE TABLE info (key TEXT NOT NULL, value TEXT NULL);") ;

        fileName_ = name ;

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
        con.exec("DROP VIEW IF EXISTS %q", (layerName + view_suffix_).c_str()) ;
        con.exec("DELETE FROM views_geometry_columns WHERE view_name='%q'",(layerName + view_suffix_).c_str()) ;
        con.exec("DELETE FROM layers WHERE name='%q'", layerName.c_str()) ;
        con.exec("COMMIT TRANSACTION") ;

        return true ;
    }
    catch ( SQLite::Exception &e )
    {
        cerr << "Error delete layer:" << e.what() << endl ;
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

bool MapFile::createLayerTable(const std::string &layerName, const string &layerType, const string &layerSrid, const string &geomColumn,
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
                sql += ") REFERENCES " + dictionary_table_name_ + "(id)" ;
            }
        }

        sql += ");" ;

        SQLite::Command(con, sql).exec() ;

        for(int i=0 ; i<dict_columns.size() ; i++ )
        {
            sql = "INSERT INTO " ;
            sql += dictionary_table_name_ + "_metadata (table_name, column_name) VALUES ('" ;
            sql += layerName ;
            sql += "','" + dict_columns[i] + "');" ;

            SQLite::Command(con, sql).exec() ;
        }

        if ( overwrite || !has_layer )
        {
            // Add geometry column

            sql = "SELECT AddGeometryColumn( '"  ;
            sql += layerName ;
            sql += "', '" + geomColumn +"', " + layerSrid + "," ;

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

        // create view

        if ( !dict_columns.empty() )
        {
            stringstream sqls ;

            sqls << "CREATE VIEW " << layerName << view_suffix_ << " AS SELECT gid," ;

            uint counter = 1 ;

            for( int i=0 ; i<tags.size() ; i++ )
            {
                if ( tag_types[i] == "dict" )
                    sqls << dictionary_table_name_  << counter++ << ".key AS " << tags[i] << "," ;
                else
                    sqls << tags[i] << "," ;
            }

            sqls << geomColumn << " FROM " << layerName ;

            counter = 1 ;

            for( int i=0 ; i<tags.size() ; i++ )
            {
                if ( tag_types[i] == "dict" )
                {
                    sqls << " LEFT JOIN " << dictionary_table_name_ << " AS " << dictionary_table_name_ << counter << " ON " << dictionary_table_name_ << counter << ".id = " << tags[i] ;
                    counter ++ ;
                }
            }

            SQLite::Command(con, sqls.str()).exec() ;

            sqls.str(std::string());

            sqls << "INSERT INTO views_geometry_columns (view_name, view_geometry, view_rowid, f_table_name, f_geometry_column, read_only) VALUES('"
                 << layerName << view_suffix_ <<"', '" << geometry_column_name_ <<"', 'gid','"  << layerName << "','" << geometry_column_name_ << "',1);" ;

            SQLite::Command(con, sqls.str()).exec() ;
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

bool MapFile::createResourcesTable(bool append)
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    try {
        con.exec("CREATE TABLE IF NOT EXISTS __resources (name TEXT PRIMARY KEY, sz INT, data BLOB);") ;
        if ( !append ) con.exec("DELETE FROM __resources;") ;
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
        string sql = "SELECT count(*) FROM sqlite_master WHERE sqlite_master.type='table' AND sqlite_master.name='" ;
        sql += dictionary_table_name_ + "';" ;

        SQLite::Query q(con, sql) ;

        SQLite::QueryResult res = q.exec() ;

        bool has_dictionary = ( res && res.get<int>(0) ) ;

        q.clear() ;

        if ( has_dictionary && overwrite )
        {
            con.exec("DROP TABLE %q", dictionary_table_name_.c_str()) ;
            con.exec("DROP TABLE %q", (dictionary_table_name_ + "_metadata").c_str()) ;
        }

        if ( overwrite )
            sql = "CREATE TABLE " + dictionary_table_name_;
        else
            sql = "CREATE TABLE IF NOT EXISTS " + dictionary_table_name_ ;

        sql +=  "(id INTEGER PRIMARY KEY AUTOINCREMENT, key text NOT NULL UNIQUE);";

        SQLite::Command(con, sql).exec() ;

        if ( overwrite )
            sql = "CREATE TABLE " + dictionary_table_name_ + "_metadata"  ;
        else
            sql = "CREATE TABLE IF NOT EXISTS " + dictionary_table_name_ + "_metadata";

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

string MapFile::insertFeatureSQL(const vector<string> &tags, const vector<string> &tag_types, const string &layerName, const string &geomCmd )
{
    string sql ;

    // collect tags

    sql = "INSERT INTO " ;
    sql += layerName ;
    sql += "(" + geometry_column_name_ ;

    for(int j=0 ; j<tags.size() ; j++ )
    {
        sql += ',' ;
        sql += tags[j] ;
    }

    sql += ") VALUES (" + geomCmd ;
    for(int j=0 ; j<tags.size() ; j++ ) {

        if ( tag_types[j] == "dict" )
            sql += ",(SELECT id FROM " + dictionary_table_name_ + " WHERE key=?)" ;
        else
            sql += ",?" ;

    }
    sql += ");" ;

    return sql ;

}


