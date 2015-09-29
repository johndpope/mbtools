#include "OsmDocument.h"
#include "OsmRuleParser.h"

#include <stdlib.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <iomanip>
#include <set>

#include <XmlReader.h>
#include <zfstream.h>

using namespace std ;

namespace OSM {

bool Document::readXML(istream &strm)
{
    XmlReader rd(strm) ;

    map<string, int> nodeMap, wayMap, relMap ;
    vector<  vector<string> > wayNodeMap, relNodeMap, relWayMap, relRelMap ;
    vector<  vector<string> > relNodeMapRole, relWayMapRole, relRelMapRole ;

    if ( !rd.readNextStartElement("osm") ) return false ;

    while ( rd.read() )
    {
        if ( rd.readNextStartElement() )
        {
            if ( rd.nodeName() == "node" )
            {
                Node node ;

                node.id = rd.attribute("id") ;

                if ( node.id.empty() ) return false ;

                node.lat = atof(rd.attribute("lat").c_str()) ;
                node.lon = atof(rd.attribute("lon").c_str()) ;

                while ( rd.read() )
                {
                    if ( rd.isStartElement("tag") )
                    {
                        string key_ = rd.attribute("k")  ;
                        string val_ = rd.attribute("v") ;

                        node.tags[key_] = val_ ;
                    }
                    else if ( rd.isEndElement("node" ) ) break ;
                }

                nodeMap[node.id] = nodes.size() ;
                nodes.push_back(node) ;

            }
            else if ( rd.nodeName() == "way" )
            {
                Way way ;

                way.id = rd.attribute("id") ;

                wayNodeMap.push_back(vector<string>()) ;
                vector<string> &map_item =  wayNodeMap.back() ;

                if ( way.id.empty() ) return false ;

                while ( rd.read() )
                {
                    if ( rd.isStartElement("nd") )
                    {
                        string ref_ = rd.attribute("ref")  ;

                        if ( ref_.empty()  ) return false ;

                        map_item.push_back(ref_) ;
                    }
                    else if ( rd.isStartElement("tag"))
                    {
                        string key_ = rd.attribute("k")  ;
                        string val_ = rd.attribute("v") ;

                        way.tags[key_] = val_ ;
                    }
                    else if ( rd.isEndElement("way" ) ) break ;
                }

                wayMap[way.id] = ways.size() ;
                ways.push_back(way) ;

            }
            else if ( rd.nodeName() == "relation" )
            {
                Relation relation ;

                relation.id = rd.attribute("id") ;

                if (relation.id.empty() ) return false ;

                relNodeMap.push_back(vector<string>()) ;
                relWayMap.push_back(vector<string>()) ;
                relRelMap.push_back(vector<string>()) ;

                relNodeMapRole.push_back(vector<string>()) ;
                relWayMapRole.push_back(vector<string>()) ;
                relRelMapRole.push_back(vector<string>()) ;

                vector<string> &node_map_item = relNodeMap.back() ;
                vector<string> &way_map_item = relWayMap.back() ;
                vector<string> &rel_map_item = relRelMap.back() ;

                vector<string> &node_map_role = relNodeMapRole.back() ;
                vector<string> &way_map_role = relWayMapRole.back() ;
                vector<string> &rel_map_role = relRelMapRole.back() ;

                while ( rd.read() )
                {
                    if ( rd.isStartElement("member") )
                    {
                        string type_ = rd.attribute("type") ;
                        string ref_ = rd.attribute("ref") ;
                        string role_ = rd.attribute("role") ;

                        if ( ref_.empty() || type_.empty() ) return false ;

                        if ( type_ == "node" )
                        {
                            node_map_item.push_back(ref_) ;
                            node_map_role.push_back(role_) ;
                        }
                        else if ( type_ == "way" )
                        {
                            way_map_item.push_back(ref_) ;
                            way_map_role.push_back(role_) ;
                        }
                        else if ( type_ == "relation" )
                        {
                            rel_map_item.push_back(ref_) ;
                            rel_map_role.push_back(role_) ;
                        }
                    }
                    else if ( rd.isStartElement("tag"))
                    {
                        string key_ = rd.attribute("k")  ;
                        string val_ = rd.attribute("v") ;

                        relation.tags[key_] = val_ ;
                    }
                    else if ( rd.isEndElement("relation" ) ) break ;

                }

                relMap[relation.id] = relations.size() ;
                relations.push_back(relation) ;

            }
        }

    }

    // establish feature dependencies

    for(int i=0 ; i<ways.size() ; i++ )
    {
        Way &way = ways[i] ;

        vector<string> &node_refs = wayNodeMap[i] ;

        for(int j=0 ; j<node_refs.size() ; j++ )
        {


            int idx = nodeMap[node_refs[j]] ;
            way.nodes.push_back(idx) ;

            nodes[idx].ways.push_back(i) ;
        }

    }

    for(int i=0 ; i<relations.size() ; i++ )
    {
        Relation &relation = relations[i] ;

        vector<string> &node_refs = relNodeMap[i] ;
        vector<string> &node_roles = relNodeMapRole[i] ;

        for(int j=0 ; j<node_refs.size() ; j++ )
        {
            std::map<string, int>::const_iterator it = nodeMap.find(node_refs[j]) ;

            if ( it != nodeMap.end() )
            {
                int idx = (*it).second ;
                relation.nodes.push_back(idx) ;
                relation.nodes_role.push_back(node_roles[j]) ;

                nodes[idx].relations.push_back(i) ;
            }
        }

    }

    for(int i=0 ; i<relations.size() ; i++ )
    {
        Relation &relation = relations[i] ;

        vector<string> &way_refs = relWayMap[i] ;
        vector<string> &way_roles = relWayMapRole[i] ;

        for(int j=0 ; j<way_refs.size() ; j++ )
        {
            std::map<string, int>::const_iterator it = wayMap.find(way_refs[j]) ;

            if ( it != wayMap.end() )
            {

                int idx = (*it).second ;
                relation.ways.push_back(idx) ;
                relation.ways_role.push_back(way_roles[j]) ;

                ways[idx].relations.push_back(i) ;
            }
        }

    }

    for(int i=0 ; i<relations.size() ; i++ )
    {
        Relation &relation = relations[i] ;

        vector<string> &rel_refs = relRelMap[i] ;
        vector<string> &rel_roles = relRelMapRole[i] ;

        for(int j=0 ; j<rel_refs.size() ; j++ )
        {
            std::map<string, int>::const_iterator it = relMap.find(rel_refs[j]) ;

            if ( it != relMap.end() )
            {
                int idx = relMap[rel_refs[j]] ;
                relation.children.push_back(idx) ;
                relation.children_role.push_back(rel_roles[j]) ;

                relations[idx].parents.push_back(i) ;
            }
        }

    }

    return true ;

}




bool Document::read(const string &fileName)
{
    if ( boost::ends_with(fileName, ".osm.gz") )
    {
        gzifstream strm(fileName.c_str()) ;

        return readXML(strm) ;
    }
    else if ( boost::ends_with(fileName, ".osm") )
    {
        ifstream strm(fileName.c_str()) ;

        return readXML(strm) ;
    }
    else if ( boost::ends_with(fileName, ".pbf") )
    {
        return readPBF(fileName) ;
    }

    return false ;
}

void Document::write(const string &fileName)
{
    if ( boost::ends_with(fileName, ".osm.gz") )
    {
        gzofstream strm(fileName.c_str()) ;

        return writeXML(strm) ;
    }
    else if ( boost::ends_with(fileName, ".osm") )
    {
        ofstream strm(fileName.c_str()) ;

        return writeXML(strm) ;
    }


}

void Document::writeXML(ostream &strm)
{

    strm << "<?xml version='1.0' encoding='UTF-8'?>\n" ;
    strm << "<osm version='0.6' generator='JOSM'>\n" ;

    for(int i=0 ; i<nodes.size() ; i++ )
    {
        const Node &node = nodes[i] ;

        strm << '\t' << "<node id='" << node.id << "' visible='true' lat='" << setprecision(12) << node.lat <<
            "' lon='" << setprecision(12) << node.lon  ;

        if ( node.tags.empty() ) strm <<  "' />\n" ;
        else
        {
            strm << "' >\n" ;

            vector<string> tags = node.tags.keys() ;

            for( int j=0 ; j<tags.size() ; j++ )
                strm << "\t\t" << "<tag k='" << tags[j] << "' v='" << node.tags.get(tags[j]) << "' />\n" ;

            strm << "\t</node>\n" ;
        }
    }

    for(int i=0 ; i<ways.size() ; i++ )
    {
        const Way &way = ways[i] ;

        strm << "\t<way id='" << way.id << "' action='modify' visible='true'>\n" ;

        vector<string> tags = way.tags.keys() ;

        for( int j=0 ; j<tags.size() ; j++ )
            strm << "\t\t" << "<tag k='" << tags[j] << "' v='" << way.tags.get(tags[j]) << "' />\n" ;

        for(int j=0 ; j<way.nodes.size() ; j++ )
        {
            strm << "\t\t<nd ref='" << nodes[way.nodes[j]].id << "'/>\n" ;
        }

        strm << "\t</way>\n" ;
    }

    for(int i=0 ; i<relations.size() ; i++ )
    {
        const Relation &relation = relations[i] ;

        strm << "\t<relation id='" << relation.id << "' action='modify' visible='true'>\n" ;

        vector<string> tags = relation.tags.keys() ;

        for( int j=0 ; j<tags.size() ; j++ )
            strm << "\t\t" << "<tag k='" << tags[j] << "' v='" << relation.tags.get(tags[j]) << "' />\n" ;

        for(int j=0 ; j<relation.nodes.size() ; j++ )
        {
            strm << "\t\t<member type='node' ref='" << nodes[relation.nodes[j]].id << "' role=''/>\n" ;
        }

        for(int j=0 ; j<relation.ways.size() ; j++ )
        {
            strm << "\t\t<member type='way' ref='" << ways[relation.ways[j]].id << "' role=''/>\n" ;
        }

        strm << "\t</relation>\n" ;
    }


    strm << "</osm>" ;

}



} // namespace OSM
