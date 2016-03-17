#ifndef __IMPORT_CONFIG_H__
#define __IMPORT_CONFIG_H__

#include <vector>

#include "OsmRuleParser.h"
#include "OsmDocument.h"

struct Action {
    std::string key_ ;
    OSM::Filter::Literal val_ ;
};

struct NodeRuleMap {
    int node_idx_ ;
    std::vector<const OSM::Filter::Rule *> matched_rules_ ;
};

struct ImportConfig {
    ImportConfig(){}

    OSM::Filter::LayerDefinition *layers_ ;

    bool parse(const std::string &fileName) ;
};

#endif
