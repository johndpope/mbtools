#ifndef __VECTOR_TILE_H__
#define __VECTOR_TILE_H__

#include <Dictionary.h>

#include <vector>
#include <string>
#include <map>

#include "GeomHelpers.h"

class VectorTile {
public:

    struct Feature {
        uint64_t id_ ;
        Geometry geom_ ;
        Dictionary tags_ ;
    };

    struct Layer {
        std::string name_ ;
        Box extent_ ;
        std::vector<Feature> features_ ;
    };

    VectorTile() ;

    std::vector<Layer> layers_ ;

};











#endif
