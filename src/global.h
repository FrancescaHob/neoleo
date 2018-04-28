#pragma once
/*
 * Copyright � 1990, 1992, 1993, 1999, 2000, 2001 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <map>
#include <string>

#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif /* RETSIGTYPE */

#ifndef VOIDSTAR
#define VOIDSTAR void*
#endif

/*
 * All kinds of other global stuff
 */
#define RCFILE ".oleorc"

/* The most important compile-time constant.  How many bits do we want to
   allocate for cell-references?  Useful values are 8 and 16 (at the moment)
   8 allows luser to access 255*255 cells (probably big enough)
   16 allows luser to access 65535*65535, which is more than will fit in
   the avaliable virtual memory on any 32-bit machine.
 */

inline constexpr auto BITS_PER_CELLREF = 16;

/* The location of a cell that can never be referenced */
#define NON_ROW		0
#define NON_COL		0

#define MIN_ROW		1
#define MIN_COL 	1

typedef unsigned short CELLREF;
#define CELLREF_MASK 0xFFFF
#define MAX_ROW 65535
#define MAX_COL 65535

/* Do *not* assume that 'name' is aligned!  It probably isn't */
#define GET_ROW(name)		((((name)[0])<<8)|(name)[1])
#define GET_COL(name)		((((name)[2])<<8)|(name)[3])
#define PUT_ROW(name,val)	((name)[0]=((val)>>8)),((name)[1]=val)
#define PUT_COL(name,val)	((name)[2]=((val)>>8)),((name)[3]=val)
#define EXP_ADD			sizeof(CELLREF)*2

/* Struct rng is used to describe a region of cells */
struct rng
{
  CELLREF lr, lc, hr, hc;
};

typedef struct rng range_t;

/* A ref_fm structure contains a list of all cells that reference some
 * value.  The value can be another cell or some global (such as the system
 * time).
 *
 * These structures are hash-consed and shared.  The hash-cons procedure
 * will re-use a particular structure if there is only one reference to it.
 */
  struct ref_array
    {
      CELLREF ref_row;
      CELLREF ref_col;
    };
struct ref_fm
{
  struct ref_fm *refs_next;
  unsigned short refs_refcnt;
  unsigned short refs_used;
  struct ref_array  fm_refs[1];
};

/* refs_to is a vector of locations in a formula where the
 * cell references other cells, ranges, or variables 
 */
struct ref_to
{
  unsigned short refs_refcnt;
  struct ref_to *refs_next;
  unsigned short refs_used;
  unsigned char to_refs[1];
};

/* These macros are used to extract/store ranges in compiled formulas. */
#define GET_RNG(name,putit)	bcopy((VOIDSTAR)(name),(VOIDSTAR)(putit),sizeof(struct rng))
#define PUT_RNG(name,putit)	bcopy((VOIDSTAR)(putit),(VOIDSTAR)(name),sizeof(struct rng))
#define EXP_ADD_RNG		sizeof(struct rng)

extern struct obstack tmp_mem;
//extern VOIDSTAR tmp_mem_start;
extern char * tmp_mem_start;

/* Defined in io-utils.c: */
#define ERR_MAX		17
extern char *ename[];
extern char tname[];
extern char fname[];
extern char iname[];
extern char mname[];
extern char nname[];

extern VOIDSTAR parse_hash;
extern double __plinf, __neinf;

/* These have two uses.  During parsing, these contain the 
 * base address of all relative references.  During evaluation,
 * these contain the address of the cell that is being updated.
 * 
 * When MY_CELL is set, these should be the address of that cell.
 * The address is used to recompute MY_CELL as the sparse array moves 
 * around.
 * 
 * Whey are all these distinct uses bound up in one pair of GLOBAL
 * variables?  GOOD QUESTION?  Why didn't the person who created the mess at
 * least toss in a COMMENT like the above to explain what was happening? 
 * ANOTHER GOOD QUESTION!
 */
extern CELLREF cur_row, cur_col;

extern int default_jst;
extern int default_fmt, default_prc;
extern int default_lock;

extern unsigned short current_cycle;
extern int ioerror;
extern int errno;
extern const char oleo_version_string[];

extern long astol (char **);
extern void panic (const char *, ...);

