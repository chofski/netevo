/* This software is distributed under the GNU Lesser General Public License */
//==========================================================================
//
//   gml_parser.cpp - parser for the GML-file-format specified in:
//                    Michael Himsolt, GML: Graph Modelling Language,
//                    21.01.1997 
//
//==========================================================================
// $Id: gml_parser.cpp,v 1.9 2001/11/07 13:58:10 pick Exp $

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "gml.h"

using namespace std;

namespace netevo {

/*
 * ISO8859-1 coding of chars >= 160
 */
char* GML_table[] = {
(char *)"&nbsp;",     /* 160 */
(char *)"&iexcl;",
(char *)"&cent;",
(char *)"&pound;",
(char *)"&curren;",
(char *)"&yen;",
(char *)"&brvbar;",
(char *)"&sect;",
(char *)"&uml;",
(char *)"&copy;",
(char *)"&ordf;",     /* 170 */
(char *)"&laquo;",
(char *)"&not;",
(char *)"&shy;",
(char *)"&reg;",
(char *)"&macr;",
(char *)"&deg;",
(char *)"&plusmn;",
(char *)"&sup2;",
(char *)"&sup3;",     /* 180 */
(char *)"&acute;",
(char *)"&micro;",
(char *)"&para;",
(char *)"&middot;",
(char *)"&cedil;",
(char *)"&sup1;",
(char *)"&ordm;",
(char *)"&raquo;",
(char *)"&frac14;",
(char *)"&frac12;",   
(char *)"&frac34;",   /* 190 */
(char *)"&iquest;",
(char *)"&Agrave;",
(char *)"&Aacute;",
(char *)"&Acirc;",
(char *)"&Atilde;",
(char *)"&Auml;",
(char *)"&Aring;",
(char *)"&AElig;",
(char *)"&Ccedil;",
(char *)"&Egrave;",   /* 200 */
(char *)"&Eacute;",
(char *)"&Ecirc;",
(char *)"&Euml;",
(char *)"&Igrave;",
(char *)"&Iacute;",
(char *)"&Icirc;",
(char *)"&Iuml;",
(char *)"&ETH;",
(char *)"&Ntilde;",
(char *)"&Ograve;",   /* 210 */
(char *)"&Oacute;",
(char *)"&Ocirc;",
(char *)"&Otilde;",
(char *)"&Ouml;",
(char *)"&times;",
(char *)"&Oslash;",
(char *)"&Ugrave;",
(char *)"&Uacute;",
(char *)"&Ucirc;",
(char *)"&Uuml;",     /* 220 */
(char *)"&Yacute;",
(char *)"&THORN;",
(char *)"&szlig;",
(char *)"&agrave;",
(char *)"&aacute;",
(char *)"&acirc;",
(char *)"&atilde;",
(char *)"&auml;",
(char *)"&aring;",
(char *)"&aelig;",    /* 230 */
(char *)"&ccedil;",
(char *)"&egrave;",
(char *)"&eacute;",
(char *)"&ecirc;",
(char *)"&euml;",
(char *)"&igrave;",
(char *)"&iacute;",
(char *)"&icirc;",
(char *)"&iuml;",
(char *)"&eth;",      /* 240 */
(char *)"&ntilde;",
(char *)"&ograve;",
(char *)"&oacute;",
(char *)"&ocirc;",
(char *)"&otilde;",
(char *)"&ouml;",
(char *)"&divide;",
(char *)"&oslash;",
(char *)"&ugrave;",
(char *)"&uacute;",   /* 250 */
(char *)"&ucirc;",
(char *)"&uuml;",
(char *)"&yacute;",
(char *)"&thorn;",
(char *)"&yuml;"
}; 


unsigned int GML_line;
unsigned int GML_column;


int GML_search_ISO (char* str, int len) {
	
	int i;
	int ret = '&'; 
	
	if (!strncmp (str, "&quot;", len)) {
		return 34;
	} else if (!strncmp (str, "&amp;", len)) {
		return 38;
	} else if (!strncmp (str, "&lt;", len)) {
		return 60;
	} else if (!strncmp (str, "&gt;", len)) {
		return 62;
	}
	
	for (i = 0; i < 96; i++) {
		if (!strncmp (str, GML_table[i], len)) {
			ret = i + 160;
			break;
		}
	}
	
	return ret;
}


void GML_init () {
	
	GML_line = 1;
	GML_column = 1;
}



struct GML_token GML_scanner (FILE* source) {
	
