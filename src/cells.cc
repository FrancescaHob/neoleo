/*
 * $Id: cells.c,v 1.12 2000/08/10 21:02:50 danny Exp $
 *
 * Copyright � 1990, 1992, 1993 Free Software Foundation, Inc.
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

#define obstack_chunk_alloc ck_malloc
#define obstack_chunk_free free
#include "obstack.h"
#include "funcdef.h"
#include <iostream>
#include <string.h>
#include <string>


#include "global.h"

#include "byte-compile.h"
#include "cell.h"
#include "cmd.h"
#include "decompile.h"
#include "eval.h"
#include "errors.h"
#include "format.h"
#include "io-abstract.h"
#include "io-generic.h"
#include "io-term.h"
#include "sheet.h"
#include "logging.h"
#include "ref.h"
#include "spans.h"
#include "utils.h"

using std::cerr;
using std::cout;
using std::endl;

#define Float	x.c_n
#define String	x.c_s
#define Int	x.c_l
#define Value	x.c_i
#define Rng	x.c_r

static void log_debug_1(std::string msg)
{
	if constexpr(true)
		log_debug("DBG:cells.cc:" + msg);
}


#define ERROR(x)	\
 {			\
	p->Value=x;	\
	p->type=TYP_ERR;\
	return;		\
 }


value::value(void)
{
//	cout << "X";
}

value::~value(void)
{
//	cout << "Y";
}

cell::cell(void)
{
}

unsigned char * cell::get_cell_formula()
{ 
	return cell_formula; 
}

void cell::set_omnival(struct value* v)
{
	auto& v1 = v->x;
	switch(v->type) {
		case TYP_FLT:
			omnival = v1.c_n;
			break;	
		case TYP_INT:
			omnival = v1.c_i;
			break;
		case TYP_STR:
			omnival = std::string(v1.c_s);
			break;
		case TYP_ERR:
			omnival = Generic<Err>(v1.c_s);
			break;
		default:
			cerr << "TODO:set_omnival:type:" << v->type << endl;
			assert(false);
	}
	
}

/*
void cell::set_cell_str(char* newval)
{ 
	reset();
	x.c_s = newval;
}
*/

unsigned char * cell::set_cell_formula( unsigned char * newval)
{ 
	cell_formula = newval ;  
	return cell_formula; 
}

void cell::reset()
{
	if(cell_formula) free(cell_formula);
	cell_formula = 0;
}

cell::~cell()
{
	magic = 0x0DEFACED; // see TR06
	cell::reset();
	//cout <<"X";
}

/*
void 
cell::sInt(int newval)
{
	x.c_i = newval;
	set_type(TYP_INT);
}
*/

bool 
vacuous(cell* cp)
{
	return (cp == nullptr) || (cp->get_type() == TYP_NUL);
}

void set_cell_input(CELLREF r, CELLREF c, const std::string& new_input)
{
	log_debug_1("set_cell_input:r:" + std::to_string(r) + ":c:" + std::to_string(c) + ":new_input:" + new_input);
	new_value(r, c, new_input.c_str());
	/*
	if(new_input.size() == 0) return;
	CELL* cp = find_or_make_cell(r, c);
	assert(cp);
	const auto inpt =  new_input.c_str();
	auto bcode =  (unsigned char*) parse_and_compile(cp, inpt); // not auto-free'd
	cp->set_cell_formula(bcode);
	*/
}

std::string
get_cell_formula_at(int r, int c)
{

	std::string res = decomp_str(r, c);
	return res;
}