extern void add_ref (CELLREF, CELLREF);
extern void add_range_ref (struct rng *);
extern void add_timer_ref (int);
extern void add_ref_to (int);

extern void push_refs (struct ref_fm *);
extern void no_more_cells (void);

extern char *range_name (struct rng *);
extern char *cell_name (CELLREF, CELLREF);

extern unsigned char parse_cell_or_range (char **, struct rng *);

struct var; /* in case it hasn't been declared yet */
extern void for_all_vars (void (*)(char *, struct var *));

/*
 * Forward declarations required to get the global variable to compile
 */
struct	CursesGlobalType;
struct	MotifGlobalType;
struct	DatabaseGlobalType;

/*
 * This structure is a start at cleaning up global variables that are
 * around all over.
 */
struct OleoGlobal {
	int				valid = 1;
	std::string			FileName;	/* current_file in io-utils.c */
	int				modified;
	CELLREF				cur_row, cur_col;
/* User settable options */
	int				bkgrnd_recalc, auto_recalc, a0, topclear, sylk_a0;
/* This is how frequently the alarm should go off. */
	unsigned int			alarm_seconds;
/* This is whether the alarm should go off at all. */
	unsigned int			alarm_active;

/* Jump here on error.  This simply restarts the top 
 * level command loop.  User state should have been 
 * reset appropriately before the longjmp.
 *
 * mcarter 22-Mar-2017 I am trying to eliminate set/longjmps
 * in favour of exception handling in C++
 */
	//jmp_buf				error_exception;
/* From Window.c */
	int				scr_lines, scr_cols, user_input, user_status, input,
					status, input_rows, status_rows, label_rows, label_emcols;
	struct info_buffer		*current_info;
	int				info_rows, info_line, info_over;
	int				default_right_border, default_bottom_border;
	int				nwin;
	struct window			*cwin, *wins;
	int				win_id;

	struct MotifGlobalType		*MotifGlobal;
	struct CursesGlobalType		*CursesGlobal;
	struct DatabaseGlobalType	*DatabaseGlobal;
	struct PlotGlobalType		*PlotGlobal;

/* From lists.c */
	float				user_height_scale, user_width_scale,
					height_scale, width_scale;

	int				cell_font_point_size, block_on_getch;
	char				*io_x11_display_name;
	int				run_load_hooks, sneaky_linec;


	struct cf			*fp;
	struct list			*the_cols;
	//struct find			*w_find;
	//struct find			*h_find;

	int				display_formula_mode;
	struct find			*finds = 0;

/* Basic.c */
	int				auto_motion_direction;

/* List.c */
	char				sl_sep;

	int				display_opened;

/* pcl.c */
	int				need_formfeed;

/* print.c 
	float				zoom;
	struct PrintDriver		*CurrentPrintDriver;
	int				interline, TopBorderHeight, BottomBorderHeight,
					LeftBorderWidth, RightBorderWidth;
*/

/* window.c */
	struct mouse_event		*current_mouse, *free_mouse;
	int				mouse_id;

/* new stuff */
	char				*encoding = 0;
	int				return_from_error;
	int				had_error;

	char				*oldLocale;

	/* added by mcarter: */
	int argc;
	char** argv;
};

extern struct OleoGlobal *Global;

/*
 * Determine which flags are set to indicate META-key
 *	OLEO_NUM_KEYS sizes a definition in key.h
 */
#define	BACKSPACE	0x7f

#if 1
/*
 * Hopefully 8-bit clean version
 */
#ifndef CTRL_CHAR
#define CTRL_CHAR(x)		((x)&037)
#endif

#define	META_BIT	0x8000			/* Must be power of 2 */
#define	MASK_META_BIT	(META_BIT - 1)		/* used to be 0x7f */
#define	OLEO_NUM_KEYS	0x10000

#ifndef META
#define META(X)		((X)|0200)
#endif

#else
/* 8-bit diry version !! */

#ifndef CTRL_CHAR
#define CTRL_CHAR(x) (x&037)
#endif
#ifndef META
#define META(X) ((X)|0200)
#endif
#define	META_BIT	0x80			/* Must be power of 2 */
#define	MASK_META_BIT	(META_BIT - 1)		/* used to be 0x7f */

#define	OLEO_NUM_KEYS	256
#endif


static_assert(BITS_PER_CELLREF == 16);
