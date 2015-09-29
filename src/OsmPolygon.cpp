#include <OsmDocument.h>

using namespace std ;

namespace OSM {

// the function will create linear rings from relation members ignoring inner, outer roles
// the topology will be fixed by spatialite function ST_BuildArea

bool makePolygonsFromRelation(const Document &doc, const Relation &rel, Polygon &polygon)
{
    vector<string> roles ;
    vector<Ring> &rings = polygon.rings ;

    list<int> unassigned_ways ;

    // first create rings from closed ways

    for(int i=0 ; i<rel.ways.size() ; i++)
    {
        const Way &way = doc.ways[rel.ways[i]] ;
        const string &role = rel.ways_role[i] ;

        if (  way.nodes.front() == way.nodes.back() )
        {
            Ring r ;
            r.nodes.insert(r.nodes.end(), way.nodes.begin(), way.nodes.end()) ;
            rings.push_back(r) ;
            roles.push_back(role) ;
        }
        else unassigned_ways.push_back(i) ;
    }

    while ( !unassigned_ways.empty() )
    {
        Ring current ;

        int idx = unassigned_ways.front() ;

        const Way &way = doc.ways[rel.ways[idx]] ;

        string current_role = rel.ways_role[idx] ;

        current.nodes.insert(current.nodes.end(), way.nodes.begin(), way.nodes.end()) ;
        unassigned_ways.pop_front() ;

        // merge ways into circular rings

        bool finished ;

        do
        {
            finished = true ;

            list<int>::iterator itu = unassigned_ways.begin() ;

            while ( itu != unassigned_ways.end() )
            {
                int idx = *itu ;

                const Way &way = doc.ways[rel.ways[idx]] ;
                string role = rel.ways_role[idx] ;

                if ( role != current_role ) { ++itu ; continue ; }

                if ( current.nodes.front() == way.nodes.front() )
                {
                    vector<int>::const_iterator it = way.nodes.begin() ;
                    ++it ;
                    while ( it != way.nodes.end() )
                       current.nodes.push_front(*it++) ;
                    finished = false ;
                    itu = unassigned_ways.erase(itu) ;
                }
                else if ( current.nodes.back() == way.nodes.front() )
                {
                    current.nodes.insert(current.nodes.end(), way.nodes.begin()+1, way.nodes.end()) ;
                    finished = false ;
                    itu = unassigned_ways.erase(itu) ;
                }
                else if ( current.nodes.front() == way.nodes.back() )
                {
                    vector<int>::const_reverse_iterator it = way.nodes.rbegin() ;
                    ++it ;
                    while ( it != way.nodes.rend() )
                        current.nodes.push_front(*it++) ;
                    finished = false ;
                    itu = unassigned_ways.erase(itu) ;
                }
                else if ( current.nodes.back() == way.nodes.back() )
                {
                    current.nodes.insert(current.nodes.end(), way.nodes.rbegin()+1, way.nodes.rend()) ;
                    finished = false ;
                    itu = unassigned_ways.erase(itu) ;
                }
                else  ++itu ;
            }
        } while ( !finished ) ;


        // we should have a closed way otherwise something is wrong
        if ( current.nodes.front() != current.nodes.back() ) return false ;

        rings.push_back(current) ;
        roles.push_back(current_role) ;
        current.nodes.clear() ;

    }

    return true ;

}


bool makeWaysFromRelation(const Document &doc, const Relation &rel, std::vector<Way> &ways)
{
    vector<Ring> rings ;

    list<int> unassigned_ways ;

    for(int i=0 ; i<rel.ways.size() ; i++)
         unassigned_ways.push_back(i) ;

    while ( !unassigned_ways.empty() )
    {
        Ring current ;

        int idx = unassigned_ways.front() ;

        const Way &way = doc.ways[rel.ways[idx]] ;

        current.nodes.insert(current.nodes.end(), way.nodes.begin(), way.nodes.end()) ;
        unassigned_ways.pop_front() ;

        // merge ways into circular rings

        bool finished ;

        do
        {
            finished = true ;

            list<int>::iterator itu = unassigned_ways.begin() ;

            while ( itu != unassigned_ways.end() )
            {
                int idx = *itu ;

                const Way &way = doc.ways[rel.ways[idx]] ;

                if ( current.nodes.front() == way.nodes.front() )
                {
                    vector<int>::const_iterator it = way.nodes.begin() ;
                    ++it ;
                    while ( it != way.nodes.end() )
                       current.nodes.push_front(*it++) ;
                    finished = false ;
                    itu = unassigned_ways.erase(itu) ;
                }
                else if ( current.nodes.back() == way.nodes.front() )
                {
                    current.nodes.insert(current.nodes.end(), way.nodes.begin()+1, way.nodes.end()) ;
                    finished = false ;
                    itu = unassigned_ways.erase(itu) ;
                }
                else if ( current.nodes.front() == way.nodes.back() )
                {
                    vector<int>::const_reverse_iterator it = way.nodes.rbegin() ;
                    ++it ;
                    while ( it != way.nodes.rend() )
                        current.nodes.push_front(*it++) ;
                    finished = false ;
                    itu = unassigned_ways.erase(itu) ;
                }
                else if ( current.nodes.back() == way.nodes.back() )
                {
                    current.nodes.insert(current.nodes.end(), way.nodes.rbegin()+1, way.nodes.rend()) ;
                    finished = false ;
                    itu = unassigned_ways.erase(itu) ;
                }
                else  ++itu ;
            }
        } while ( !finished ) ;


        rings.push_back(current) ;
        current.nodes.clear() ;

    }

    for(int i=0 ; i< rings.size() ; i++ )
    {
        ways.push_back(Way()) ;
        Way &way = ways.back() ;

        way.nodes.insert(way.nodes.end(), rings[i].nodes.begin(), rings[i].nodes.end()) ;

        way.tags = rel.tags ;
    }

    return true ;

}

}
