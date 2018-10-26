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

extern FEDFILE *files;
extern int report_io;

void sbeep args( (void) );
char char_by_num args( ( int progress ) );
char *fread_ascii args( ( char *filepath ) );
char *fread_binary args( ( char *filepath ) );
char *extension args( ( char *filename ) );
int count_fedfiles args( ( FEDFILE *feds ) );
int write_ascii args( ( char *data, char *file ) );
int write_binary args( ( char *data, char *file ) );
FEDFILE *append_fedfile args( ( FEDFILE *list, char *filename, int mode ) );
FEDFILE *prepend_fedfile args( ( FEDFILE *list, char *filename, int mode ) );
FEDFILE *append_fedfile_new args( ( FEDFILE *list, char *filename, int mode ) );