	unsigned int cur_max_size = INITIAL_SIZE;
	static char buffer[INITIAL_SIZE];
	char* tmp = buffer;
	char* ret = tmp;
	struct GML_token token;
	int is_float = 0;
	unsigned int count = 0;
	int next;
	char ISO_buffer[8];
	int ISO_count;
	
	assert (source != NULL);
	
	/* 
	 * eliminate preceeding white spaces
	 */
	
	do {
		next = fgetc (source);
		GML_column++;
		
		if (next == '\n') {
			GML_line++;
			GML_column = 1;
		}
		
	} while (isspace (next) && next != EOF);
	
	if (next == EOF) {
		
		/*
		 * reached EOF
		 */
		
		token.kind = GML_END;
		return token; 
		
	} else if (isdigit (next) || next == '.' || next == '+' || next == '-') {
		
		/* 
		 * floating point or integer 
		 */
		
		do {
			if (count == INITIAL_SIZE - 1) {
				token.value.err.err_num = GML_TOO_MANY_DIGITS;
				token.value.err.line = GML_line;
				token.value.err.column = GML_column + count;
				token.kind = GML_ERROR;
				return token;
			}
			
			if (next == '.' || next == 'E') {
				is_float = 1;
			}
			
			buffer[count] = next;
			count++;
			next = fgetc (source);
			
		} while (!isspace(next) && next != ']' && next != EOF);
		
		buffer[count] = 0;
		
		if (next == ']') {
			ungetc (next, source);
		}
		
		if (next == '\n') {
			GML_line++;
			GML_column = 1;
		} else {
			GML_column += count;
		}
		
		if (is_float) {
			token.value.floating = atof (tmp);
			token.kind = GML_DOUBLE;
		} else {
			token.value.integer = atol (tmp);
			token.kind = GML_INT;
		}
		
		return token;
		
	} else if (isalpha (next) || next == '_') {
		
		/*
		 * key
		 */
		
		do {
			if (count == cur_max_size - 1) {
				*tmp = 0;
				tmp =  (char*) malloc(2 * cur_max_size * sizeof (char));
				strcpy (tmp, ret);
				
				if (cur_max_size > INITIAL_SIZE) {
					free (ret);
				}
				
				ret = tmp;
				tmp += count;
				cur_max_size *= 2;
			}
			
			*tmp++ = next;
			count++;
			next = fgetc (source);
		} while (isalnum (next) || next == '_');
		
		if (next == '\n') {
			GML_line++;
			GML_column = 1;
		} else {
			GML_column += count;
		}
		
		if (next == '[') {
			ungetc (next, source);
		} else if (!isspace (next)) {
			token.value.err.err_num = GML_UNEXPECTED;
			token.value.err.line = GML_line;
			token.value.err.column = GML_column + count;
			token.kind = GML_ERROR;
			
			if (cur_max_size > INITIAL_SIZE) {
				free (ret);
			}
			
			return token;   
		} 
		
		*tmp = 0;
		token.kind = GML_KEY;
		token.value.string = (char*) malloc((count+1) * sizeof (char));
		strcpy (token.value.string, ret);
		
		if (cur_max_size > INITIAL_SIZE) {
			free (ret);
		}
		
		return token;
		
	} else {
		/*
		 * comments, brackets and strings
		 */
		
		switch (next) {
			case '#':
				do {
					next = fgetc (source);
				} while (next != '\n' && next != EOF);
				
				GML_line++;
				GML_column = 1;
				return GML_scanner (source);
				
			case '[':
				token.kind = GML_L_BRACKET;
				return token;
				
			case ']':
				token.kind = GML_R_BRACKET;
				return token;
				
			case '"':
				next = fgetc (source);
				GML_column++;
				
				while (next != '"') {
					
					if (count >= cur_max_size - 8) {
						*tmp = 0;
						tmp = (char*) malloc (2 * cur_max_size * sizeof(char));
						strcpy (tmp, ret);
						
						if (cur_max_size > INITIAL_SIZE) {
							free (ret);
						}
						
						ret = tmp;
						tmp += count;
						cur_max_size *= 2;
					}
					
					if (next == '&') {
						ISO_count = 0;
						
						while (next != ';') {
							if (next == '"' || next == EOF) {
								ungetc (next, source);
								ISO_count = 0;
								break;
							}
							
							if (ISO_count < 8) {
								ISO_buffer[ISO_count] = next;
								ISO_count++;
							}
							
							next = fgetc (source);
						}
						
						if (ISO_count == 8) {
							ISO_count = 0;
						}
						
						if (ISO_count) {
							ISO_buffer[ISO_count] = ';';
							ISO_count++;
							next = GML_search_ISO (ISO_buffer, ISO_count);
							ISO_count = 0;
						} else {
							next = '&';
						}
					} 
					
					*tmp++ = next;
					count++;
					GML_column++;
					
					next = fgetc (source);
					
					if (next == EOF) {
						token.value.err.err_num = GML_PREMATURE_EOF;
						token.value.err.line = GML_line;
						token.value.err.column = GML_column + count;
						token.kind = GML_ERROR;
						
						if (cur_max_size > INITIAL_SIZE) {
							free (ret);
						}
						
						return token;
					}
					
					if (next == '\n') {
						GML_line++;
						GML_column = 1;
					}
				}
				
				*tmp = 0;
				token.kind = GML_STRING;
				token.value.string = (char*) malloc((count+1) * sizeof (char));
				strcpy (token.value.string, ret);
				
				if (cur_max_size > INITIAL_SIZE) {
					free (ret);
				}
				
				return token;
				
			default:
				token.value.err.err_num = GML_UNEXPECTED;
				token.value.err.line = GML_line;
				token.value.err.column = GML_column;
				token.kind = GML_ERROR;
				return token;
		}		
	}
}


struct GML_pair* GML_parser (FILE* source, struct GML_stat* stat, int open) {
	
