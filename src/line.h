#pragma once
/*
 * Copyright � 1992, 1993 Free Software Foundation, Inc.
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
 * along with Oleo; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <string>

#define LINE_MIN 28

typedef struct line
{
	int alloc = 0;	
	char *buf = nullptr;
	~line();
} line_t;

//#define init_line(L)  bzero((L), sizeof (struct line))

void set_line (struct line *line, const char *string);
void set_line_a (line_t& a_line, const char *string);
extern void setn_line (struct line *line, const char *string, int n);
extern void catn_line (struct line *line, const char *string, int n);
void catn_line (struct line *line, const std::string& str);
extern void sprint_line (struct line *line, const char * fmt, ...);
void splicen_line(struct line * line, const char * str, int n, int pos);
extern void edit_line (struct line * line, int begin, int len);
extern void free_line (struct line * line);
