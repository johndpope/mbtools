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

    fs::directory_iterator end_itr;

    for (fs::directory_iterator itr(filePath); itr != end_itr; ++itr)
    {
        if (fs::is_regular_file(itr->path())) {
            string current_file = itr->path().string();
            files.add(key + '/' + itr->path().filename().string(), current_file) ;
        }
        else get_file_list(key + '/' + itr->path().stem().string(), itr->path(), files) ;
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

#if 0
/*
** Make sure the parent directory for zName exists.  Create it if it does
** not exist.
*/
static void make_parent_directory(const char *zName){
    char *zParent;
    int i, j, rc;
    for(i=j=0; zName[i]; i++) if( zName[i]=='/' ) j = i;
    if( j>0 ){
        zParent = sqlite3_mprintf("%.*s", j, zName);
        if( zParent==0 ) errorMsg("mprintf failed\n");
        while( j>0 && zParent[j]=='/' ) j--;
        zParent[j] = 0;
        if( j>0 && access(zParent,F_OK)!=0 ){
            make_parent_directory(zParent);
            rc = mkdir(zParent, 0777);
            if( rc ) errorMsg("cannot create directory: %s\n", zParent);
        }
        sqlite3_free(zParent);
    }
}

/*
** Write a file or a directory.
**
** Create any missing directories leading up to the given file or directory.
** Also set the access mode and the modification time.
**
** If sz>nCompr that means that the content is compressed and needs to be
** decompressed before writing.
*/
static void write_file(
        const char *zFilename,   /* Store content in this file */
        int iMode,               /* The unix-style access mode */
        sqlite3_int64 mtime,     /* Modification time */
        int sz,                  /* Size of file as stored on disk */
        const char *pCompr,      /* Content (usually compressed) */
        int nCompr               /* Size of content (prior to decompression) */
        ){
    char *pOut;
    unsigned long int nOut;
    int rc;
    FILE *out;
    make_parent_directory(zFilename);
    if( pCompr==0 ){
        rc = mkdir(zFilename, iMode);
        if( rc ) errorMsg("cannot make directory: %s\n", zFilename);
        return;
    }
    out = fopen(zFilename, "wb");
    if( out==0 ) errorMsg("cannot open for writing: %s\n", zFilename);
    if( sz==nCompr ){
        if( sz>0 && fwrite(pCompr, sz, 1, out)!=1 ){
            errorMsg("failed to write: %s\n", zFilename);
        }
    }else{
        pOut = sqlite3_malloc( sz+1 );
        if( pOut==0 ) errorMsg("cannot allocate %d bytes\n", sz+1);
        nOut = sz;
        rc = uncompress((Bytef*)pOut, &nOut, (const Bytef*)pCompr, nCompr);
        if( rc!=Z_OK ) errorMsg("uncompress failed for %s\n", zFilename);
        if( nOut>0 && fwrite(pOut, nOut, 1, out)!=1 ){
            errorMsg("failed to write: %s\n", zFilename);
        }
        sqlite3_free(pOut);
    }
    fclose(out);
    rc = chmod(zFilename, iMode&0777);
    if( rc ) errorMsg("cannot change mode to %03o: %s\n", iMode, zFilename);
}

/*
** Error out if there are any issues with the given filename
*/
static void check_filename(const char *z){
    if( strncmp(z, "../", 3)==0 || sqlite3_strglob("*/../*", z)==0 ){
        errorMsg("Filename with '..' in its path: %s\n", z);
    }
    if( sqlite3_strglob("*\\*", z)==0 ){
        errorMsg("Filename with '\\' in its name: %s\n", z);
    }
}

/*
** Add a file to the database.
*/
static void add_file(
        const char *zFilename,     /* Name of file to add */
        int verboseFlag,           /* If true, show each file added */
        int noCompress             /* If true, always omit compression */
        ){
    int rc;
    struct stat x;
    int szOrig;
    int szCompr;
    const char *zName;

    check_filename(zFilename);
    rc = stat(zFilename, &x);
    if( rc ) errorMsg("no such file or directory: %s\n", zFilename);
    if( x.st_size>1000000000 ){
        errorMsg("file too big: %s\n", zFilename);
    }
    if( pStmt==0 ){
        db_prepare("REPLACE INTO sqlar(name,mode,mtime,sz,data)"
                   " VALUES(?1,?2,?3,?4,?5)");
    }
    zName = zFilename;
    while( zName[0]=='/' ) zName++;
    sqlite3_bind_text(pStmt, 1, zName, -1, SQLITE_STATIC);
    sqlite3_bind_int(pStmt, 2, x.st_mode);
    sqlite3_bind_int64(pStmt, 3, x.st_mtime);
    if( S_ISREG(x.st_mode) ){
        char *zContent = read_file(zFilename, &szOrig, &szCompr, noCompress);
        sqlite3_bind_int(pStmt, 4, szOrig);
        sqlite3_bind_blob(pStmt, 5, zContent, szCompr, sqlite3_free);
        if( verboseFlag ){
            if( szCompr<szOrig ){
                int pct = szOrig ? (100*(sqlite3_int64)szCompr)/szOrig : 0;
                printf("  added: %s (deflate %d%%)\n", zFilename, 100-pct);
            }else{
                printf("  added: %s\n", zFilename);
            }
        }
    }else{
        sqlite3_bind_int(pStmt, 4, 0);
        sqlite3_bind_null(pStmt, 5);
        if( verboseFlag ) printf("  added: %s\n", zFilename);
    }
    rc = sqlite3_step(pStmt);
    if( rc!=SQLITE_DONE ){
        errorMsg("Insert failed for %s: %s\n", zFilename, sqlite3_errmsg(db));
    }
    sqlite3_reset(pStmt);
    if( S_ISDIR(x.st_mode) ){
        DIR *d;
        struct dirent *pEntry;
        char *zSubpath;
        d = opendir(zFilename);
        if( d ){
            while( (pEntry = readdir(d))!=0 ){
                if( strcmp(pEntry->d_name,".")==0 || strcmp(pEntry->d_name,"..")==0 ){
                    continue;
                }
                zSubpath = sqlite3_mprintf("%s/%s", zFilename, pEntry->d_name);
                add_file(zSubpath, verboseFlag, noCompress);
                sqlite3_free(zSubpath);
            }
            closedir(d);
        }
    }
}

/*
** List of command-line arguments
*/
typedef struct NameList NameList;
struct NameList {
    char **azName;   /* List of names */
    int nName;       /* Number of names on the list */
};

/*
** Inplementation of SQL function "name_on_list(X)".  Return
** true if X is on the list of names given on the command-line.
*/
static void name_on_list(
        sqlite3_context *context,
        int argc,
        sqlite3_value **argv
        ){
    NameList *pList = (NameList*)sqlite3_user_data(context);
    int i;
    int rc = 0;
    const char *z = (const char*)sqlite3_value_text(argv[0]);
    if( z!=0 ){
        for(i=0; i<pList->nName; i++){
            if( strcmp(pList->azName[i], z)==0 ){
                rc = 1;
                break;
            }
        }
    }
    sqlite3_result_int(context, rc);
}
#endif