	struct GML_token token;
	struct GML_pair* pair;
	struct GML_pair* list;
	struct GML_pair* tmp = NULL;
	struct GML_list_elem* tmp_elem;
	
	assert (stat);
	
	pair = (struct GML_pair*) malloc (sizeof (struct GML_pair));
	list = pair;
	
	for (;;) {
		token = GML_scanner (source);
		
		if (token.kind == GML_END) {
			if (open) {
				stat->err.err_num = GML_OPEN_BRACKET;
				stat->err.line = GML_line;
				stat->err.column = GML_column;
				free (pair);
				
				if (tmp == NULL) {
					return NULL;
				} else {
					tmp->next = NULL;
					return list;
				}
			}
			
			break;
			
		} else if (token.kind == GML_R_BRACKET) {
			if (!open) {
				stat->err.err_num = GML_TOO_MANY_BRACKETS;
				stat->err.line = GML_line;
				stat->err.column = GML_column;
				free (pair);
				
				if (tmp == NULL) {
					return NULL;
				} else {
					tmp->next = NULL;
					return list;
				}
			}
			
			break;
			
		} else if (token.kind == GML_ERROR) {
			stat->err.err_num = token.value.err.err_num;
			stat->err.line = token.value.err.line;
			stat->err.column = token.value.err.column;
			free (pair);
	      
			if (tmp == NULL) {
				return NULL;
			} else {
				tmp->next = NULL;
				return list;
			}
			
		} else if (token.kind != GML_KEY) {
			stat->err.err_num = GML_SYNTAX;
			stat->err.line = GML_line;
			stat->err.column = GML_column;
			free (pair);
			
			if (token.kind == GML_STRING) {
				free (token.value.string);
			}
			
			if (tmp == NULL) {
				return NULL;
			} else {
				tmp->next = NULL;
				return list;
			}
		}
	   
		if (!stat->key_list) {
			stat->key_list = (struct GML_list_elem*) 
			malloc (sizeof (struct GML_list_elem));
			stat->key_list->next = NULL;
			stat->key_list->key = token.value.string;
			pair->key = token.value.string;
			
		} else {
			tmp_elem = stat->key_list;
			
			while (tmp_elem) {
				if (!strcmp (tmp_elem->key, token.value.string)) {
					free (token.value.string);
					pair->key = tmp_elem->key;
					
					break;
				}
				
				tmp_elem = tmp_elem->next;
			}
			
			if (!tmp_elem) {
				tmp_elem = (struct GML_list_elem*)
				malloc (sizeof (struct GML_list_elem));
				tmp_elem->next = stat->key_list;
				stat->key_list = tmp_elem;
				tmp_elem->key = token.value.string;
				pair->key = token.value.string;
			}
		}
		
		token = GML_scanner (source);
		
		
		switch (token.kind) {
			case GML_INT:
				pair->kind = GML_INT;
				pair->value.integer = token.value.integer;
				break;
				
			case GML_DOUBLE:
				pair->kind = GML_DOUBLE;
				pair->value.floating = token.value.floating;
				break;
				
			case GML_STRING:
				pair->kind = GML_STRING;
				pair->value.string = token.value.string;
				break;
				
			case GML_L_BRACKET:
				pair->kind = GML_LIST;
				pair->value.list = GML_parser (source, stat, 1);
				
				if (stat->err.err_num != GML_OK) {
					return list;
				}
				
				break;
				
			case GML_ERROR:
				stat->err.err_num = token.value.err.err_num;
				stat->err.line = token.value.err.line;
				stat->err.column = token.value.err.column;
				free (pair);
				
				if (tmp == NULL) {
					return NULL;
				} else {
					tmp->next = NULL;
					return list;
				}
				
			default:    
				stat->err.line = GML_line;
				stat->err.column = GML_column;
				stat->err.err_num = GML_SYNTAX;
				free (pair);
				
				if (tmp == NULL) {
					return NULL;
				} else {
					tmp->next = NULL;
					return list;
				}
		}
		
		tmp = pair;
		pair = (struct GML_pair*) malloc (sizeof (struct GML_pair));
		tmp->next = pair;
	}
	
