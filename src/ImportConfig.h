#ifndef __IMPORT_CONFIG_H__
#define __IMPORT_CONFIG_H__

#include <vector>

#include "OsmRuleParser.h"
#include "OsmDocument.h"


struct Rule {
    OSM::Rule::ExpressionNode *condition_ ;
    OSM::Rule::Command *actions_ ;
};

struct ImportLayer {
    ImportLayer(): srid_("3857") {}

    std::string name_ ;
    std::string geom_ ;
    std::string srid_ ;

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

struct ImportConfig {
    ImportConfig(){}

    std::vector<ImportLayer> layers_ ;

    bool parse(const std::string &fileName) ;
};

#endif