static int
cell_mc ( long row, long col, char *dowhat, struct value *p)
{
	struct func
	{
		const char *name;
		ValType typ;
	};

	static struct func cell_funs[] =
	{
		{"row", TYP_INT},		// 0
		{"column", TYP_INT},	// 1 
		{"width", TYP_INT},		// 2 
		{"lock", TYP_STR},		// 3 
		{"protection", TYP_STR},	// 4 
		{"justify", TYP_STR},	// 5 
		{"alignment", TYP_STR},	// 6 
		{"fmt", TYP_STR},		// 7 
		{"format", TYP_STR},	// 8 
		{"type", TYP_STR},		// 9
		{"formula", TYP_STR},	// 10
		{"value", TYP_NUL},		// 11 - TODO are we sure TYP_NUL is the correct type?
		{0, TYP_NUL}
	};

	struct func *func;
	struct func *f1;
	int n;

	n = strlen (dowhat) - 1;
	f1 = 0;
	for (func = cell_funs; func->name; func++)
		if (func->name[0] == dowhat[0]
				&& (n == 0 || !strincmp (&(func->name[1]), &dowhat[1], n)))
		{
			if (f1)
				return BAD_INPUT;
			f1 = func;
		}
	if (!f1)
		return BAD_INPUT;
	p->type = f1->typ;
	switch (f1 - cell_funs)
	{
		case 0:
			p->Int = row;
			break;
		case 1:
			p->Int = col;
			break;
		case 2:
			p->Int = get_width (col);
			break;
		case 3:
		case 4:
			{
				static char slock[] = "locked";
				static char sulock[] = "unlocked";
				CELL* cell_ptr = find_cell (row, col);
				p->String = ( (cell_ptr ? GET_LCK (cell_ptr) : default_lock)
						? slock
						: sulock);
			}
			break;
		case 5:
		case 6:
			{
				CELL* cell_ptr = find_cell (row, col);
				const char* str = jst_to_str (cell_ptr ?  GET_JST (cell_ptr) : default_jst); 
				p->String = (char *) obstack_alloc (&tmp_mem, strlen(str) + 1);
				strcpy(p->String, str);
			}
			break;
		case 7:
		case 8:
			p->String = cell_format_string(find_cell(row, col));
			break;      
		case 9:
			{
				CELL* cell_ptr = find_cell (row, col);
				const char* str = "null";
				if (cell_ptr)
					switch (GET_TYP (cell_ptr))
					{
						case TYP_FLT:
							str = "float";
							break;
						case TYP_INT:
							str= "integer";
							break;
						case TYP_STR:
							str = "string";
							break;
						case TYP_BOL:
							str = "boolean";
							break;
						case TYP_ERR:
							str = "error";
							break;
						default:
							str = "unknown";
					}
				p->String = (char *) obstack_alloc (&tmp_mem, strlen(str) + 1);
				strcpy(p->String, str);
			}
			break;
		case 10:
			{
				std::string formula = decomp_str(row, col);
				p->String = (char *) obstack_alloc (&tmp_mem, formula.size() + 1);
				strcpy(p->String, formula.c_str());
				break;
			}
		case 11:
			{
				CELL* cell_ptr = find_cell (row, col);
				if (cell_ptr)
				{
					p->type = GET_TYP (cell_ptr);
					p->x = cell_ptr->get_c_z();
				}
				else
					p->type = TYP_NUL;
			}
			break;
		default:
			return BAD_INPUT;
	}
	return 0;
}


static void
do_curcell (struct value *p)
{
  int tmp;

  tmp = cell_mc (curow, cucol, p->String, p);
  if (tmp)
    ERROR (tmp);
}

static void
do_my (value *p)
{
  int tmp;

  tmp = cell_mc (cur_row, cur_col, p->String, p);
  if (tmp)
    ERROR (tmp);
}

/* Note that the second argument may be *anything* including ERROR.  If it is
   error, we find the first occurence of that ERROR in the range */

static void
do_member (struct value *p)
{
  CELLREF crow;
  CELLREF ccol;
  int foundit;
  CELL *cell_ptr;

  find_cells_in_range (&(p->Rng));
  while ((cell_ptr = next_row_col_in_range (&crow, &ccol)))
    {
      if (GET_TYP (cell_ptr) != (p + 1)->type)
	continue;
      switch ((p + 1)->type)
	{
	case 0:
	  foundit = 1;
	  break;
	case TYP_FLT:
	  foundit = cell_ptr->gFlt() == (p + 1)->Float;
	  break;
	case TYP_INT:
	  foundit = cell_ptr->gInt() == (p + 1)->Int;
	  break;
	case TYP_STR:
	  foundit = !strcmp (cell_ptr->gString(), (p + 1)->String);
	  break;
	case TYP_BOL:
	  foundit = cell_ptr->gBol() == (p + 1)->Value;
	  break;
	case TYP_ERR:
	  foundit = cell_ptr->gErr() == (p + 1)->Value;
	  break;
	default:
	  foundit = 0;
#ifdef TEST
	  panic ("Unknown type (%d) in member", (p + 1)->type);
#endif
	}
      if (foundit)
	{
	  no_more_cells ();
	  p->Int = 1 + crow - p->Rng.lr + (ccol - p->Rng.lc) * (1 + p->Rng.hr - p->Rng.lr);
	  p->type = TYP_INT;
	  return;
	}
    }
  p->Int = 0L;
  p->type = TYP_INT;
}

