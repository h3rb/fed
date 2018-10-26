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

extern LANG *langs;
extern bool langs_loaded;

LANG *find_lang args( ( char *lang ) );
bool not_in_comment args( ( char *location, char *code ) );
bool alphaisolated args( ( char *key, char *position, char *code ) );
char *colorize args( ( char *code, LANG *lang ) );
void add_to_lang args( ( LANG *lang, int color, char *symbol ) );
void load_lang args( ( char *filename, char *lang ) );
void load_langs args( ( void ) );
