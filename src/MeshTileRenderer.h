#ifndef __MESH_TILE_RENDERER_H__
#define __MESH_TILE_RENDERER_H__

#include <stdint.h>
#include <string>
#include <memory>

#include "GeomHelpers.h"

struct RenderingContext ;

class MeshTileRenderer {
public:

    MeshTileRenderer(uint32_t tile_size = 256) ;
    ~MeshTileRenderer() ;

    // will render a mesh tile encoded in protobuf string into an image and return the PNG encoded image bytes
    std::string render(uint32_t x, uint32_t y, uint32_t z, const std::string &tilebytes, const std::string &pname) ;

private:

    uint32_t tile_size_ ;
    std::shared_ptr<RenderingContext> ctx_ ;
} ;


#endif