static void
do_smember (
     struct value *p)
{
  CELLREF crow;
  CELLREF ccol;
  CELL *cell_ptr;
  char *string;

  string = (p + 1)->String;
  find_cells_in_range (&(p->Rng));
  while ((cell_ptr = next_row_col_in_range (&crow, &ccol)))
    {
      if (((GET_TYP (cell_ptr) == 0) && (string[0] == '\0'))
	  || (cell_ptr && (GET_TYP (cell_ptr) == TYP_STR)
	      && strstr (string, cell_ptr->gString())))
	{
	  no_more_cells ();
	  p->Int = 1 + (crow - p->Rng.lr)
	    + (ccol - p->Rng.lc) * (1 + (p->Rng.hr - p->Rng.lr));
	  p->type = TYP_INT;
	  return;
	}
    }
  p->Int = 0L;
  p->type = TYP_INT;
}

static void
do_members (
     struct value *p)
{
  CELLREF crow;
  CELLREF ccol;
  CELL *cell_ptr;
  char *string;

  string = (p + 1)->String;
  find_cells_in_range (&(p->Rng));
  while ((cell_ptr = next_row_col_in_range (&crow, &ccol)))
    {
      if (GET_TYP (cell_ptr) != TYP_STR)
	continue;
      if (strstr (cell_ptr->gString(), string))
	{
	  no_more_cells ();
	  p->Int = 1 + (crow - p->Rng.lr)
	    + (ccol - p->Rng.lc) * (1 + (p->Rng.hr - p->Rng.lr));
	  p->type = TYP_INT;
	  return;
	}
    }
  p->Int = 0L;
  p->type = TYP_INT;
}

static void
do_pmember (
     struct value *p)
{
  CELLREF crow;
  CELLREF ccol;
  CELL *cell_ptr;
  char *string;

  string = (p + 1)->String;
  find_cells_in_range (&(p->Rng));
  while ((cell_ptr = next_row_col_in_range (&crow, &ccol)))
    {
      if ((GET_TYP (cell_ptr) == 0 && string[0] == '\0')
	  || (cell_ptr && GET_TYP (cell_ptr) == TYP_STR && 
		  !strncmp (string, cell_ptr->gString(), strlen (cell_ptr->gString()))))
	{
	  no_more_cells ();
	  p->Int = 1 + (crow - p->Rng.lr)
	    + (ccol - p->Rng.lc) * (1 + (p->Rng.hr - p->Rng.lr));
	  p->type = TYP_INT;
	  return;
	}
    }
  p->Int = 0L;
  p->type = TYP_INT;
}

static void
do_memberp (
     struct value *p)
{
  CELLREF crow;
  CELLREF ccol;
  CELL *cell_ptr;
  int tmp;
  char *string;

  string = (p + 1)->String;
  find_cells_in_range (&(p->Rng));
  tmp = strlen (string);
  while ((cell_ptr = next_row_col_in_range (&crow, &ccol)))
    {
      if (GET_TYP (cell_ptr) != TYP_STR)
	continue;
      if (!strncmp (cell_ptr->gString(), string, tmp))
	{
	  no_more_cells ();
	  p->Int = 1 + (crow - p->Rng.lr)
	    + (ccol - p->Rng.lc) * (1 + (p->Rng.hr - p->Rng.lr));
	  p->type = TYP_INT;
	  return;
	}
    }
  p->Int = 0L;
  p->type = TYP_INT;
}

static void
do_hlookup (struct value *p)
{

  struct rng *rng = &((p)->Rng);
  num_t fltval = (p + 1)->Float;
  long offset = (p + 2)->Int;

  CELL *cell_ptr;
  num_t f;
  CELLREF col;
  CELLREF row;
  char *strptr;

  row = rng->lr;
  for (col = rng->lc; col <= rng->hc; col++)
    {
      if (!(cell_ptr = find_cell (row, col)))
	ERROR (NON_NUMBER);
      switch (GET_TYP (cell_ptr))
	{
	case TYP_FLT:
	  if (fltval < cell_ptr->gFlt())
	    goto out;
	  break;
	case TYP_INT:
	  if (fltval < cell_ptr->gInt())
	    goto out;
	  break;
	case TYP_STR:
	  strptr = cell_ptr->gString();
	  f = astof (&strptr);
	  if (!*strptr && fltval > f)
	    goto out;
	  else
	    ERROR (NON_NUMBER);
	case 0:
	case TYP_BOL:
	case TYP_ERR:
	default:
	  ERROR (NON_NUMBER);
	}
    }
out:
  if (col == rng->lc)
    ERROR (OUT_OF_RANGE);
  --col;
  row = rng->lr + offset;
  if (row > rng->hr)
    ERROR (OUT_OF_RANGE);
  cell_ptr = find_cell (row, col);
  if (!cell_ptr)
    {
      p->type = TYP_NUL;
      p->Int = 0;
    }
  else
    {
      p->type = GET_TYP (cell_ptr);
      p->x = cell_ptr->get_c_z();
    }
}

