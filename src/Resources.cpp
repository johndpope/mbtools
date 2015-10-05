#include "MapFile.h"

#include <boost/filesystem.hpp>
#include <fstream>

#include <zlib.h>

using namespace std ;
namespace fs = boost::filesystem ;

static string read_file( const string &fileName, uint &sizeOrig, bool compressFile = true)
{
    ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate );

    sizeOrig = ifs.tellg();
    ifs.seekg(0, ios::beg);

    string bytes ;
    bytes.reserve(sizeOrig) ;
    ifs.read(&bytes[0], sizeOrig);

    if ( compressFile )
    {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        if ( deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK )
            return string() ;

        zs.next_in = (Bytef*)bytes.data() ;
        zs.avail_in = sizeOrig ;

        int ret;
        char outbuffer[32768];
        std::string outstring;

        // retrieve the compressed bytes blockwise
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = deflate(&zs, Z_FINISH);

            if (outstring.size() < zs.total_out) {
                // append the block to the output string
                outstring.append(outbuffer,
                                 zs.total_out - outstring.size());
            }
        } while (ret == Z_OK);

        deflateEnd(&zs);

        return outstring ;
    }
    else return bytes ;
}

void get_file_list(const string &key, const fs::path &filePath, Dictionary &files)
{

    string prefix ;

    if ( !key.empty() ) prefix = key + '/' ;

    if ( fs::is_regular_file(filePath) )
    {
        files.add(prefix + filePath.filename().string(), filePath.string()) ;
    }
    else {
        fs::directory_iterator end_itr;

        for (fs::directory_iterator itr(filePath); itr != end_itr; ++itr)
        {
            if (fs::is_regular_file(itr->path())) {
                string current_file = itr->path().string();
                files.add(prefix + itr->path().filename().string(), current_file) ;
            }
            else get_file_list(prefix + itr->path().stem().string(), itr->path(), files) ;
        }
    }

}

bool MapFile::addResource(const string &key, const fs::path &filePath)
{
    SQLite::Session session(db_) ;
    SQLite::Connection &con = session.handle() ;

    SQLite::Command stmt(con, "REPLACE INTO __resources (name,sz,data) VALUES(?,?,?)") ;

    SQLite::Transaction trans(con) ;

    Dictionary files ;
    get_file_list(key, filePath, files) ;

    DictionaryIterator it(files) ;

    while (it)
    {
        string key = it.key() ;
        string file_path = it.value() ;

        uint sz ;
        string content = read_file(file_path, sz) ;

        stmt.bind(key) ;
        stmt.bind((int)sz) ;
        stmt.bind(content.data(), content.size()) ;

        stmt.exec() ;
        stmt.clear() ;

        ++it ;
    }

    trans.commit() ;

}

