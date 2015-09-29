#include "OsmRuleScanner.h"
#include "OsmRuleParser.h"
#include <boost/regex.hpp>
#include <boost/format.hpp>

#include <errno.h>
using namespace std;

namespace OSM {
namespace Rule {

Parser::Parser(std::istream &strm) :
    scanner(strm),
    parser(*this, loc),
    node(NULL),
    actions(0) {}

bool Parser::parse() {
//    parser.set_debug_level(5);

    loc.initialize() ;
    int res = parser.parse();

    if ( res == 0 ) return true ;
    else {
        delete node ;
        delete actions ;
        return false ;
    }
}

void Parser::error(const OSM::BisonParser::location_type &loc,
           const std::string& m)
{
    std::stringstream strm ;

    strm << m << " (at column: " ;
    if ( loc.begin.column == loc.end.column || loc.begin.column == loc.end.column-1 ) strm << loc.begin.column ;
    else strm << loc.begin.column <<  '-' << loc.end.column-1 ;
    strm <<  ")" ;

    errorString = strm.str() ;
}
//////////////////////////////////////////////////////////////////


std::string Context::id() const {

    assert(feat) ;
    return feat->id ;
}

bool Context::has_tag(const string &key) const
{
    assert(feat) ;

    return feat->tags.contains(key) ;
}

string Context::value(const string &key) const
{
    assert(feat) ;

    return feat->tags.get(key) ;
}


///////////////////////////////////////////////////////////////////

Literal::Literal(const std::string &val)
{
    char * e;
    double x = std::strtod(val.c_str(), &e);

    if (*e != 0 ||  errno != 0 )  {
        type_ = String ;
        string_val_ = val ;
    }
    else
    {
        type_ = Number ;
        number_val_ = x ;
    }

}

bool Literal::toBoolean() const {

    switch ( type_ ) {
        case Null: return false ;
        case Boolean: return boolean_val_ ;
        case Number: return number_val_ != 0.0 ;
        case String: return ( string_val_ != "0" && string_val_.empty()) ;
    }
}

double Literal::toNumber() const {

    switch ( type_ ) {
        case Null: return 0 ;
        case Boolean: return (double)boolean_val_ ;
        case Number: return number_val_ ;
        case String: return atof(string_val_.c_str()) ;
    }
}

string Literal::toString() const {
    switch ( type_ ) {
        case Null: return "" ;
        case Boolean: ( boolean_val_ ) ? "TRUE" : "FALSE" ;
        case Number: return str(boost::format("%f") % number_val_) ;
        case String: return string_val_ ;
    }
}


Literal BooleanOperator::eval(Context &ctx)
{
    switch ( op ) {
        case And:
            return ( children[0]->eval(ctx).toBoolean() && children[1]->eval(ctx).toBoolean() ) ;
        case Or:
            return ( children[0]->eval(ctx).toBoolean() || children[1]->eval(ctx).toBoolean() ) ;
        case Not:
           return !( children[0]->eval(ctx).toBoolean() ) ;
    }
}

Literal ComparisonPredicate::eval(Context &ctx)
{

   Literal lhs = children[0]->eval(ctx) ;
   Literal rhs = children[1]->eval(ctx) ;

   if ( lhs.isNull() || rhs.isNull() ) return false ;

    switch ( op ) {
        case Equal:
            {
                if ( lhs.type_ == Literal::String && lhs.type_ == Literal::String )
                    return Literal(lhs.string_val_ == rhs.string_val_) ;
                else return Literal(lhs.toNumber() == rhs.toNumber()) ;
            }
        case NotEqual:
            if ( lhs.type_ == Literal::String && lhs.type_ == Literal::String )
                return Literal(lhs.string_val_ != rhs.string_val_) ;
            else return Literal(lhs.toNumber() != rhs.toNumber()) ;
        case Less:
            return lhs.toNumber() < rhs.toNumber() ;
        case Greater:
            return lhs.toNumber() > rhs.toNumber() ;
        case LessOrEqual:
            return lhs.toNumber() <= rhs.toNumber() ;
        case GreaterOrEqual:
            return lhs.toNumber() >= rhs.toNumber() ;
    }

    return Literal() ;
}
static string globToRegex(const char *pat)
{
    // Convert pattern
    string rx = "(?i)^", be ;

    int i = 0;
    const char *pc = pat ;
    int clen = strlen(pat) ;
    bool inCharClass = false ;

    while (i < clen)
    {
        char c = pc[i++];

        switch (c)
        {
            case '*':
                rx += "[^\\\\/]*" ;
                break;
            case '?':
                rx += "[^\\\\/]" ;
                break;
            case '$':  //Regex special characters
            case '(':
            case ')':
            case '+':
            case '.':
            case '|':
                rx += '\\';
                rx += c;
                break;
            case '\\':
                if ( pc[i] == '*' ) rx += "\\*" ;
                else if ( pc[i] == '?' )  rx += "\\?" ;
                ++i ;
            break ;
            case '[':
            {
                if ( inCharClass )  rx += "\\[";
                else {
                    inCharClass = true ;
                    be += c ;
                }
                break ;
            }
            case ']':
            {
                if ( inCharClass ) {
                    inCharClass = false ;
                    rx += be ;
                    rx += c ;
                    rx += "{1}" ;
                    be.clear() ;
                }
                else rx += "\\]" ;

                break ;
            }
            case '%':
            {
                boost::regex rd("(0\\d)?d") ;
                boost::smatch what;

                if ( boost::regex_match(std::string(pat + i), what, rd,  boost::match_extra) )
                {

                    rx += "[[:digit:]]" ;

                    if ( what.size() == 2 )
                    {
                        rx +=  "{" ;
                        rx += what[1] ;
                        rx += "}" ;
                    }
                    else
                        rx += "+" ;

                    i += what.size() ;
                }
                else
                {
                    if ( inCharClass ) be += c ;
                    else rx += c;
                }
                break ;

            }
            default:
                if ( inCharClass ) be += c ;
                else rx += c;
        }
    }

    rx += "$" ;
    return rx ;
}

LikeTextPredicate::LikeTextPredicate(ExpressionNode *op, const std::string &pattern_, bool is_pos):
    ExpressionNode(op), isPos(is_pos)
{

    if ( !pattern_.empty() )
        pattern.set_expression(globToRegex(pattern_.c_str())) ;

}


Literal LikeTextPredicate::eval(Context &ctx)
{
    Literal op = children[0]->eval(ctx) ;

    return boost::regex_match(op.toString(), pattern) ;

}

ListPredicate::ListPredicate(const string &id, ExpressionNode *op, bool is_pos):
    id_(id), ExpressionNode(op), isPos(is_pos)
{
    Context ctx ;

    for(int i=0 ; i<children[0]->children.size() ; i++)
    {
        string lval = children[0]->children[i]->eval(ctx).toString() ;
        lvals_.push_back(lval) ;
    }

}


Literal ListPredicate::eval(Context &ctx)
{
    if ( !ctx.has_tag(id_) ) return Literal() ;

    string val = ctx.value(id_) ;

    for(int i=0 ; i<lvals_.size() ; i++)
        if ( val == lvals_[i] ) return isPos ;

    return !isPos ;
}


Literal IsTypePredicate::eval(Context &ctx)
{
    assert(ctx.feat) ;

    if ( keyword == "node")
    {
        return ctx.feat->type == Feature::NodeFeature ;
    }
    else if ( keyword == "way" )
    {
        return ctx.feat->type == Feature::WayFeature ;
    }
    else if ( keyword == "relation" )
    {
        return ctx.feat->type == Feature::RelationFeature ;
    }

}

Literal Attribute::eval(Context &ctx)
{
    if ( !ctx.has_tag(name_) ) return Literal() ;

    return ctx.value(name_) ;
}

Literal Function::eval(Context &ctx)
{
    return Literal() ;

}

Literal BinaryOperator::eval(Context &ctx)
{
    Literal op1 = children[0]->eval(ctx) ;
    Literal op2 = children[1]->eval(ctx) ;

    if ( op == '+' )
    {
        if ( op1.type_ == Literal::String && op2.type_ == Literal::String )
            return op1.toString() + op2.toString() ;
        else return op1.toNumber() + op2.toNumber() ;
    }
    else if ( op == '-' )
    {
        return op1.toNumber() + op2.toNumber() ;
    }
    else if ( op == '.' )
    {
        return op1.toString() + op2.toString() ;
    }
    else if ( op == '*' )
    {
        return op1.toNumber() + op2.toNumber() ;
    }
    else if ( op == '/' ) {
        if ( op2.toNumber() == 0.0 ) return Literal() ;
        else return op1.toNumber()/op2.toNumber() ;
    }

}

Literal ExistsPredicate::eval(Context &ctx)
{
    return ctx.has_tag(tag_) ;

}

} // namespace Filter
} // namespace OSM
