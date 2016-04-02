#include <unordered_map>
#include <set>
#include <boost/filesystem.hpp>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/gzip_stream.h>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "map/geom_helpers.hpp"
#include "mesh/mesh_tile_writer.hpp"
#include "mesh/mesh.hpp"

using namespace std ;

class Quadtree {
public:

    ~Quadtree() {
        for( auto t: children_)
            if ( t ) delete t ;
    }

    Quadtree(const BBox &box, uint level, uint32_t max_levels): children_{nullptr}, level_(level), box_(box) {
        if (level < max_levels) {
            double w = box.width(), h = box.height() ;
            level ++ ;
            children_[0] = new Quadtree({box.minx_, box.miny_, box.minx_ + w/2.0, box.miny_ + h/2.0}, level, max_levels);
            children_[1] = new Quadtree({box.minx_, box.miny_+ h/2.0, box.minx_ + w/2.0, box.maxy_}, level, max_levels);
            children_[2] = new Quadtree({box.minx_ + w/2.0, box.miny_, box.maxx_, box.miny_ + h/2.0}, level, max_levels);
            children_[3] = new Quadtree({box.minx_ + w/2.0, box.miny_ + w/2.0, box.maxx_, box.maxy_}, level, max_levels);
        }
    }

    void addVertex(float x, float y, uint64_t idx) {

        if ( !box_.contains(x, y) ) return ;

        if ( children_[0] == nullptr ) {
            idxs_.push_back(idx) ;
            return;
        }

        for ( auto child: children_ ) {
            if ( child->box_.contains(x, y) ) {
                child->addVertex(x, y, idx) ;
                return ;
            }
        }
    }

    void query(const BBox &qb, vector<uint64_t> &idxs) {

        if ( !box_.intersects(qb) ) return ;

        if ( children_[0] == nullptr ) {
            std::copy(idxs_.begin(), idxs_.end(), std::back_inserter(idxs)) ;
            return;
        }

        for ( auto child: children_ ) {
            if ( child->box_.intersects(qb) ) {
                child->query(qb, idxs) ;
            }
        }
    }


private:
    uint32_t level_ ;
    Quadtree *children_[4] ;
    BBox box_ ;
    vector<uint64_t> idxs_ ;

};

class MeshHelper {
public:
    MeshHelper() {}

    ~MeshHelper() {
        delete index_ ;
    }

    bool load(const string &file_name, const BBox &box) {
        // load mesh
        if ( !mesh_.load(file_name) ) return false ;

        // create map of faces adjacent to each vertex

        for( uint i=0 ; i<mesh_.triangles_.size() ; i++)
            vfmap_.insert(std::make_pair(mesh_.triangles_[i], i/3)) ;

        index_ = new Quadtree(box, 0, 5) ;

        for( uint i=0 ; i<mesh_.coords_.size() ; i++) {
            index_->addVertex(mesh_.coords_[i].x_, mesh_.coords_[i].y_, i) ;
        }

    }

    // we handle the case that some vertices of a face are outside the box and thus they have to be inserted also

    void safe_vertex_add(uint64_t v0, uint64_t &k, unordered_map<uint64_t, uint64_t> &vtx_map,
                  vector<tin::Vertex3> &coords, vector<tin::Vertex3> &normals, vector<uint64_t> &triangles ) {
        auto it = vtx_map.find(v0) ;
        if ( it != vtx_map.end() ) v0 = it->second ;
        else {
            const tin::Vertex3 &c = mesh_.coords_[v0] ;
            const tin::Vertex3 &n = mesh_.normals_[v0] ;
            coords.push_back(c) ;
            normals.push_back(n) ;
            vtx_map[v0] = k ;
            v0 = k ;
            k++ ;
        }

        triangles.push_back(v0) ;
    }

