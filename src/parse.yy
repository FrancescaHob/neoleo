%{
/*
 * $Id: parse.y,v 1.11 2001/02/04 00:03:48 pw Exp $
 *
 * Copyright (C) 1990, 1992, 1993, 1999 Free Software Foundation, Inc.
 *
 * This file is part of Oleo, the GNU Spreadsheet.
 * 
 * Oleo is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * Oleo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Oleo; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */


%}

%define api.prefix {yyreg}

%right '?' ':'
/* %left '|' */
%left '&'
%nonassoc '=' NE
%nonassoc '<' LE '>' GE
%left '+' '-'
%left '*' '/' '%'
%right '^'
%left NEG '!'

%token	L_CELL L_RANGE
%token	L_VAR

%token	L_CONST
%token	L_FN0	L_FN1	L_FN2	L_FN3	L_FN4	L_FNN
%token	L_FN1R	L_FN2R	L_FN3R	L_FN4R	L_FNNR

%token	L_LE	L_NE	L_GE

%{
#include <iostream>
#include <map>
#include <stdexcept>
#include <string_view>
using namespace std::literals;

#include <ctype.h>
#include <string.h>

#include "global.h"
#include "errors.h"
#include "mem.h"
#include "node.h"
#include "eval.h"
#include "ref.h"
#include "parse_parse.h"

using std::cout;

int yyreglex ();
void yyregerror (std::string_view s);

YYREGSTYPE  parse_return;
YYREGSTYPE make_list (YYREGSTYPE, YYREGSTYPE);

void check_parser_called_correctly();

%}
%%
top: line { check_parser_called_correctly(); }

line:	exp
		{ parse_return=$1; }
	| error {
		if(!parse_error)
			parse_error=PARSE_ERR;
		parse_return=0; }
	;

exp:	  L_CONST
	| cell
	| L_FN0 '(' ')' {
		$$=$1; }
	| L_FN1 '(' exp ')' {
		($1)->n_x.v_subs[0]=$3;
		($1)->n_x.v_subs[1]=(struct node *)0;
		$$=$1; }
	| L_FN2 '(' exp ',' exp ')' {
		($1)->n_x.v_subs[0]=$3;
		($1)->n_x.v_subs[1]=$5;
		$$=$1; }
	| L_FN3 '(' exp ',' exp ',' exp ')' {
		($1)->n_x.v_subs[0]=make_list($3,$5);
 		($1)->n_x.v_subs[1]=$7;
 		$$=$1;}
	| L_FN4 '(' exp ',' exp ',' exp ',' exp ')' {
		($1)->n_x.v_subs[0]=make_list($3,$5);
 		($1)->n_x.v_subs[1]=make_list($7,$9);
 		$$=$1;}
	| L_FNN '(' exp_list ')' {
		($1)->n_x.v_subs[0]=(struct node *)0;
		($1)->n_x.v_subs[1]=$3;
		$$=$1; }
	| L_FN1R '(' L_RANGE ')' {
		$1->n_x.v_subs[0]=$3;
		$$=$1; }
	| L_FN1R '(' L_VAR ')' {
		$1->n_x.v_subs[0]=$3;
		$$=$1; }

	| L_FN2R '(' L_RANGE ',' exp ')' {
		$1->n_x.v_subs[0]=$3;
		$1->n_x.v_subs[1]=$5;
		$$=$1; }
	| L_FN2R '(' L_VAR ',' exp ')' {
		$1->n_x.v_subs[0]=$3;
		$1->n_x.v_subs[1]=$5;
		$$=$1; }

	| L_FN3R '(' L_RANGE ',' exp ',' exp ')' {
		($1)->n_x.v_subs[0]=make_list($3,$5);
 		($1)->n_x.v_subs[1]=$7;
 		$$=$1;}
	| L_FN3R '(' L_VAR ',' exp ',' exp ')' {
		($1)->n_x.v_subs[0]=make_list($3,$5);
 		($1)->n_x.v_subs[1]=$7;
 		$$=$1;}

	| L_FN4R '(' L_RANGE ',' exp ',' exp ',' exp ')' {
		($1)->n_x.v_subs[0]=make_list($3,$5);
 		($1)->n_x.v_subs[1]=make_list($7,$9);
 		$$=$1;}
	| L_FN4R '(' L_VAR ',' exp ',' exp ',' exp ')' {
		($1)->n_x.v_subs[0]=make_list($3,$5);
 		($1)->n_x.v_subs[1]=make_list($7,$9);
 		$$=$1;}

	| L_FNNR '(' range_exp_list ')' {
		($1)->n_x.v_subs[0]=(struct node *)0;
		($1)->n_x.v_subs[1]=$3;
		$$=$1; }
	| exp '?' exp ':' exp {
		$2->comp_value=IF;
		$2->n_x.v_subs[0]=$4;
		$2->n_x.v_subs[1]=$5;
		$4->n_x.v_subs[0]=$1;
		$4->n_x.v_subs[1]=$3;
		$$=$2; }
	/* | exp '|' exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; } */
	| exp '&' exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp '<' exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp LE exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp '=' exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp NE exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp '>' exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp GE exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp '+' exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp '-' exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp '*' exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp '/' exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp '%' exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| exp '^' exp {
		$2->n_x.v_subs[0]=$1;
		$2->n_x.v_subs[1]=$3;
		$$ = $2; }
	| '+' exp {
		if($2->comp_value==CONST_FLT) {
			$2->n_x.v_float= ($2->n_x.v_float);
			$$=$2;
		} else if($2->comp_value==CONST_INT) {
			$2->n_x.v_int= ($2->n_x.v_int);
			$$=$2;
		} else {
			$$ = $2;
		} }
	| '-' exp %prec NEG {
		if($2->comp_value==CONST_FLT) {
			$2->n_x.v_float= -($2->n_x.v_float);
			$$=$2;
		} else if($2->comp_value==CONST_INT) {
			$2->n_x.v_int= -($2->n_x.v_int);
			$$=$2;
		} else {
			$1->comp_value = NEGATE;
			$1->n_x.v_subs[0]=$2;
			$1->n_x.v_subs[1]=(struct node *)0;
			$$ = $1;
		} }
	| '!' exp {
		$1->n_x.v_subs[0]=$2;
		$1->n_x.v_subs[1]=(struct node *)0;
		$$ = $1; }
	| '(' exp ')'
		{ $$ = $2; }
	| '(' exp error {
		if(!parse_error)
			parse_error=NO_CLOSE;
		}
	/* | exp ')' error {
		if(!parse_error)
			parse_error=NO_OPEN;
		} */
	| '(' error {
		if(!parse_error)
			parse_error=NO_CLOSE;
		}
	;


