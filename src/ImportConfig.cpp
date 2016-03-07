#include "ImportConfig.h"

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <iomanip>
#include <fstream>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"

#include "GeomHelpers.h"

using namespace std ;

static void split_line(const string &src, vector<string> &tokens) {

    using boost::tokenizer;
    using boost::escaped_list_separator;

    typedef tokenizer<escaped_list_separator<char> > stokenizer;

    stokenizer tok(src, escaped_list_separator<char>("\\", " ,\t",   "\"'"));

    for( stokenizer::iterator beg = tok.begin(); beg!=tok.end(); ++beg)
        if ( !beg->empty() ) tokens.push_back(*beg) ;
}

bool ImportConfig::parse(const string &fileName)
{
    ifstream strm(fileName.c_str()) ;
    if ( !strm ) {
        cerr << "Cannot open config file: " << fileName << endl ;
        return false ;
    }

    int count = 0 ;
    while ( !strm.eof() )
    {
        string line ;
        getline(strm, line) ;

        ++count ;

        if ( line.empty() || line.at(0)== '#' ) continue ;
        else if ( boost::starts_with(line, "@layer") )
        {
            vector<string> tokens ;
            split_line(line, tokens) ;

            if ( tokens.size() < 3 ) {
                cerr << "error parsing layer definition" << endl ;
                return false ;
            }
            // parse layer info

            ImportLayer layer ;

            layer.name_ = tokens[1] ;
            layer.geom_ = tokens[2] ;

            layers_.push_back(layer) ;
        }
        else // rules
        {
            if ( layers_.empty() )
            {
                cerr << "Error parsing " << fileName << " :rule found before any layer definition (line: " << count << ")" ;
                return false ;
            }

            ImportLayer &layer = layers_.back() ;

            istringstream sstrm(line) ;
            OSM::Rule::Parser parser(sstrm) ;

            if ( ! parser.parse() )
            {
                cerr << "Error parsing " << fileName << " : error in rule (line: " << count << ")" << endl ;
                cerr << parser.errorString << endl ;

                return false ;
            }

            Rule rule ;

            rule.condition_ = parser.node ;
            rule.actions_ = parser.actions ;

            layer.rules_.push_back(rule) ;
        }

    }

    return true ;
}