    void queryMesh(const BBox &box, vector<tin::Vertex3> &coords, vector<tin::Vertex3> &normals, vector<uint64_t> &triangles) {
        vector<uint64_t> idxs ;
        index_->query(box, idxs) ;

        unordered_map<uint64_t, uint64_t> vtx_map ;
        set<uint64_t> fset ;

        uint64_t k=0 ;
        for( uint64_t idx: idxs) {
            const tin::Vertex3 &c = mesh_.coords_[idx] ;
            const tin::Vertex3 &n = mesh_.normals_[idx] ;
            if ( box.contains(c.x_, c.y_) ) {

                coords.push_back(c) ;
                normals.push_back(n) ;
                vtx_map[idx] = k++ ;

                auto fr = vfmap_.equal_range(idx) ;
                for( auto fit = fr.first ; fit != fr.second ; fit++ )
                    fset.insert(fit->second) ;
            }
        }

        for( uint64_t face: fset ) {
            uint64_t v0 = mesh_.triangles_[3 * face] ;
            uint64_t v1 = mesh_.triangles_[3 * face + 1] ;
            uint64_t v2 = mesh_.triangles_[3 * face + 2] ;

            safe_vertex_add(v0, k, vtx_map, coords, normals, triangles) ;
            safe_vertex_add(v1, k, vtx_map, coords, normals, triangles) ;
            safe_vertex_add(v2, k, vtx_map, coords, normals, triangles) ;
        }
    }

    tin::Mesh mesh_ ;
    std::unordered_multimap<uint64_t, uint64_t> vfmap_ ;
    Quadtree *index_ ;
};

MeshTileWriter::MeshTileWriter(const std::string &file_name, const MapConfig &cfg): mesh_(new MeshHelper), cfg_(cfg)
{
    mesh_->load(file_name, cfg_.bbox_) ;
}


static void encodeMesh(mesh_tile::Tile &tile_msg, vector<tin::Vertex3> &coords, vector<tin::Vertex3> &normals, vector<uint64_t> &triangles)
{
    mesh_tile::Tile_Mesh *mesh = tile_msg.mutable_mesh() ;

    for( uint i = 0 ; i<coords.size() ; i++ ) {
        mesh->add_coords(coords[i].x_) ;
        mesh->add_coords(coords[i].y_) ;
    }

    for( auto t: triangles )
        mesh->add_triangles(t) ;

    mesh_tile::Tile_Channel *echannel = tile_msg.add_channels() ;
    echannel->set_name("z") ;
    echannel->set_dimensions(1) ;

    for( uint i = 0 ; i<coords.size() ; i++ ) {
        echannel->add_data(coords[i].z_) ;
    }

    mesh_tile::Tile_Channel *nchannel = tile_msg.add_channels() ;
    nchannel->set_name("normals") ;
    nchannel->set_dimensions(3) ;

    for( uint i = 0 ; i<coords.size() ; i++ ) {
        nchannel->add_data(normals[i].x_) ;
        nchannel->add_data(normals[i].y_) ;
        nchannel->add_data(normals[i].z_) ;
    }
}

bool MeshTileWriter::queryTile(uint32_t tx, uint32_t ty, uint32_t tz, uint32_t te)
{
    BBox box ;
    tms::tileBounds(tx, ty, tz, box.minx_, box.miny_, box.maxx_, box.maxy_, 0) ;
    box.srid_ = 3857 ;

    vector<tin::Vertex3> coords, normals ;
    vector<uint64_t> triangles ;

    mesh_->queryMesh(box, coords, normals, triangles) ;

    tile_msg_.Clear() ;

    encodeMesh(tile_msg_, coords, normals, triangles) ;


    ofstream strm("/tmp/oo.obj") ;

    for(uint i=0 ; i<coords.size() ; i++) {
        strm << std::setiosflags(ios::fixed) << "v " << coords[i].x_ << ' ' << coords[i].y_ << ' ' << coords[i].z_ << endl ;
        strm << std::setiosflags(ios::fixed) << "vn " << normals[i].x_ << ' ' << normals[i].y_ << ' ' << normals[i].z_ << endl ;
    }

    for(uint i=0 ; i<triangles.size() ; i+=3) {
        strm << "f " << triangles[i] + 1 << ' ' << triangles[i+1] + 1 << ' ' << triangles[i+2] + 1 << endl ;
    }

}

string MeshTileWriter::toString(bool compress)
{
    string res ;
    if ( !compress ) {
        tile_msg_.SerializeToString(&res) ;
        return res ;
    }
    else {
        ::google::protobuf::io::StringOutputStream compressedStream(&res);
        ::google::protobuf::io::GzipOutputStream compressingStream(&compressedStream);
        tile_msg_.SerializeToZeroCopyStream(&compressingStream);
        return res ;
    }
}