exp_list: exp
 		{ $$ = make_list($1, 0); }
	| exp_list ',' exp
		{ $$ = make_list($3, $1); }
	;

range_exp: L_RANGE
	| exp
	;

range_exp_list: range_exp
		{ $$=make_list($1, 0); }
	|   range_exp_list ',' range_exp
		{ $$=make_list($3,$1); }
	;

cell:	L_CELL
		{ $$=$1; }
	| L_VAR
	;
%%

void yyerror (std::string_view s)
{
	if(!parse_error)
		parse_error=PARSE_ERR;
}

static mem_c* _mem_ptr = nullptr;

void* alloc_parsing_memory(size_t nbytes)
{
	void* ptr = malloc(nbytes);
	_mem_ptr->add_ptr(ptr);
	return ptr;
}

YYREGSTYPE
make_list (YYSTYPE car, YYSTYPE cdr)
{
	YYSTYPE ret;

	ret=(YYSTYPE)alloc_parsing_memory(sizeof(*ret));
	ret->comp_value = 0;
	ret->n_x.v_subs[0]=car;
	ret->n_x.v_subs[1]=cdr;
	return ret;
}


//extern struct node *yylval;

unsigned char parse_cell_or_range (char **,struct rng *);


/* create a sentinel to check that yyparse() is only called
 * via yyparse_parse()
 */

static bool allow_yyparse = false;	
void check_parser_called_correctly()
{
	if(!allow_yyparse)
		throw std::logic_error("parse.yy:yyparse() called erroneously. Call yyparse_parse() instead");
}

int yyparse_parse(const std::string& input, mem_c& yymem)
{
	allow_yyparse = true;	
	instr = (char*) malloc(input.size()+1);
	yymem.add_ptr(instr);
	assert(instr);
	strcpy(instr, input.c_str());
	_mem_ptr = &yymem;
	_mem_ptr->auto_release();
	int ret = yyparse();
	allow_yyparse = false;	
	return ret;
}

int yyparse_parse(const std::string& input)
{
	mem_c yymem;
	return yyparse_parse(input, yymem);
}
void FormulaParser::clear()
{
	parser_mem.release_all();
	m_root = nullptr;
	parse_error = 0;
	parse_return =0;
}

bool  FormulaParser::parse(const std::string& text)
{

	clear();

	bool ok = true;

	// is the text purely whitespace?
	bool trivial = std::all_of(text.cbegin(), text.cend(), [](int c) { return std::isspace(c) !=0;});
	//cout << "FormulaParser::parse:trivial:"  << trivial ;
	if(trivial) return ok;

	bool ok1 = yyparse_parse(text, parser_mem) == 0;
	bool ok2 = parse_error == 0;
	ok  = ok1 && ok2; 
	m_root = parse_return;
	//cout << ":parse_error:" << parse_error << ":ok1:" << ok1 << ":ok2:" << ok2 << ":ok:" << ok << ":root:" << (void*) root() << "\n";
	//cout << ":ok:" << ok << "\n";
	return ok;
}

node* FormulaParser::root() { return m_root; }

FormulaParser::~FormulaParser()
{
	clear();
}
