#ifndef __MBTILE_WRITER_H__
#define __MBTILE_WRITER_H__

#include "MapFile.h"

class MBTileWriter {
public:
    MBTileWriter(const std::string &fileName) ;

    bool writeTiles(const MapFile &map, MapConfig &cfg) ;
    bool writeMetaData(const std::string &name, const std::string &val);

private:

    std::unique_ptr<SQLite::Database> db_ ;
};




















#endif
