%{
/*
 * AT&T lex can't handle this lexer due to lex bugs.  It works with flex
 * 2.3.7, pclex 2.0.5, and MKS lex 3.1a.
 */
#include <OsmRuleScanner.h>
#include <OsmRuleParser.h>

#include <osm_parser.hpp>
#include <string.h>



%}

%option debug noyywrap
%option yyclass="FlexScanner"
%option c++
%option prefix="osm"
%option debug
%option case-insensitive


%{
# define YY_USER_ACTION  yylloc->columns (yyleng);
%}
%%

%{
   yylloc->step();
%}

"AND"                 { return OSM::BisonParser::make_AND(*yylloc); }
"&&"                { return OSM::BisonParser::make_AND(*yylloc) ; }
"OR"                  { return OSM::BisonParser::make_OR(*yylloc); }
"||"                { return OSM::BisonParser::make_OR(*yylloc); }
"IN"				{ return OSM::BisonParser::make_IN(*yylloc); }
"NOT"				{ return OSM::BisonParser::make_NOT(*yylloc); }

        /* punctuation */

"="    { return OSM::BisonParser::make_ASSIGN(*yylloc) ; }
"=="    { return OSM::BisonParser::make_EQUAL(*yylloc) ; }
"<>"    { return OSM::BisonParser::make_NOT_EQUAL(*yylloc) ; }
"!="    { return OSM::BisonParser::make_NOT_EQUAL(*yylloc) ; }
"<"     { return OSM::BisonParser::make_LESS_THAN(*yylloc) ; }
">"     { return OSM::BisonParser::make_GREATER_THAN(*yylloc) ; }
"<="	{ return OSM::BisonParser::make_LESS_THAN_OR_EQUAL(*yylloc) ; }
">="	{ return OSM::BisonParser::make_GREATER_THAN_OR_EQUAL(*yylloc) ; }
"~"     { return OSM::BisonParser::make_MATCHES(*yylloc) ; }
"!~"    { return OSM::BisonParser::make_NOT_MATCHES(*yylloc) ; }
"!"     { return OSM::BisonParser::make_NOT(*yylloc) ; }
"^"		{ return OSM::BisonParser::make_EXISTS(*yylloc) ; }
"("     { return OSM::BisonParser::make_LPAR(*yylloc) ; }
")"     { return OSM::BisonParser::make_RPAR(*yylloc) ; }
"+"     { return OSM::BisonParser::make_PLUS(*yylloc) ; }
"-"     { return OSM::BisonParser::make_MINUS(*yylloc) ; }
"*"     { return OSM::BisonParser::make_STAR(*yylloc) ; }
"/"     { return OSM::BisonParser::make_DIV(*yylloc) ; }
","     { return OSM::BisonParser::make_COMMA(*yylloc) ; }
"."     { return OSM::BisonParser::make_DOT(*yylloc) ; }
"{"		{ return OSM::BisonParser::make_LEFT_BRACE(*yylloc) ; }
"}"		{ return OSM::BisonParser::make_RIGHT_BRACE(*yylloc) ; }
";"		{ return OSM::BisonParser::make_COLON(*yylloc) ; }

add([[:space:]]+(tag))? { return OSM::BisonParser::make_ADD_CMD(*yylloc) ; }
set([[:space:]]+(tag))? { return OSM::BisonParser::make_SET_CMD(*yylloc) ; }
delete[[:space:]]+(tag)? { return OSM::BisonParser::make_DELETE_CMD(*yylloc) ; }
"store" { return OSM::BisonParser::make_STORE_CMD(*yylloc) ; }
"continue" { return OSM::BisonParser::make_CONTINUE_CMD(*yylloc) ; }

[a-zA-Z]:[a-zA-Z0-9_]+ |
[a-zA-Z][a-zA-Z0-9_]* {
        return OSM::BisonParser::make_IDENTIFIER(yytext, *yylloc);
}

[0-9]+	|
[0-9]+"."[0-9]* |
"."[0-9]*	|
[0-9]+[eE][+-]?[0-9]+	|
[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
"."[0-9]*[eE][+-]?[0-9]+	{
    return OSM::BisonParser::make_NUMBER(atof(yytext), *yylloc) ;
}

['\"]([^\\\']|\\.)*['\"] {
    return OSM::BisonParser::make_STRING(std::string(yytext+1, yyleng-2), *yylloc) ;
}

[\n]+   yylloc->lines (yyleng); yylloc->step ();

[ \t]+   yylloc->step ();

<<EOF>>    return OSM::BisonParser::make_END(*yylloc);
%%