static void
do_vlookup (
     struct value *p)
{

  struct rng *rng = &((p)->Rng);
  num_t fltval = (p + 1)->Float;
  long offset = (p + 2)->Int;

  CELL *cell_ptr;
  //double f;
  num_t f;
  CELLREF col;
  CELLREF row;
  char *strptr;

  col = rng->lc;
  for (row = rng->lr; row <= rng->hr; row++)
    {
      if (!(cell_ptr = find_cell (row, col)))
	ERROR (NON_NUMBER);
      switch (GET_TYP (cell_ptr))
	{
	case TYP_FLT:
	  if (fltval < cell_ptr->gFlt())
	    goto out;
	  break;
	case TYP_INT:
	  if (fltval < cell_ptr->gInt())
	    goto out;
	  break;
	case TYP_STR:
	  strptr = cell_ptr->gString();
	  f = astof (&strptr);
	  if (!*strptr && fltval > f)
	    goto out;
	  else
	    ERROR (NON_NUMBER);
	case 0:
	case TYP_BOL:
	case TYP_ERR:
	default:
	  ERROR (NON_NUMBER);
	}
    }
out:
  if (row == rng->lr)
    ERROR (OUT_OF_RANGE);
  --row;
  col = rng->lc + offset;
  if (col > rng->hc)
    ERROR (OUT_OF_RANGE);

  cell_ptr = find_cell (row, col);
  if (!cell_ptr)
    {
      p->type = TYP_NUL;
      p->Int = 0;
    }
  else
    {
      p->type = GET_TYP (cell_ptr);
      p->x = cell_ptr->get_c_z();
    }
}

static void
do_vlookup_str (
     struct value *p)
{

  struct rng *rng = &((p)->Rng);
  char * key = (p + 1)->String;
  long offset = (p + 2)->Int;

  CELL *cell_ptr;
  CELLREF col;
  CELLREF row;

  col = rng->lc;
  for (row = rng->lr; row <= rng->hr; row++)
    {
      if (!(cell_ptr = find_cell (row, col)))
	ERROR (NON_NUMBER);
      switch (GET_TYP (cell_ptr))
	{
	case TYP_STR:
	  if (!strcmp (key, cell_ptr->gString()))
	    goto out;
	  break;
	case 0:
	case TYP_FLT:
	case TYP_INT:
	case TYP_BOL:
	case TYP_ERR:
	default:
	  ERROR (NON_NUMBER);
	}
    }
out:
  if (row > rng->hr)
    ERROR (OUT_OF_RANGE);
  col = rng->lc + offset;
  if (col > rng->hc)
    ERROR (OUT_OF_RANGE);

  cell_ptr = find_cell (row, col);
  if (!cell_ptr)
    {
      p->type = TYP_NUL;
      p->Int = 0;
    }
  else
    {
      p->type = GET_TYP (cell_ptr);
      p->x = cell_ptr->get_c_z();
    }
}


static void
do_cell (
     struct value *p)
{
  int tmp;

  tmp = cell_mc (p->Int, (p + 1)->Int, (p + 2)->String, p);
  if (tmp)
    ERROR (tmp);
}

/* While writing the macro loader, I found a need for a function
 * that could report back on whether or not a variable was defined.
 * The function below does that and more.  It accepts an integer
 * and two strings as arguments.  The integer specifies a corner;
 * 0 for NW, 1 for NE, 2 for SE and 3 for SW.  The first string
 * argument should specify a variable name.  The second should
 * specify one of the elements accepted by cell().  If the
 * variable has not been defined, this reports back its bare
 * name as a string.
 */

