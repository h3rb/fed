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

extern int show_eol_eof;

GBUFFER *new_gbuffer args( ( int w, int h ) );
void free_gbuffer args( ( GBUFFER *old ) );
GBUFFER *clone_gbuffer args( ( GBUFFER *in, int copyit ) );
void copy_region args( ( GBUFFER *src, GBUFFER *dst, int sx, int sy, int w, int h ) );
void cursor_in_file args( ( FEDFILE *file, int w, int h ) );
GBUFFER *setup_gbuffer args( ( FEDFILE *file, int w, int h ) );
void render_gbuffer args( ( GBUFFER *g, GBUFFER *litho, int cs, int term, int invert ) );
