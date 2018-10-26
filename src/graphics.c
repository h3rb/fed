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

#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "fed.h"
#include "string.h"
#include "graphics.h"

int mode=MODE_ANSI;

void mvprintw2( int y, int x, char *coded ) {
 draw_color_coded( x, y, coded );
}

void draw_color_coded( int x, int y, char *coded ) {
 int xx=x; int yy=y;
 char *p=coded;
 int curattr=-1;

 while ( *p != '\0' ) {
  if ( *p == '\r' ) { p++; } else
  if ( *p == '\t' ) { p++; } else
  if ( *p == '\n' ) { yy++; xx=x; p++; } else
  if ( *p == '^' && *(p+1) == '^' ) {
   p++; p++;
   if ( curattr != -1 ) attroff( curattr );
   switch ( *(p) ) {
    case '1': attron( curattr=(COLOR_PAIR(1)) );  break;
    case '2': attron( curattr=(COLOR_PAIR(2)) );  break;
    case '3': attron( curattr=(COLOR_PAIR(3)) );  break;
    case '4': attron( curattr=(COLOR_PAIR(4)) );  break;
    case '5': attron( curattr=(COLOR_PAIR(5)) );  break;
    case '6': attron( curattr=(COLOR_PAIR(6)) );  break;
    case '7': attron( curattr=(COLOR_PAIR(7)) );  break;
    case '8': attron( curattr=(COLOR_PAIR(8)) );  break;
    case '9': attron( curattr=(COLOR_PAIR(9)) );  break;
    case '0': attron( curattr=(COLOR_PAIR(10)) ); break;
    case 'A': attron( curattr=(COLOR_PAIR(11)) ); break;
    case 'B': attron( curattr=(COLOR_PAIR(12)) ); break;
    case 'C': attron( curattr=(COLOR_PAIR(13)) ); break;
    case 'D': attron( curattr=(COLOR_PAIR(14)) ); break;
    case 'E': attron( curattr=(COLOR_PAIR(15)) ); break;
    case 'F': attron( curattr=(COLOR_PAIR(16)) ); break;
    case 'G': attron( curattr=(COLOR_PAIR(17)) ); break;
    case 'H': attron( curattr=(COLOR_PAIR(18)) ); break;
    case 'I': attron( curattr=(COLOR_PAIR(19)) ); break;
    case 'J': attron( curattr=(COLOR_PAIR(20)) ); break;
    case 'K': attron( curattr=(COLOR_PAIR(21)) ); break;
    case 'L': attron( curattr=(COLOR_PAIR(22)) ); break;
    case 'M': attron( curattr=(COLOR_PAIR(23)) ); break;
    case 'N': attron( curattr=(COLOR_PAIR(24)) ); break;
    case 'O': attron( curattr=(COLOR_PAIR(25)) ); break;
    case 'P': attron( curattr=(COLOR_PAIR(26)) ); break;
    case 'Q': attron( curattr=(COLOR_PAIR(27)) ); break;
    default: break;
   }
   p++;
  } else { mvprintw( yy, xx, "%c", *p ); p++; xx++; }
 }
 if ( curattr != -1 ) attroff( curattr );
}
