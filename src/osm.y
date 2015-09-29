%require "3.0.2"
%skeleton "lalr1.cc"

%defines
%locations
%define api.namespace {OSM}
%define parser_class_name {BisonParser}
%define api.token.constructor
%define api.value.type variant
%define api.token.prefix {TOK_}

%param { OSM::Rule::Parser &driver }
%param { OSM::BisonParser::location_type &loc }

%define parse.trace
%define parse.error verbose

%code requires {
	namespace OSM {
		namespace Rule {
			class Parser ;
			class ExpressionNode ;
			class Command ;
		}
	}

}

%code {

#include <OsmRuleParser.h>

	// Prototype for the yylex function
static OSM::BisonParser::symbol_type yylex(OSM::Rule::Parser &driver, OSM::BisonParser::location_type &loc);


}


	/* literal keyword tokens */
%token NOT "!"
%token AND "&&"
%token OR "||"
%token MATCHES "~"
%token  NOT_MATCHES "!~"
%token EQUAL "=="
%token NOT_EQUAL "!="
%token LESS_THAN "<"
%token GREATER_THAN ">"
%token LESS_THAN_OR_EQUAL "<="
%token GREATER_THAN_OR_EQUAL ">="
%token TRUEX "true"
%token FALSEX "false"
%token EXISTS "^"
%token PLUS "+"
%token MINUS "-"
%token STAR "*"
%token DIV "/"
%token LPAR "("
%token RPAR ")"
%token COMMA ","
%token DOT "."
%token LEFT_BRACE "{"
%token RIGHT_BRACE "}"
%token COLON ";"
%token ADD_CMD "add tag"
%token SET_CMD "set tag"
%token DELETE_CMD "delete tag"
%token STORE_CMD "store"
%token CONTINUE_CMD "continue"
%token ASSIGN "="
%token IN "in"
%token NOT "not"

%token <std::string> IDENTIFIER "identifier";
%token <double> NUMBER "number";
%token <std::string> STRING "string literal";
%token END  0  "end of file";

%type <OSM::Rule::ExpressionNode *> boolean_value_expression boolean_term boolean_factor boolean_primary predicate comparison_predicate like_text_predicate exists_predicate
%type <OSM::Rule::ExpressionNode *> expression term factor numeric_literal boolean_literal general_literal literal function function_argument function_argument_list attribute
%type <OSM::Rule::ExpressionNode *> program complex_expression list_predicate literal_list
%type <OSM::Rule::Command *> action_block command_list command rule

/*%destructor { delete $$; } STRING IDENTIFIER*/

/*operators */

%left OR
%left AND
%left LESS_THAN GREATER_THAN LESS_THAN_OR_EQUAL GREATER_THAN_OR_EQUAL EQUAL NOT_EQUAL
%left PLUS MINUS DOT
%left STAR DIV
%nonassoc UMINUS EXISTS

%start rule

%%


rule: boolean_value_expression action_block { driver.node = $1 ; driver.actions = $2 ; }
;


action_block:
	LEFT_BRACE command_list RIGHT_BRACE { $$ = $2 ; }
	;

command_list:
	command { $$ = $1 ;  }
	| command COLON { $$ = $1 ; }
	| command COLON command_list { $$ = $1 ; $1->next_ = $3 ;}
	;

command:
		ADD_CMD IDENTIFIER ASSIGN expression { $$ = new OSM::Rule::Command( OSM::Rule::Command::Add, $2, $4) ; }
	|	SET_CMD IDENTIFIER ASSIGN expression { $$ = new OSM::Rule::Command( OSM::Rule::Command::Set, $2, $4) ;}
	|   DELETE_CMD IDENTIFIER { $$ = new OSM::Rule::Command( OSM::Rule::Command::Delete, $2) ; }
	|   STORE_CMD IDENTIFIER expression { $$ = new OSM::Rule::Command( OSM::Rule::Command::Store, $2, $3) ; }
	|   CONTINUE_CMD { $$ = new OSM::Rule::Command( OSM::Rule::Command::Continue) ;}
	;

complex_expression:
	boolean_value_expression	{ $$ = $1 ; }
	| expression { $$ = $1 ; }
	;

boolean_value_expression:
	boolean_term								{ $$ = $1 ; }
	| boolean_term OR boolean_value_expression	{ $$ = new OSM::Rule::BooleanOperator( OSM::Rule::BooleanOperator::Or, $1, $3) ; }
	;

boolean_term:
	boolean_factor						{ $$ = $1 ; }
	| boolean_factor AND boolean_term	{ $$ = new OSM::Rule::BooleanOperator( OSM::Rule::BooleanOperator::And, $1, $3) ; }
	;

boolean_factor:
	boolean_primary			{ $$ = $1 ; }
	| NOT boolean_primary	{ $$ = new OSM::Rule::BooleanOperator( OSM::Rule::BooleanOperator::Not, $2, NULL) ; }
	;

