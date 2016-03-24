#include "Mesh.h"

#include <fstream>
#include <cmath>
#include <limits>
#include <iomanip>

extern "C" {
#define REAL double
#define VOID void
#define ANSI_DECLARATORS
#include "triangle.h"
}

// stuff to define the mesh
#include <vcg/complex/complex.h>
#include <vcg/complex/append.h>

// io
#include <wrap/io_trimesh/import.h>
#include <wrap/io_trimesh/export_obj.h>

// local optimization
#include <vcg/complex/algorithms/local_optimization.h>
#include <vcg/complex/algorithms/local_optimization/tri_edge_collapse_quadric.h>
#include <vcg/complex/algorithms/hole.h>
#include <vcg/space/triangle3.h>

using namespace std;

namespace tin {

bool Mesh::loadXYZ(const std::string &fileName, int nodata) {

    ifstream strm(fileName.c_str()) ;

    float X, Y, Z ;

    while ( 1 ) {
        strm >> X >> Y >> Z ;

        if ( !strm ) break ;

        if ( fabs(Z -nodata) >  std::numeric_limits<float>::epsilon() )  {
            coords_.push_back({X, Y, Z}) ;
        }
    }

    return true ;
}

bool Mesh::triangulateCoords(float area)
{
    char triswitches[20] ;
    int i, k ;

    if ( area > 0.0 ) sprintf(triswitches, "zqQa%f", area) ;
    sprintf(triswitches, "zQ") ;

    struct triangulateio trsin, trsout ;

    int n = trsin.numberofpoints = coords_.size() ;
    double *coords = new double [n * 2] ;
    trsin.pointlist = coords ;

    double *attr_list = new double [n] ;

    trsin.numberofpointattributes = 1 ;
    trsin.pointattributelist = attr_list ;

    for(i=0, k=0 ; i<n ; i++, k+=2) {
        const Vertex3 &p = coords_[i] ;
        trsin.pointlist[k]   = (double)p.x_ ;
        trsin.pointlist[k+1] = (double)p.y_ ;
        attr_list[i] = p.z_ ;
    }

    trsin.pointmarkerlist = NULL ;

    trsin.numberofsegments = 0 ;
    trsin.numberofholes = 0 ;
    trsin.numberofregions = 0 ;

    trsout.pointlist = (double *)NULL ;
    trsout.pointattributelist = (double *)NULL ;
    trsout.pointmarkerlist = (int *)NULL ;
    trsout.trianglelist = (int *) NULL;
    trsout.triangleattributelist = (double *) NULL;

    triangulate(triswitches, &trsin, &trsout, NULL) ;

    coords_.clear() ;

    for(i=0, k=0 ; i<trsout.numberofpoints ; i++, k+=2) {
        REAL x, y, z ;

        x = trsout.pointlist[k] ;
        y = trsout.pointlist[k+1] ;
        z = trsout.pointattributelist[i] ;

        coords_.push_back({x, y, z}) ;
    }

    for(i=0 ; i < 3 * trsout.numberoftriangles ; i++ ) {
        triangles_.push_back(trsout.trianglelist[i]) ;
    }

    free(trsout.pointlist) ;
    free(trsout.pointmarkerlist) ;
    free(trsout.trianglelist) ;
    free(trsout.pointattributelist) ;

    delete [] attr_list ;
    delete [] coords ;

    return true ;
}

class MyVertex;
class MyEdge;
class MyFace;

struct MyUsedTypes: public vcg::UsedTypes<vcg::Use<MyVertex>::AsVertexType,vcg::Use<MyEdge>::AsEdgeType,vcg::Use<MyFace>::AsFaceType>{};

class MyVertex  : public vcg::Vertex< MyUsedTypes,
        vcg::vertex::VFAdj,
        vcg::vertex::Coord3f,
        vcg::vertex::Normal3f,
        vcg::vertex::Mark,
        vcg::vertex::BitFlags  >{
public:
    vcg::math::Quadric<double> &Qd() {return q;}
private:
    vcg::math::Quadric<double> q;
};

class MyEdge : public vcg::Edge< MyUsedTypes> {};

typedef BasicVertexPair<MyVertex> VertexPair;

class MyFace    : public vcg::Face< MyUsedTypes,
        vcg::face::VFAdj,
        vcg::face::FFAdj,
        vcg::face::VertexRef,
        vcg::vertex::Normal3f,
        vcg::face::Mark,
        vcg::face::BitFlags
        > {};

// the main mesh class
class MyMesh:
        public vcg::tri::TriMesh<std::vector<MyVertex>, std::vector<MyFace> > {

public:

    typedef vcg::Point3f NormalType;

    // create mesh from buffers

    void create(const std::vector<Vertex3> &coords, const std::vector<uint> &triangles) {

        uint nb_vert = coords.size() ;
        uint nb_face = triangles.size()/3 ;

        vcg::tri::Allocator<MyMesh>::AddVertices(*this, nb_vert);
        vcg::tri::Allocator<MyMesh>::AddFaces   (*this, nb_face);

        std::vector<MyMesh::VertexPointer> ivp( nb_vert );
        MyMesh::VertexIterator vi = vert.end();

        for(int i = (nb_vert-1); i >= 0; --i)
        {
            --vi;
            ivp[i] = & *vi;
            (*vi).P() = MyMesh::CoordType(coords[i].x_, coords[i].y_, coords[i].z_);
        }

        MyMesh::FaceIterator fi = face.end();

        for(int i = (nb_face-1); i >= 0; --i)
        {
            uint v1 = triangles[i*3 + 0] ;
            uint v2 = triangles[i*3 + 1] ;
            uint v3 = triangles[i*3 + 2] ;

            --fi;
            (*fi).V(0) = ivp[ v1 ];
            (*fi).V(1) = ivp[ v2 ];
            (*fi).V(2) = ivp[ v3 ];
        }
    }

    // read mesh to buffers

    void read(std::vector<Vertex3> &coords, std::vector<uint> &triangles, vector<Vertex3> &normals) {


        ConstVertexIterator vci = vert.begin();

        vector<uint> vertexIDs(vert.size()) ;

        for( int k=0 ; vci != vert.end() ; ++vci) {

            if ( !(*vci).IsD() ) {
                MyMesh::CoordType coord = (*vci).P();
                MyMesh::NormalType normal = (*vci).N();

                coords.push_back({coord.X(), coord.Y(), coord.Z()}) ;
                normals.push_back({normal.X(), normal.Y(), normal.Z()}) ;

                vertexIDs[ vci- vert.begin() ] = k++ ;
            }
        }

        ConstFaceIterator fci = face.begin();

        for( ; fci != face.end() ; ++fci) {

            if ( !(*fci).IsD() ) {

                uint v0 = vertexIDs[vcg::tri::Index(*this, (*fci).cV(0))] ;
                uint v1 = vertexIDs[vcg::tri::Index(*this, (*fci).cV(1))] ;
                uint v2 = vertexIDs[vcg::tri::Index(*this, (*fci).cV(2))] ;

                triangles.push_back(v0) ;
                triangles.push_back(v1) ;
                triangles.push_back(v2) ;
            }
        }
    }

};


class MyTriEdgeCollapse: public vcg::tri::TriEdgeCollapseQuadric< MyMesh, VertexPair, MyTriEdgeCollapse, QInfoStandard<MyVertex>  > {
public:
    typedef  vcg::tri::TriEdgeCollapseQuadric< MyMesh,  VertexPair, MyTriEdgeCollapse, QInfoStandard<MyVertex>  > TECQ;
    typedef  MyMesh::VertexType::EdgeType EdgeType;
    inline MyTriEdgeCollapse(  const VertexPair &p, int i, vcg::BaseParameterClass *pp) :TECQ(p,i,pp){}
};


static void compute_normals(MyMesh &m)
{
    vcg::tri::UpdateNormal<MyMesh>::PerVertexClear(m);

    for(MyMesh::FaceIterator f=m.face.begin();f!=m.face.end();++f)

    if( !(*f).IsD() && (*f).IsR() )
    {
        typename MyFace::NormalType t = vcg::TriangleNormal(*f);
        float n = t.Norm() ;

    for(int j=0; j<(*f).VN(); ++j)
     if( !(*f).V(j)->IsD() && (*f).V(j)->IsRW() && n > 1000 )
      (*f).V(j)->N() += t;
   }

    vcg::tri::UpdateNormal<MyMesh>::NormalizePerVertex(m) ;
}

bool Mesh::decimate(uint target_pts, float target_error) {

    TriEdgeCollapseQuadricParameter qparams ;
    qparams.QualityThr  = 1;

    MyMesh mesh ;
    mesh.create(coords_, triangles_) ;

    vcg::tri::UpdateBounding<MyMesh>::Box(mesh);

    vcg::tri::Clean<MyMesh>::RemoveDuplicateVertex(mesh);
    vcg::tri::Clean<MyMesh>::RemoveUnreferencedVertex(mesh);

    // decimator initialization

    qparams.OptimalPlacement	= true;
    qparams.PreserveBoundary	= false;
    qparams.PreserveTopology	= false;
    qparams.QualityQuadric = true ;
    qparams.QualityWeight = false ;
    qparams.BoundaryWeight = 1.0 ;

    if ( target_pts == 0 )
        target_pts = 0.1 * mesh.VN() ;

    vcg::LocalOptimization<MyMesh> deci_session(mesh, &qparams);

    deci_session.Init<MyTriEdgeCollapse>();
    deci_session.SetTargetVertices(target_pts);

    if( target_error < std::numeric_limits<float>::max() ) deci_session.SetTargetMetric(target_error);

    while( deci_session.DoOptimization() && mesh.vn > target_pts && deci_session.currMetric < target_error ) ;

    compute_normals(mesh) ;

    coords_.clear() ; triangles_.clear(), normals_.clear() ;

    mesh.read(coords_, triangles_, normals_) ;

    return true ;
}

bool Mesh::load(const string &file_name)
{
   MyMesh mesh ;
   vcg::tri::io::ImporterOBJ<MyMesh>::Info info ;
   int err = vcg::tri::io::ImporterOBJ<MyMesh>::Open(mesh, file_name.c_str(), info);
   if ( err ) return false ;

   mesh.read(coords_, triangles_, normals_) ;

   return true ;
}


void Mesh::save(const string &fileName) {
    ofstream strm(fileName.c_str()) ;

    for(uint i=0 ; i<coords_.size() ; i++) {
        strm << std::setiosflags(ios::fixed) << "v " << coords_[i].x_ << ' ' << coords_[i].y_ << ' ' << coords_[i].z_ << endl ;
        strm << std::setiosflags(ios::fixed) << "vn " << normals_[i].x_ << ' ' << normals_[i].y_ << ' ' << normals_[i].z_ << endl ;
    }

    for(uint i=0 ; i<triangles_.size() ; i+=3) {
        strm << "f " << triangles_[i] + 1 << ' ' << triangles_[i+1] + 1 << ' ' << triangles_[i+2] + 1 << endl ;
    }
}

}
