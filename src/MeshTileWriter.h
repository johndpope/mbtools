#ifndef __MESH_TILE_WRITER_H__
#define __MESH_TILE_WRITER_H__

#include <vector>
#include <string>
#include <map>

#include <memory>

#include "MapConfig.h"
#include "mesh_tile.pb.h"

class MeshHelper ;

class MeshTileWriter {
public:

    MeshTileWriter(const std::string &file_name, const MapConfig &cfg)  ;

    bool queryTile(uint32_t tx, uint32_t ty, uint32_t tz, uint32_t te = 4096) ;

    std::string toString(bool compress = true) ;

private:

    mesh_tile::Tile tile_msg_ ;
    double tile_origin_x_, tile_origin_y_ ;
    uint32_t te_, tx_, ty_, tz_ ;

private:

    std::shared_ptr<MeshHelper> mesh_ ;
    const MapConfig &cfg_ ;
    void load_mesh(const std::string &file_name) ;
};




















#endif
