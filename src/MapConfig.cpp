#include "MapConfig.h"

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <iomanip>
#include <fstream>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"

using namespace std ;

static void split_line(const string &src, vector<string> &tokens) {

    using boost::tokenizer;
    using boost::escaped_list_separator;

    typedef tokenizer<escaped_list_separator<char> > stokenizer;

    stokenizer tok(src, escaped_list_separator<char>("\\", " ,\t",   "\"'"));

    for( stokenizer::iterator beg = tok.begin(); beg!=tok.end(); ++beg)
        if ( !beg->empty() ) tokens.push_back(*beg) ;
}

static void split_at_colon(const string &src, string &arg1, string &arg2, const string &def = string())
{
    vector<string> sub_tokens ;
    boost::split( sub_tokens, src , boost::is_any_of(" :"), boost::token_compress_on);

    arg1 = sub_tokens[0] ;
    if ( sub_tokens.size() == 2 ) arg2 = sub_tokens[1]  ;
    else arg2 = def ;
}

bool MapConfig::parse(const string &fileName)
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
            rapidjson::Document jsdoc ;
            jsdoc.Parse(line.c_str() + 7) ;

            if ( jsdoc.IsNull() || !jsdoc.HasMember("name") || !jsdoc.HasMember("type") ) {
                cerr << "Error parsing layer definition" << endl ;
            }

            // parse layer info

            Layer layer ;

            layer.name_ = jsdoc["name"].GetString() ;
            layer.geom_ = jsdoc["type"].GetString() ;

            rapidjson::Value &levels = jsdoc["levels"] ;

            if ( !levels.IsNull() ) {
                if ( levels.IsArray() ) {
                    for( uint i=0 ; i<levels.Size() ; i++ ) {
                        rapidjson::Value &level = levels[i] ;

                        ZoomInterval zi ;
                        if ( level.HasMember("min_zoom") ) zi.min_zoom_ = level["min_zoom"].GetInt() ;
                        if ( level.HasMember("max_zoom" ) )zi.max_zoom_ = level["max_zoom"].GetInt() ;
                        if ( level.HasMember("simplify_threshold") ) zi.simplify_threshold_ = level["max_zoom"].GetDouble() ;

                        layer.zr_.intervals_.push_back(zi) ;
                    }
                }
                else {
                    ZoomInterval zi ;
                    if ( levels.HasMember("min_zoom") ) zi.min_zoom_ = levels["min_zoom"].GetInt() ;
                    if ( levels.HasMember("max_zoom") ) zi.max_zoom_ = levels["max_zoom"].GetInt() ;
                    if ( levels.HasMember("simplify_threshold") ) zi.simplify_threshold_ = levels["max_zoom"].GetDouble() ;

                    layer.zr_.intervals_.push_back(zi) ;
                }
            }

            layers_.push_back(layer) ;
        }
        else if ( boost::starts_with(line, "@pragma") )
        {
            vector<string> tokens ;
            split_line(line, tokens) ;

            if ( tokens.size() < 3 )
            {
                cerr << "Error parsing pragma declaration in " << fileName << " not enough arguments (line: " << count << ")" ;
                return false ;
            }

            string key = tokens[1] ;
            string val = tokens[2] ;

            pragmas_.add(key, val) ;
        }

        else // rules
        {
            if ( layers_.empty() )
            {
                cerr << "Error parsing " << fileName << " :rule found before any layer definition (line: " << count << ")" ;
                return false ;
            }

            Layer &layer = layers_.back() ;

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

