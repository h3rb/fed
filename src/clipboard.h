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

extern int clip_grid;
extern char *clip;
extern char *delbuffer;
extern int show_clipboard;

bool find_constraint  args( ( char **content, char **start, char **end ) );

void cut_area         args( ( char **content, char *start, char *end, int use_grid, int show_clip  ) );
void insert_area      args( ( char **content, char *start, char *end, int use_grid, char *ins ) );
void copy_area        args( ( char **content, char *start, char *end, int use_grid  ) );
void clear_area       args( ( char **content, char *start, char *end, int use_grid  ) );
void append_delbuffer args( ( char **content, int r ) );
void insert_delbuffer args( ( char **content, char *start, char *end ) );
void clear_delbuffer  args( ( void ) );
void draw_clipboard   args( ( void ) );
