#ifndef __OSM_DOCUMENT_H__
#define __OSM_DOCUMENT_H__

#include <string>
#include <map>
#include <vector>
#include <deque>

#include <Dictionary.h>

namespace OSM {


struct Feature {

    enum Type { NodeFeature, WayFeature, RelationFeature, PolygonFeature } ;

    Feature(Type type_): type(type_), visited_(false) {}

    std::string id ; // feature id
    Dictionary tags ; // tags associated with this feature
    Type type ; // feature type ;
    bool visited_ ; // used by algorithms ;

};

class Way ;
class Relation ;

struct Node: public Feature {

    Node(): Feature(NodeFeature) {}

    double lat, lon ;

    std::vector<int> ways ;     // ways in which this nodes participates
    std::vector<int> relations ; // relations that this node participates directly

} ;


struct Way: public Feature {

    Way(): Feature(WayFeature) {}

    std::vector<int> nodes ; // nodes corresponding to this way
    std::vector<int> relations ; // relations that this way participates
} ;


struct Relation: public Feature {

    Relation(): Feature(RelationFeature) {}

    std::vector<int> nodes ;     // node members
    std::vector<int> ways ;      // way members
    std::vector<int> children ; // relation members

    std::vector<std::string> nodes_role ;
    std::vector<std::string> ways_role ;
    std::vector<std::string> children_role ;

    std::vector<int> parents ;    // parent relations
};

struct Ring {
    std::deque<int> nodes ;
};

struct Polygon: public Feature {

    Polygon(): Feature(PolygonFeature) {}

    std::vector<Ring> rings ;
};


class Document {
public:

    // empty document
    Document() {}

    // open an existing document
    Document(const std::string &fileName) ;

    // create OSM document by filtering an existing one
    Document(const Document &other, const std::string &filter) ;

    // read Osm file (format determined by extension)
    bool read(const std::string &fileName) ;

    // write Osm file (format determined by extension)
    void write(const std::string &fileName) ;

public:

    std::vector<Node> nodes ;
    std::vector<Way> ways ;
    std::vector<Relation> relations ;

protected:

    bool readXML(std::istream &strm) ;
    void writeXML(std::ostream &strm);

    bool readPBF(const std::string &fileName) ;
    bool isPBF(const std::string &fileName) ;


};

bool makePolygonsFromRelation(const Document &doc, const Relation &rel, Polygon &polygon) ;
bool makeWaysFromRelation(const Document &doc, const Relation &rel, std::vector<Way> &ways) ;

}

#endif
