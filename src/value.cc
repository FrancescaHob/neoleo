#include "value.h"

ValType value::get_type() { return type;}
void value::set_type(ValType t) { type = t;}

int value::gInt() { return x.c_i; };
void value::sInt(int newval) { type = TYP_INT; x.c_i = newval; };

long value::gLong() { assert(type == TYP_INT); return x.c_l; };
void value::sLong(long newval) { type = TYP_INT; x.c_l = newval; };
		
 char *value::gString() { assert(type == TYP_STR); return x.c_s; };
void value::sString(char* newval) { type = TYP_STR; x.c_s = newval;};

num_t value::gFlt() { return x.c_n ;};
void value::sFlt(num_t v) { type = TYP_FLT; x.c_n = v ;};

int value::gErr() { return x.c_err ;};
void value::sErr(int newval) { type = TYP_ERR ; x.c_err = newval ;};

int value::gBol() { return x.c_b ;};
void value::sBol(int newval) { type = TYP_BOL; x.c_b = newval; };