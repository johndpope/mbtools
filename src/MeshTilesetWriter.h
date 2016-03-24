#ifndef __MESH_TILESET_WRITER_H__
#define __MESH_TILESET_WRITER_H__

#include "MapConfig.h"
#include "Database.h"
#include <boost/filesystem.hpp>

class MeshTilesetWriter {
public:
    MeshTilesetWriter(const std::string &fileName) ;

    bool writeTilesDB(const std::string &mesh_file, MapConfig &cfg) ;
    bool writeTilesFolder(const std::string &mesh_file, MapConfig &cfg) ;

    bool writeMetaData(const std::string &name, const std::string &val);

private:

    std::unique_ptr<SQLite::Database> db_ ;
    boost::filesystem::path tileset_ ;
};




















#endif
