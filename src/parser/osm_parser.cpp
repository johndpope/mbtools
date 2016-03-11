// A Bison parser, made by GNU Bison 3.0.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2013 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.


// First part of user declarations.

#line 37 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:399

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "osm_parser.hpp"

// User implementation prologue.

#line 51 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:407
// Unqualified %code blocks.
#line 29 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:408


#include <OsmRuleParser.h>

	// Prototype for the yylex function
static OSM::BisonParser::symbol_type yylex(OSM::Rule::Parser &driver, OSM::BisonParser::location_type &loc);



#line 63 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:408


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << std::endl;                  \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE(Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void>(0)
# define YY_STACK_PRINT()                static_cast<void>(0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyempty = true)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 6 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:474
namespace OSM {
#line 149 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:474

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  BisonParser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              // Fall through.
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  BisonParser::BisonParser (OSM::Rule::Parser &driver_yyarg, OSM::BisonParser::location_type &loc_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      driver (driver_yyarg),
      loc (loc_yyarg)
  {}

  BisonParser::~BisonParser ()
  {}


  /*---------------.
  | Symbol types.  |
  `---------------*/



  // by_state.
  inline
  BisonParser::by_state::by_state ()
    : state (empty)
  {}

  inline
  BisonParser::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  inline
  void
  BisonParser::by_state::move (by_state& that)
  {
    state = that.state;
    that.state = empty;
  }

  inline
  BisonParser::by_state::by_state (state_type s)
    : state (s)
  {}

  inline
  BisonParser::symbol_number_type
  BisonParser::by_state::type_get () const
  {
    return state == empty ? 0 : yystos_[state];
  }

  inline
  BisonParser::stack_symbol_type::stack_symbol_type ()
  {}


  inline
  BisonParser::stack_symbol_type::stack_symbol_type (state_type s, symbol_type& that)
    : super_type (s, that.location)
  {
      switch (that.type_get ())
    {
      case 41: // rule
      case 42: // action_block
      case 43: // command_list
      case 44: // command
        value.move< OSM::Rule::Command * > (that.value);
        break;

      case 45: // boolean_value_expression
      case 46: // boolean_term
      case 47: // boolean_factor
      case 48: // boolean_primary
      case 49: // predicate
      case 50: // comparison_predicate
      case 51: // like_text_predicate
      case 52: // exists_predicate
      case 53: // list_predicate
      case 54: // literal_list
      case 55: // expression
      case 56: // term
      case 57: // factor
      case 58: // function
      case 59: // function_argument_list
      case 60: // function_argument
      case 61: // literal
      case 62: // general_literal
      case 63: // boolean_literal
      case 64: // numeric_literal
      case 65: // attribute
        value.move< OSM::Rule::ExpressionNode * > (that.value);
        break;

      case 37: // "number"
        value.move< double > (that.value);
        break;

      case 36: // "identifier"
      case 38: // "string literal"
        value.move< std::string > (that.value);
        break;

      default:
        break;
    }

    // that is emptied.
    that.type = empty;
  }

  inline
  BisonParser::stack_symbol_type&
  BisonParser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
      switch (that.type_get ())
    {
      case 41: // rule
      case 42: // action_block
      case 43: // command_list
      case 44: // command
        value.copy< OSM::Rule::Command * > (that.value);
        break;

      case 45: // boolean_value_expression
      case 46: // boolean_term
      case 47: // boolean_factor
      case 48: // boolean_primary
      case 49: // predicate
      case 50: // comparison_predicate
      case 51: // like_text_predicate
      case 52: // exists_predicate
      case 53: // list_predicate
      case 54: // literal_list
      case 55: // expression
      case 56: // term
      case 57: // factor
      case 58: // function
      case 59: // function_argument_list
      case 60: // function_argument
      case 61: // literal
      case 62: // general_literal
      case 63: // boolean_literal
      case 64: // numeric_literal
      case 65: // attribute
        value.copy< OSM::Rule::ExpressionNode * > (that.value);
        break;

      case 37: // "number"
        value.copy< double > (that.value);
        break;

      case 36: // "identifier"
      case 38: // "string literal"
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }


  template <typename Base>
  inline
  void
  BisonParser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  BisonParser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    YYUSE (yytype);
    yyo << ')';
  }
#endif

  inline
  void
  BisonParser::yypush_ (const char* m, state_type s, symbol_type& sym)
  {
    stack_symbol_type t (s, sym);
    yypush_ (m, t);
  }

  inline
  void
  BisonParser::yypush_ (const char* m, stack_symbol_type& s)
  {
    if (m)
      YY_SYMBOL_PRINT (m, s);
    yystack_.push (s);
  }

  inline
  void
  BisonParser::yypop_ (unsigned int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  BisonParser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  BisonParser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  BisonParser::debug_level_type
  BisonParser::debug_level () const
  {
    return yydebug_;
  }

  void
  BisonParser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  inline BisonParser::state_type
  BisonParser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  inline bool
  BisonParser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  BisonParser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  BisonParser::parse ()
  {
    /// Whether yyla contains a lookahead.
    bool yyempty = true;

    // State.
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    // Accept?
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    // Backup.
  yybackup:

    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyempty)
      {
        YYCDEBUG << "Reading a token: ";
        try
          {
            symbol_type yylookahead (yylex (driver, loc));
            yyla.move (yylookahead);
          }
        catch (const syntax_error& yyexc)
          {
            error (yyexc);
            goto yyerrlab1;
          }
        yyempty = false;
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Discard the token being shifted.
    yyempty = true;

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
        switch (yyr1_[yyn])
    {
      case 41: // rule
      case 42: // action_block
      case 43: // command_list
      case 44: // command
        yylhs.value.build< OSM::Rule::Command * > ();
        break;

      case 45: // boolean_value_expression
      case 46: // boolean_term
      case 47: // boolean_factor
      case 48: // boolean_primary
      case 49: // predicate
      case 50: // comparison_predicate
      case 51: // like_text_predicate
      case 52: // exists_predicate
      case 53: // list_predicate
      case 54: // literal_list
      case 55: // expression
      case 56: // term
      case 57: // factor
      case 58: // function
      case 59: // function_argument_list
      case 60: // function_argument
      case 61: // literal
      case 62: // general_literal
      case 63: // boolean_literal
      case 64: // numeric_literal
      case 65: // attribute
        yylhs.value.build< OSM::Rule::ExpressionNode * > ();
        break;

      case 37: // "number"
        yylhs.value.build< double > ();
        break;

      case 36: // "identifier"
      case 38: // "string literal"
        yylhs.value.build< std::string > ();
        break;

      default:
        break;
    }


      // Compute the default @$.
      {
        slice<stack_symbol_type, stack_type> slice (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, slice, yylen);
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
      try
        {
          switch (yyn)
            {
  case 2:
#line 101 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { driver.node = yystack_[1].value.as< OSM::Rule::ExpressionNode * > () ; driver.actions = yystack_[0].value.as< OSM::Rule::Command * > () ; }
#line 638 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 3:
#line 106 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::Command * > () = yystack_[1].value.as< OSM::Rule::Command * > () ; }
#line 644 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 4:
#line 110 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::Command * > () = yystack_[0].value.as< OSM::Rule::Command * > () ;  }
#line 650 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 5:
#line 111 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::Command * > () = yystack_[1].value.as< OSM::Rule::Command * > () ; }
#line 656 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 6:
#line 112 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::Command * > () = yystack_[2].value.as< OSM::Rule::Command * > () ; yystack_[2].value.as< OSM::Rule::Command * > ()->next_ = yystack_[0].value.as< OSM::Rule::Command * > () ;}
#line 662 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 7:
#line 116 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::Command * > () = new OSM::Rule::Command( OSM::Rule::Command::Add, yystack_[2].value.as< std::string > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 668 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 8:
#line 117 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::Command * > () = new OSM::Rule::Command( OSM::Rule::Command::Set, yystack_[2].value.as< std::string > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ;}
#line 674 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 9:
#line 118 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::Command * > () = new OSM::Rule::Command( OSM::Rule::Command::Delete, yystack_[0].value.as< std::string > ()) ; }
#line 680 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 10:
#line 119 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::Command * > () = new OSM::Rule::Command( OSM::Rule::Command::Store, yystack_[1].value.as< std::string > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 686 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 11:
#line 120 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::Command * > () = new OSM::Rule::Command( OSM::Rule::Command::Continue) ;}
#line 692 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 12:
#line 129 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 698 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 13:
#line 130 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::BooleanOperator( OSM::Rule::BooleanOperator::Or, yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 704 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 14:
#line 134 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 710 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 15:
#line 135 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::BooleanOperator( OSM::Rule::BooleanOperator::And, yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 716 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 16:
#line 139 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 722 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 17:
#line 140 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::BooleanOperator( OSM::Rule::BooleanOperator::Not, yystack_[0].value.as< OSM::Rule::ExpressionNode * > (), NULL) ; }
#line 728 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 18:
#line 144 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 734 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 19:
#line 145 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[1].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 740 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 20:
#line 149 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 746 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 21:
#line 150 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 752 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 22:
#line 151 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 758 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 23:
#line 152 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 764 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 24:
#line 157 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::Equal, yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ) ; }
#line 770 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 25:
#line 158 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::NotEqual, yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ) ; }
#line 776 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 26:
#line 159 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::Less, yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ) ; }
#line 782 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 27:
#line 160 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::Greater, yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ) ; }
#line 788 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 28:
#line 161 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::LessOrEqual, yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ) ; }
#line 794 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 29:
#line 162 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::ComparisonPredicate( OSM::Rule::ComparisonPredicate::GreaterOrEqual, yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ) ; }
#line 800 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 30:
#line 166 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::LikeTextPredicate(yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< std::string > (), true) ; }
#line 806 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 31:
#line 167 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::LikeTextPredicate(yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< std::string > (), false) ; }
#line 812 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 32:
#line 171 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::ExistsPredicate(yystack_[0].value.as< std::string > ()) ; }
#line 818 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 33:
#line 175 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::ListPredicate(yystack_[4].value.as< std::string > (), yystack_[1].value.as< OSM::Rule::ExpressionNode * > (), true) ; }
#line 824 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 34:
#line 176 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::ListPredicate(yystack_[5].value.as< std::string > (), yystack_[1].value.as< OSM::Rule::ExpressionNode * > (), false) ; }
#line 830 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 35:
#line 180 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::ExpressionNode() ; yylhs.value.as< OSM::Rule::ExpressionNode * > ()->appendChild(yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 836 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 36:
#line 181 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()->prependChild(yystack_[2].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 842 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 37:
#line 185 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 848 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 38:
#line 186 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::BinaryOperator('+',yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 854 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 39:
#line 187 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::BinaryOperator('.',yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 860 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 40:
#line 188 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::BinaryOperator('-', yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 866 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 41:
#line 192 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 872 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 42:
#line 193 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::BinaryOperator('*', yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 878 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 43:
#line 194 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::BinaryOperator('/', yystack_[2].value.as< OSM::Rule::ExpressionNode * > (), yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 884 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 44:
#line 198 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 890 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 45:
#line 199 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 896 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 46:
#line 200 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 902 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 47:
#line 201 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[1].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 908 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 48:
#line 205 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::Function(yystack_[2].value.as< std::string > ()) ; }
#line 914 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 49:
#line 206 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    {
			yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::Function(yystack_[3].value.as< std::string > (), yystack_[1].value.as< OSM::Rule::ExpressionNode * > ()) ;
		 }
#line 922 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 50:
#line 212 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::ExpressionNode() ; yylhs.value.as< OSM::Rule::ExpressionNode * > ()->appendChild(yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 928 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 51:
#line 213 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; yystack_[0].value.as< OSM::Rule::ExpressionNode * > ()->prependChild(yystack_[2].value.as< OSM::Rule::ExpressionNode * > ()) ; }
#line 934 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 52:
#line 217 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 940 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 53:
#line 221 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 946 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 54:
#line 222 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 952 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 55:
#line 226 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::LiteralExpressionNode(yystack_[0].value.as< std::string > ()) ; }
#line 958 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 56:
#line 227 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = yystack_[0].value.as< OSM::Rule::ExpressionNode * > () ; }
#line 964 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 57:
#line 232 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::LiteralExpressionNode(true) ; }
#line 970 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 58:
#line 233 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    { yylhs.value.as< OSM::Rule::ExpressionNode * > () =  new OSM::Rule::LiteralExpressionNode(false) ; }
#line 976 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 59:
#line 237 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    {
		yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::LiteralExpressionNode((double)yystack_[0].value.as< double > ()) ;
	}
#line 984 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;

  case 60:
#line 243 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:847
    {
		yylhs.value.as< OSM::Rule::ExpressionNode * > () = new OSM::Rule::Attribute(yystack_[0].value.as< std::string > ()) ;
	}
#line 992 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
    break;


#line 996 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:847
            default:
              break;
            }
        }
      catch (const syntax_error& yyexc)
        {
          error (yyexc);
          YYERROR;
        }
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, yylhs);
    }
    goto yynewstate;

  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state,
                                           yyempty ? yyempty_ : yyla.type_get ()));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyempty)
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyempty = true;
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    yyerror_range[1].location = yystack_[yylen - 1].location;
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", error_token);
    }
    goto yynewstate;

    // Accept.
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    // Abort.
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyempty)
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (!yyempty)
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  void
  BisonParser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what());
  }

  // Generate an error message.
  std::string
  BisonParser::yysyntax_error_ (state_type yystate, symbol_number_type yytoken) const
  {
    std::string yyres;
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yytoken) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char BisonParser::yypact_ninf_ = -77;

  const signed char BisonParser::yytable_ninf_ = -1;

  const signed char
  BisonParser::yypact_[] =
  {
      -2,   -11,   -77,   -77,   -25,    -2,    -1,   -77,   -77,    18,
       7,    42,    19,   -77,   -77,   -77,   -77,   -77,   -77,    92,
      61,    -3,   -77,   -77,   -77,   -77,   -77,   -77,   -77,   -77,
      32,    55,    21,    35,    37,   -77,    62,   -77,    -2,    -2,
      22,    31,    38,    38,    38,    38,    38,    38,    38,    38,
      38,    38,    38,   -77,   -77,    49,    38,   -77,    59,   -77,
      60,    58,    -8,    48,    50,    51,    52,   -77,    69,    79,
     -77,   -77,   -77,   -77,   -77,   -77,   -77,   -77,   -77,   -77,
     -77,   -77,   -77,   -77,   -77,    -8,    85,   -77,    38,    86,
      87,    76,    78,   -77,    38,   -77,    62,    90,   -77,   -77,
      -8,    38,    38,   -77,   -77,   -77,   -77,   -77,   -77
  };

  const unsigned char
  BisonParser::yydefact_[] =
  {
       0,     0,    57,    58,     0,     0,    60,    59,    55,     0,
       0,    12,    14,    16,    18,    20,    21,    22,    23,     0,
      37,    41,    44,    45,    54,    56,    53,    46,    17,    32,
       0,     0,     0,     0,     0,     1,     0,     2,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    19,    47,     0,     0,    48,    60,    52,
       0,    50,     0,     0,     0,     0,     0,    11,     0,     4,
      13,    15,    30,    31,    24,    25,    26,    27,    28,    29,
      38,    40,    39,    42,    43,     0,     0,    49,     0,     0,
      35,     0,     0,     9,     0,     3,     5,     0,    51,    33,
       0,     0,     0,    10,     6,    34,    36,     7,     8
  };

  const signed char
  BisonParser::yypgoto_[] =
  {
     -77,   -77,   -77,    17,   -77,    10,    75,   -77,   114,   -77,
     -77,   -77,   -77,   -77,   -76,    -5,   -30,   -77,   -77,    28,
     -77,   -54,   -77,   -77,   -77,   -77
  };

  const signed char
  BisonParser::yydefgoto_[] =
  {
      -1,     9,    37,    68,    69,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    89,    19,    20,    21,    22,    60,
      61,    23,    24,    25,    26,    27
  };

  const unsigned char
  BisonParser::yytable_[] =
  {
      31,     1,    32,     2,     3,     4,     2,     3,    90,    97,
       5,    29,     2,     3,     4,    30,    51,    52,    35,     5,
      33,    83,    84,    39,   106,     6,     7,     8,    59,     7,
       8,    90,    36,    34,     6,     7,     8,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    90,    38,    70,     2,
       3,    86,     2,     3,    53,    55,    56,    57,    62,    56,
      72,    40,    41,    42,    43,    44,    45,    46,    47,    73,
      85,    58,     7,     8,    58,     7,     8,    54,    48,    49,
      33,    88,    87,    59,    91,    50,    92,    93,    94,   103,
      63,    64,    65,    66,    67,    95,   107,   108,    40,    41,
      42,    43,    44,    45,    46,    47,    96,    54,    99,   101,
     100,   102,   105,   104,    71,    28,    98
  };

  const unsigned char
  BisonParser::yycheck_[] =
  {
       5,     3,     3,    14,    15,    16,    14,    15,    62,    85,
      21,    36,    14,    15,    16,     5,    19,    20,     0,    21,
      21,    51,    52,     4,   100,    36,    37,    38,    33,    37,
      38,    85,    25,    34,    36,    37,    38,    42,    43,    44,
      45,    46,    47,    48,    49,    50,   100,     5,    38,    14,
      15,    56,    14,    15,    22,    34,    21,    22,    21,    21,
      38,     6,     7,     8,     9,    10,    11,    12,    13,    38,
      21,    36,    37,    38,    36,    37,    38,    22,    17,    18,
      21,    23,    22,    88,    36,    24,    36,    36,    36,    94,
      28,    29,    30,    31,    32,    26,   101,   102,     6,     7,
       8,     9,    10,    11,    12,    13,    27,    22,    22,    33,
      23,    33,    22,    96,    39,     1,    88
  };

  const unsigned char
  BisonParser::yystos_[] =
  {
       0,     3,    14,    15,    16,    21,    36,    37,    38,    41,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    55,
      56,    57,    58,    61,    62,    63,    64,    65,    48,    36,
      45,    55,     3,    21,    34,     0,    25,    42,     5,     4,
       6,     7,     8,     9,    10,    11,    12,    13,    17,    18,
      24,    19,    20,    22,    22,    34,    21,    22,    36,    55,
      59,    60,    21,    28,    29,    30,    31,    32,    43,    44,
      45,    46,    38,    38,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    56,    56,    21,    55,    22,    23,    54,
      61,    36,    36,    36,    36,    26,    27,    54,    59,    22,
      23,    33,    33,    55,    43,    22,    54,    55,    55
  };

  const unsigned char
  BisonParser::yyr1_[] =
  {
       0,    40,    41,    42,    43,    43,    43,    44,    44,    44,
      44,    44,    45,    45,    46,    46,    47,    47,    48,    48,
      49,    49,    49,    49,    50,    50,    50,    50,    50,    50,
      51,    51,    52,    53,    53,    54,    54,    55,    55,    55,
      55,    56,    56,    56,    57,    57,    57,    57,    58,    58,
      59,    59,    60,    61,    61,    62,    62,    63,    63,    64,
      65
  };

  const unsigned char
  BisonParser::yyr2_[] =
  {
       0,     2,     2,     3,     1,     2,     3,     4,     4,     2,
       3,     1,     1,     3,     1,     3,     1,     2,     1,     3,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     5,     6,     1,     3,     1,     3,     3,
       3,     1,     3,     3,     1,     1,     1,     3,     3,     4,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const BisonParser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\"!\"", "\"&&\"", "\"||\"",
  "\"~\"", "\"!~\"", "\"==\"", "\"!=\"", "\"<\"", "\">\"", "\"<=\"",
  "\">=\"", "\"true\"", "\"false\"", "\"^\"", "\"+\"", "\"-\"", "\"*\"",
  "\"/\"", "\"(\"", "\")\"", "\",\"", "\".\"", "\"{\"", "\"}\"", "\";\"",
  "\"add tag\"", "\"set tag\"", "\"delete tag\"", "\"store\"",
  "\"continue\"", "\"=\"", "\"in\"", "\"not\"", "\"identifier\"",
  "\"number\"", "\"string literal\"", "UMINUS", "$accept", "rule",
  "action_block", "command_list", "command", "boolean_value_expression",
  "boolean_term", "boolean_factor", "boolean_primary", "predicate",
  "comparison_predicate", "like_text_predicate", "exists_predicate",
  "list_predicate", "literal_list", "expression", "term", "factor",
  "function", "function_argument_list", "function_argument", "literal",
  "general_literal", "boolean_literal", "numeric_literal", "attribute", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned char
  BisonParser::yyrline_[] =
  {
       0,   101,   101,   106,   110,   111,   112,   116,   117,   118,
     119,   120,   129,   130,   134,   135,   139,   140,   144,   145,
     149,   150,   151,   152,   157,   158,   159,   160,   161,   162,
     166,   167,   171,   175,   176,   180,   181,   185,   186,   187,
     188,   192,   193,   194,   198,   199,   200,   201,   205,   206,
     212,   213,   217,   221,   222,   226,   227,   232,   233,   237,
     243
  };

  // Print the state stack on the debug stream.
  void
  BisonParser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  BisonParser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):" << std::endl;
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


#line 6 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:1155
} // OSM
#line 1444 "/home/malasiot/source/mbtools/src/parser/osm_parser.cpp" // lalr1.cc:1155
#line 250 "/home/malasiot/source/mbtools/src/osm.y" // lalr1.cc:1156

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
