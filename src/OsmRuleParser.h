#ifndef __OSM_RULE_PARSER_H__
#define __OSM_RULE_PARSER_H__

#include <OsmRuleScanner.h>
#include <OsmDocument.h>

#include <deque>
#include <string>
#include <vector>
#include <sstream>
#include <map>

#include <boost/regex.hpp>

namespace OSM {

class FlexScanner ;

namespace Rule {

class ExpressionNode ;
class Context ;
class Command ;

class Parser {

public:

    Parser(std::istream &strm)  ;

    bool parse() ;

    void error(const BisonParser::location_type &loc,  const std::string& m) ;

    FlexScanner scanner;
    BisonParser parser;
    ExpressionNode *node ;
    Command *actions ;

    std::string errorString ;
    OSM::BisonParser::location_type loc ;
 } ;


class Context {

public:

    Context(const OSM::Document *doc_ = 0): doc(doc_) {}

    void set(const OSM::Feature *f) {
        feat = f ;
    }

    const Feature *feat ;
    const OSM::Document *doc ;

    bool has_tag(const std::string &tag) const ;
    std::string value(const std::string &key) const ;
    std::string id() const ;
};


struct Literal {
public:
    enum Type { String, Number, Boolean, Null } ;

    Literal(): type_(Null) {}
    Literal(const std::string &val) ;
    Literal(const double val): type_(Number), number_val_(val) {}
    Literal(const bool val): type_(Boolean), boolean_val_(val) {}

    bool isNull() const { return type_ == Null ; }
    bool toBoolean() const ;
    std::string toString() const ;
    double toNumber() const ;


    Type type_ ;
    std::string string_val_ ;
    double number_val_ ;
    bool boolean_val_ ;
};

class ExpressionNode {

    public:

    ExpressionNode() {}

    virtual Literal eval(Context &ctx) { return false ; }

    ExpressionNode(ExpressionNode *child) { appendChild(child) ; }
    ExpressionNode(ExpressionNode *a1, ExpressionNode *a2) {
        appendChild(a1) ;
        appendChild(a2) ;
    }
    ExpressionNode(ExpressionNode *a1, ExpressionNode *a2, ExpressionNode *a3) {
        appendChild(a1) ;
        appendChild(a2) ;
        appendChild(a3) ;
    }

    virtual ~ExpressionNode() {
        std::deque<ExpressionNode *>::iterator it = children.begin() ;
        for( ; it != children.end() ; ++it ) delete (*it) ;
    }

    void appendChild(ExpressionNode *node) { children.push_back(node) ; }
    void prependChild(ExpressionNode *node) { children.push_front(node) ; }

    std::deque<ExpressionNode *> children ;


};


class Command {
public:
    enum Type { Set, Add, Store, Continue, Delete } ;

    Command(Type cmd, std::string ident = std::string(), ExpressionNode *val = 0): cmd_(cmd), tag_(ident), expression_(val), next_(0) {}

    ExpressionNode *expression_ ;
    std::string tag_ ;
    Type cmd_ ;
    Command *next_ ;
};

class CommandList {
public:
    CommandList(std::vector<Command> &commands) {}
};


class LiteralExpressionNode: public ExpressionNode {
public:
    enum Type { String, Number, Boolean } ;

    LiteralExpressionNode(const std::string &str): val_(str) {}
    LiteralExpressionNode(const double val): val_(val) {}
    LiteralExpressionNode(const bool val): val_(val) {}

    Literal eval(Context &ctx) { return val_ ; }

    Literal val_ ;
};

class Attribute: public ExpressionNode {
public:
    Attribute(const std::string name): name_(name) {}

    Literal eval(Context &ctx) ;

private:
    std::string name_ ;
};


class Function: public ExpressionNode {
public:
    Function(const std::string &name_): name(name_) {}
    Function(const std::string &name_, ExpressionNode *args): name(name_), ExpressionNode(args) {}

    Literal eval(Context &ctx) ;

private:
    std::string name ;

};


class BinaryOperator: public ExpressionNode {
public:
    BinaryOperator(int op_, ExpressionNode *op1, ExpressionNode *op2): op(op_), ExpressionNode(op1, op2) {}

    Literal eval(Context &ctx) ;

private:
    int op ;

};


class BooleanOperator: public ExpressionNode {
public:
    enum Type { And, Or, Not } ;

    BooleanOperator(Type op_, ExpressionNode *op1, ExpressionNode *op2): op(op_), ExpressionNode(op1, op2) {}

    Literal eval(Context &ctx) ;
private:
    Type op ;

};

class ComparisonPredicate: public ExpressionNode {
public:
    enum Type { Equal, NotEqual, Less, Greater, LessOrEqual, GreaterOrEqual } ;

    ComparisonPredicate(Type op_, ExpressionNode *lhs, ExpressionNode *rhs): op(op_), ExpressionNode(lhs, rhs) {}

    Literal eval(Context &ctx) ;

private:
    Type op ;

};

class LikeTextPredicate: public ExpressionNode {
public:

    LikeTextPredicate(ExpressionNode *op, const std::string &pattern_, bool is_pos) ;


     Literal eval(Context &ctx) ;
private:
    boost::regex pattern ;
    bool isPos ;

};

class ListPredicate: public ExpressionNode {
public:

    ListPredicate(const std::string &identifier, ExpressionNode *op, bool is_pos) ;


    Literal eval(Context &ctx) ;

private:
    std::string id_ ;
    std::vector<std::string> lvals_ ;
    bool isPos ;

};

class IsTypePredicate: public ExpressionNode {
public:

    IsTypePredicate(const std::string &keyword_):  keyword(keyword_) {}

    Literal eval(Context &ctx) ;

private:

    std::string keyword ;

};

class ExistsPredicate: public ExpressionNode {
public:

    ExistsPredicate(const std::string &tag):  tag_(tag) {}

    Literal eval(Context &ctx) ;

private:

    std::string tag_ ;

};


} // namespace Filter

} // namespace OSM

#endif