boolean_primary:
	predicate							{ $$ = $1 ; }
	| LPAR boolean_value_expression RPAR	{ $$ = $2 ; }
	;

predicate:
	comparison_predicate	{ $$ = $1 ; }
	| like_text_predicate	{ $$ = $1 ; }
	| exists_predicate	    { $$ = $1 ; }
	| list_predicate        { $$ = $1 ; }
	;


comparison_predicate:
	 expression EQUAL expression					{ $$ = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::Equal, $1, $3 ) ; }
	| expression NOT_EQUAL expression				{ $$ = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::NotEqual, $1, $3 ) ; }
	| expression LESS_THAN expression				{ $$ = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::Less, $1, $3 ) ; }
	| expression GREATER_THAN expression			{ $$ = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::Greater, $1, $3 ) ; }
	| expression LESS_THAN_OR_EQUAL expression		{ $$ = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::LessOrEqual, $1, $3 ) ; }
	| expression GREATER_THAN_OR_EQUAL expression	{ $$ = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::GreaterOrEqual, $1, $3 ) ; }
	 ;

like_text_predicate:
	expression MATCHES STRING							{ $$ = new OSM::Rule::LikeTextPredicate($1, $3, true) ; }
	| expression NOT_MATCHES STRING					{ $$ = new OSM::Rule::LikeTextPredicate($1, $3, false) ; }
	;

exists_predicate:
	EXISTS IDENTIFIER								{ $$ = new OSM::Rule::ExistsPredicate($2) ; }
	;

list_predicate:
		IDENTIFIER IN LPAR literal_list RPAR	{ $$ = new OSM::Rule::ListPredicate($1, $4, true) ; }
	  | IDENTIFIER NOT IN LPAR literal_list RPAR	{ $$ = new OSM::Rule::ListPredicate($1, $5, false) ; }
		;

literal_list:
				  literal		{ $$ = new OSM::Rule::ExpressionNode() ; $$->appendChild($1) ; }
				| literal COMMA literal_list { $$ = $3 ; $3->prependChild($1) ; }
				;

expression:
		  term					{ $$ = $1 ; }
		| term PLUS expression	{ $$ = new OSM::Rule::BinaryOperator('+',$1, $3) ; }
		| term DOT expression	{ $$ = new OSM::Rule::BinaryOperator('.',$1, $3) ; }
		| term MINUS expression	{ $$ = new OSM::Rule::BinaryOperator('-', $1, $3) ; }
	  ;

term:
		factor					{ $$ = $1 ; }
		| factor STAR term		{ $$ = new OSM::Rule::BinaryOperator('*', $1, $3) ; }
		| factor DIV term		{ $$ = new OSM::Rule::BinaryOperator('/', $1, $3) ; }
		;

factor:
		  function				{ $$ = $1 ; }
		| literal				{ $$ = $1 ; }
		| attribute			{ $$ = $1 ; }
		| LPAR expression RPAR	{ $$ = $2 ; }
		;

function:
		IDENTIFIER LPAR RPAR		{ $$ = new OSM::Rule::Function($1) ; }
		 | IDENTIFIER LPAR function_argument_list RPAR {
			$$ = new OSM::Rule::Function($1, $3) ;
		 }
	;

function_argument_list:
		  function_argument		{ $$ = new OSM::Rule::ExpressionNode() ; $$->appendChild($1) ; }
		| function_argument COMMA function_argument_list { $$ = $3 ; $3->prependChild($1) ; }
		;

function_argument :
		expression			{ $$ = $1 ; }
		;

literal:
		numeric_literal		{ $$ = $1 ; }
		| general_literal	{ $$ = $1 ; }
		;

general_literal :
		STRING				{ $$ = new OSM::Rule::LiteralExpressionNode($1) ; }
		| boolean_literal	{ $$ = $1 ; }

		;

boolean_literal:
		TRUEX	{ $$ = new OSM::Rule::LiteralExpressionNode(true) ; }
		| FALSEX { $$ =  new OSM::Rule::LiteralExpressionNode(false) ; }
	;

numeric_literal:
	NUMBER {
		$$ = new OSM::Rule::LiteralExpressionNode((double)$1) ;
	}
	;

attribute:
	IDENTIFIER {
		$$ = new OSM::Rule::Attribute($1) ;
	}
	;



%%
#include <OsmRuleScanner.h>

// We have to implement the error function
void OSM::BisonParser::error(const OSM::BisonParser::location_type &loc, const std::string &msg) {

	driver.error(loc, msg) ;
}

// Now that we have the Parser declared, we can declare the Scanner and implement
// the yylex function

static OSM::BisonParser::symbol_type yylex(OSM::Rule::Parser &driver, OSM::BisonParser::location_type &loc) {
	return  driver.scanner.lex(&loc);
}

static int yydebug_=1 ;
