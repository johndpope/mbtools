#ifndef __MAP_CONFIG_H__
#define __MAP_CONFIG_H__

#include <vector>

#include "OsmRuleParser.h"
#include "OsmDocument.h"


struct BBox {
    double minx_, miny_, maxx_, maxy_ ;
    uint srid_ ;
};

struct ZoomInterval {
    ZoomInterval(): min_zoom_(-1), max_zoom_(-1), simplify_threshold_(0) {}

    int min_zoom_ ;
    int max_zoom_ ;
    float simplify_threshold_ ;
} ;

struct ZoomRange {
    std::vector<ZoomInterval> intervals_ ;

    bool isInZoomRange(uint z) {
        for (auto iv: intervals_) {
            if ( iv.min_zoom_ == -1 && z <= iv.max_zoom_ ) return true ;
            else if ( iv.max_zoom_ == -1 && z >= iv.min_zoom_ ) return true ;
            else if ( z >= iv.min_zoom_ && z <= iv.max_zoom_ ) return true ;
        }
        return false ;
    }
} ;

struct Rule {
    OSM::Rule::ExpressionNode *condition_ ;
    OSM::Rule::Command *actions_ ;
};

struct Layer {
    Layer(): srid_("3857") {}
    ZoomRange zr_ ;
    std::string name_ ;
    std::string geom_ ;
    std::string srid_ ;
    std::vector<std::string> tags_, types_ ;
    std::vector<Rule> rules_ ;
};

struct Action {
    std::string key_ ;
    OSM::Rule::Literal val_ ;
};

struct NodeRuleMap {
    int node_idx_ ;
    std::vector<uint> matched_rules_ ;
};

struct MapConfig {
    MapConfig(): minz_(10), maxz_(16), has_bbox_(false) {}

    std::vector<Layer> layers_ ;
    BBox bbox_ ;
    std::string name_ ;
    std::string description_ ;
    std::string attribution_ ;
    int minz_, maxz_ ;
    bool has_bbox_ ;

    bool parse(const std::string &fileName) ;
};

#endif
