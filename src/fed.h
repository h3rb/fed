/*
+-------------------------+
|      ___           _    | fed : a linux console editor
|     / __)         | |   |
|   _| |__ _____  __| |   | (c)2011 h. elwood gilliland iii
|  (_   __) ___ |/ _  |   |
|    | |  | ____( (_| |   | supports: figlets, GeSHi
|    |_|  |_____)\____|   |
|                         | dependencies: gcc, php
+-------------------------+
 */

#include <ncurses.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <malloc.h>

#define EXE_FILE "fed"

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )            ( )
#define COMMAND( fun )          void fun( )
#define DECLARE_HIT_FUN( fun )          void fun( )
#else
#define args( list )            list
#define COMMAND( fun )          CMD_FUN   fun
#define DECLARE_HIT_FUN( fun )          HIT_FUN   fun
#endif

/*
 * Short scalar types.
 */
#if    !defined(FALSE)
#define FALSE     0
#endif

#if    !defined(TRUE)
#define TRUE     1
#endif

#if    defined(_AIX)
#if    !defined(const)
#define const
#endif
#define unix
#else
#endif

/*
 * Structs
 */
typedef struct  fedfile       FEDFILE;
typedef struct  plugin        PLUGIN;
typedef struct  result        RESULT;
typedef struct  string_list   STRING_LIST;
typedef struct  gbuffer       GBUFFER;
typedef struct  undo          UNDO;
typedef struct  note          NOTE;
typedef struct  lang          LANG;
typedef struct  save_request  SAVE_REQUEST;
typedef struct  search_result SEARCH_RESULT;

struct string_list {
 char *str;
 STRING_LIST *next;
};

struct plugin {
 char *name;
 char *description;
 char *content;
 PLUGIN *next;
};

struct result {
 char *option;
 char *value;
 RESULT *next;
};

struct fedfile {
 UNDO *undos;
 UNDO *redos;

 char *highlight; // highlighting buffer
 bool changed,modified;
 char *original;
 char *fileraw;
 char *lang;
 char *filename;

 /* Selection */
 int grid;
 char *begin, *end;
 int sx,sy,ex,ey;
 // Page scroll
 int ofsx,ofsy;
 // Cursor
 int cx, cy;
 char *curc;

 int type;
 int inserting;
 int r,c;
 int total_lines;
 FEDFILE *next;
};

struct gbuffer {
 char *content;
 char *colormap;
 int w,h,mode;
};

struct color_element
{
   char         code[10];
   char         name[15];
   char         css[10];
};

struct undo {
 char *fileraw;
 UNDO *next;
};

struct note {
 char *content;
 int duration;
 int time;
 NOTE *next;
};

struct lang {
 char *name;
 int symbols;
 char **symbol;
 int *color;
 bool case_sensitive;
 LANG *next;
};

struct save_request {
 char *name;
 FEDFILE *file;
 int enter_pressed;
 SAVE_REQUEST *next;
};

struct search_result {
 FEDFILE *file;
 char *position;
 int touched;
 int line_no,col_no;
 SEARCH_RESULT *next;
};

extern const struct color_element color_code [];

#define F_ASCII  0
#define F_BINARY 1

#define VERSION      "fed 1.0 beta"

/*
 * Utility macros.
 */
#define UMIN(a, b)        ((a) < (b) ? (a) : (b))
#define UMAX(a, b)        ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)        ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)        ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)        ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_VOWEL(c)             (c == 'A' || c == 'a' ||                     \
                                 c == 'E' || c == 'e' ||                     \
                                 c == 'I' || c == 'i' ||                     \
                                 c == 'O' || c == 'o' ||                     \
                                 c == 'U' || c == 'u'      )

/*
 * main.c
 */

extern int go;
//extern int used_termcap;
//extern char *termtype;
//extern int term_lines;
//extern int term_columns;
//extern int old_term_lines;
//extern int old_term_columns;
extern int row,col;
extern struct winsize window_size;
extern struct winsize old_window_size;
void resized args( (void) );
void get_window_size args( (void) );
void init args ( ( void ) );
void loop args ( ( void ) );

//#define WINWIDTH    ( used_termcap ? term_lines   : window_size.ws_row )
//#define WINHEIGHT   ( used_termcap ? term_columns : window_size.ws_col )

/*
#define WINWIDTH  ( window_size.ws_row )
#define WINHEIGHT ( window_size.ws_col )
#define OLDWIDTH  ( old_window_size.ws_row )
#define OLDHEIGHT ( old_window_size.ws_col )
 */

#define WINHEIGHT row
#define WINWIDTH  col

//#define OLDWIDTH    ( used_termcap ? old_term_lines   : old_window_size.ws_row )
//#define OLDHEIGHT   ( used_termcap ? old_term_columns : old_window_size.ws_col )

extern char *cwd;

char *gnu_getcwd args( (void) );
