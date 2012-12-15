/* This software is distributed under the GNU Lesser General Public License */
//==========================================================================
//
//   gml_parser.h 
//
//==========================================================================
// $Id: gml_parser.h,v 1.7 2000/01/05 16:32:36 raitner Exp $

// This code has been adapted from the GML Parser hosted at:
// http://www.fim.uni-passau.de/en/fim/faculty/chairs/theoretische-informatik/projects.html

#ifndef NE_EXT_GML_H
#define NE_EXT_GML_H

/*
 * start-size of buffers for reading strings. If too small it will be enlarged
 * dynamically
 */
#define INITIAL_SIZE 5120

namespace netevo {

typedef enum {
	GML_KEY, GML_INT, GML_DOUBLE, GML_STRING, GML_L_BRACKET, 
	GML_R_BRACKET, GML_END, GML_LIST, GML_ERROR
} GML_value; 


typedef enum {
	GML_UNEXPECTED, GML_SYNTAX, GML_PREMATURE_EOF, GML_TOO_MANY_DIGITS,
	GML_OPEN_BRACKET, GML_TOO_MANY_BRACKETS, GML_OK
} GML_error_value;


struct GML_error {
	GML_error_value err_num;
	int line;
	int column;
};


union GML_tok_val {
	long integer;
	double floating;
	char* string;
	struct GML_error err;
};


struct GML_token { 
	GML_value kind;
	union GML_tok_val value;
};

/*
 * global variables
 */
extern unsigned int GML_line;
extern unsigned int GML_column;

/*
 * if you are interested in the position where an error occured it is a good
 * idea to set GML_line and GML_column back. 
 * This is what GML_init does.
 */
void GML_init ();

/*
 * returns the next token in file. If an error occured it will be stored in 
 * GML_token.
 */
struct GML_token GML_scanner (FILE*);

union GML_pair_val {
	long integer;
	double floating;
	char* string;
	struct GML_pair* list;
};

struct GML_pair {
	char* key;
	GML_value kind;
	union GML_pair_val value;
	struct GML_pair* next;
};

struct GML_list_elem {
	char* key;
	struct GML_list_elem* next;
};

struct GML_stat {
	struct GML_error err;
	struct GML_list_elem* key_list;
};

/*
 * returns list of KEY - VALUE pairs. Errors and a pointer to a list
 * of key-names are returned in GML_stat. Previous information contained
 * in GML_stat, i.e. the key_list, will be *lost*. 
 */
struct GML_pair* GML_parser (FILE*, struct GML_stat*, int);

/*
 * free memory used in a list of GML_pair
 */
void GML_free_list (struct GML_pair*, struct GML_list_elem*);


/*
 * debugging 
 */
void GML_print_list (struct GML_pair*, int);
	
} // netevo namespace

#endif // NE_EXT_GML_H