static void
do_varval (struct value *p)
{
  int tmp;
  int vr;
  int vc;
  struct var * v;

  v = find_var ((p+1)->String, strlen((p+1)->String));

  if (!v || v->var_flags == VAR_UNDEF) {
    p->type = TYP_STR;
    p->String = (p+1)->String;
  } else {
    switch(p->Int) {
      case 0:
        vr = v->v_rng.lr;
        vc = v->v_rng.lc;
        break;
      case 1:
        vr = v->v_rng.lr;
        vc = v->v_rng.hc;
        break;
      case 2:
        vr = v->v_rng.hr;
        vc = v->v_rng.hc;
        break;
      case 3:
        vr = v->v_rng.hr;
        vc = v->v_rng.lc;
        break;
      default:
        ERROR(OUT_OF_RANGE);
        return;
    }
    p->String = (p+2)->String;
    tmp = cell_mc (vr, vc, p->String, p);
    if (tmp)
      ERROR (tmp);
  }
}

#define S (char *)
#define T (void (*)())
static void
do_button(struct value *p)
{
	io_do_button(cur_row, cur_col, p->String, (p+1)->String);

	p->type = TYP_STR;
	p->String = (p+1)->String;
}

struct function cells_funs[] =
{
  {C_FN1 | C_T, X_A1, "S", T do_curcell, S "curcell"},
  {C_FN1 | C_T, X_A1, "S", T do_my, S "my"},
  {C_FN3 | C_T, X_A3, "IIS", T do_cell, S "cell"},
  {C_FN3 | C_T, X_A3, "ISS", T do_varval, S "varval"},

  {C_FN2, X_A2, "RA", T do_member, S "member"},
  {C_FN2, X_A2, "RS", T do_smember, S "smember"},
  {C_FN2, X_A2, "RS", T do_members, S "members"},
  {C_FN2, X_A2, "RS", T do_pmember, S "pmember"},
  {C_FN2, X_A2, "RS", T do_memberp, S "memberp"},

  {C_FN3, X_A3, "RFI", T do_hlookup, S "hlookup"},
  {C_FN3, X_A3, "RFI", T do_vlookup, S "vlookup"},
  {C_FN3, X_A3, "RSI", T do_vlookup_str, S "vlookup_str"},

  {C_FN2,	X_A2,	"SS",	T do_button,	S "button" },

  {0, 0, "", 0, 0},
};

int init_cells_function_count(void) 
{
        return sizeof(cells_funs) / sizeof(struct function) - 1;
}

void edit_cell(const char* input)
{
	new_value(curow, cucol, input);
	/*
	set_cell_input(curow, cucol, input);
	CELL* cp = find_cell(curow, cucol);
	assert(cp);
	update_cell(cp);
	*/
}


/*
void
edit_cell_at(CELLREF row, CELLREF col, std::string new_formula)
{
	edit_cell_at(row, col, new_formula.c_str());
}
void
edit_cell_at(CELLREF row, CELLREF col, const char* new_formula)
{
	char * fail;
	fail = new_value (row, col, new_formula);
	if (fail)
		io_error_msg (fail);
	else
		Global->modified = 1;
}
*/

/*
 * highest_row() and highest_col() should be expected to 
 * overallocate the number of rows and columns in the 
 * spreadsheet. The true number of rows and columns must 
 * be calculated
 *
 * TODO this is likely to be a very useful function
 *
 */

RC_t
ws_extent()
{
	int capacity_r = highest_row(), capacity_c = highest_col();
	//cell* m[capacity_r][capacity_c] = {};
	int size_r = 0, size_c = 0;
	for(int r=0; r<capacity_r; ++r)
		for(int c=0; c < capacity_c; ++c){
			cell * cp = find_cell(r+1, c+1);
			if(vacuous(cp)) continue;
			size_r = std::max(size_r, r+1);
			size_c = std::max(size_c, c+1);
			//m[r][c] = cp;
		}
	return RC_t{size_r, size_c};
}


//////////////////////////////////////////////////////////////////////
// for copying and pasting cells


static std::string m_copied_cell_formula = "";

void
copy_this_cell_formula()
{
	m_copied_cell_formula = decomp_str(curow, cucol);
}

void 
paste_this_cell_formula()
{	
	//edit_cell_at(curow, cucol, m_copied_cell_formula.c_str());
	set_cell_input(curow, cucol, m_copied_cell_formula.c_str());
}

//////////////////////////////////////////////////////////////////////