	stat->err.err_num = GML_OK;
	free (pair);
	
	if (tmp == NULL) {
		return NULL;
	} else {
		tmp->next = NULL;
		return list;
	}
}


void GML_free_list (struct GML_pair* list, struct GML_list_elem* keys) {
	
	struct GML_pair* tmp = list;
	struct GML_list_elem* tmp_key;
	
	while (keys) {
		free (keys->key);
		tmp_key = keys->next;
		free (keys);
		keys = tmp_key;
	}
	
	while (list) {
		
		switch (list->kind) {
			case GML_LIST:
				GML_free_list (list->value.list, NULL);
				break;
				
			case GML_STRING:
				free (list->value.string);
				break;
				
			default:
				break;
		}
		
		tmp = list->next;
		free (list);
		list = tmp;
	}
}



void GML_print_list (struct GML_pair* list, int level) {
	
	struct GML_pair* tmp = list;
	int i;
	
	while (tmp) {
		
		for (i = 0; i < level; i++) {
			printf ("    ");
		}
		
		printf ("*KEY* : %s", tmp->key);
		
		switch (tmp->kind) {
			case GML_INT:
				printf ("  *VALUE* (long) : %ld \n", tmp->value.integer);
				break;
				
			case GML_DOUBLE:
				printf ("  *VALUE* (double) : %f \n", tmp->value.floating);
				break;
				
			case GML_STRING:
				printf ("  *VALUE* (string) : %s \n", tmp->value.string);
				break;
				
			case GML_LIST:
				printf ("  *VALUE* (list) : \n");
				GML_print_list (tmp->value.list, level+1);
				break;
				
			default:
				break;
		}
		
		tmp = tmp->next;
	}
}
	
} // netevo namespace
