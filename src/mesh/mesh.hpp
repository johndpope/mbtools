#ifndef __TIN_MESH_H__
#define __TIN_MESH_H__

#include <vector>
#include <string>
#include <limits>

namespace tin {

struct Vertex3 {
    float x_, y_, z_ ;
};

class Mesh {
public:

    Mesh() = default ;

    // loads 3D coordinates from file in OGR XYZ format and Global mercator coordinates
    bool loadXYZ(const std::string &fileName, int nodata) ;

    // triangulates the 2D coordinates to form a mesh (area is the minimum triangle area for constraint Delaunay triangulation)
    bool triangulateCoords(float area = 0);

    // decimate mesh to given number of target points and approximation error
    bool decimate(uint npts, float target_error = std::numeric_limits<float>::max()) ;

    // load from wavefront OBJ
    bool load(const std::string &fileName) ;

    // save to wavefront OBJ
    void save(const std::string &fileName);


public:

    std::vector<Vertex3> coords_ ;
    std::vector<uint> triangles_ ;
    std::vector<Vertex3> normals_ ;
} ;

}
#endif
