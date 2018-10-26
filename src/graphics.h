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

#define MODE_ANSI 1
#define MODE_ASCII 0

extern int mode;

void mvprintw2 args( ( int y, int x, char *coded ) );
void draw_color_coded args( ( int x, int y, char *coded ) );
