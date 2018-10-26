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

#include "fed.h"
#include "cli.h"
#include "dashboard.h"
#include "display.h"
#include "graphics.h"
#include "io.h"
#include "timer.h"
#include "strings.h"
#include "color.h"
#include "clipboard.h"

extern int has_color;

int ruler_y=0;
int gbuffer_start=1;

NOTE *notes=NULL;

void new_note( char *content, int time_in_sec ) {
 NOTE *n=malloc(sizeof(NOTE));
 n->content=str_dup(content);
 n->duration=time_in_sec;
 n->time=now;
 n->next=notes;
 notes=n;
 return;
}

void draw_multi_tabs( int loaded_files ) {
 bool activeone=false;
 int pertab=(col-1)/(loaded_files),m;
 int writing,activetab,activelength,diff=0;
 static char tab[1024];
 char *t;
 FEDFILE *a=files;
 attron(COLOR_PAIR(YELLOW_ON_MAGENTA));
 for ( m=0; m<col; m++ ) mvprintw( 0, m, " " ); // clear top line
 attroff(COLOR_PAIR(YELLOW_ON_MAGENTA));
 if ( pertab < _mintab ) pertab=_mintab; // adjust to minimum tab size
//  if ( pertab < 12 ) pertab=12;
 a=files;
 writing=0;
 while ( a ) { // calculate the whole line
  int begin=writing;
  int g; char *p=a->filename;
  tab[writing++]='[';
  for ( g=0; g<pertab-2; g++ ) { if ( !p || *p == '\0' ) tab[writing++]=' '; else { tab[writing++]=*p; p++; } }
  tab[writing++]=']';
  if ( a == active ) {
   activeone=true;
   activetab=begin;
   activelength=writing-begin;
  }
  a=a->next;
 }
 tab[writing]='\0';

 while ( activetab-diff > col-1 ) {
  diff+=col-1;
 }
 t=&(tab[diff]);

 // draw the multi tab
 if ( has_color ) attron(COLOR_PAIR(BLACK_ON_RED));
 mvprintw( 0, 0, "%s", fit(t,col) );
 if ( has_color ) attroff(COLOR_PAIR(BLACK_ON_RED));
 if ( has_color ) attron(COLOR_PAIR(WHITE_ON_BLUE)); // active tab highlight
 t=&(tab[activetab]);
 tab[activetab+activelength]='\0';
 mvprintw( 0, activetab-diff, "%s", t );
 if ( has_color ) attroff(COLOR_PAIR(WHITE_ON_BLUE));
}

void dashboard( void ) {
 int loaded_files=count_fedfiles( files );
 static char selecting[16];
 static char clipboard[16];
 static char buf[1024];
 gbuffer_start= _ruler ? ( loaded_files > 1 ? 2 : 1 ) : ( loaded_files > 1 ? 1 : 0 );
 if ( loaded_files > 1 ) { // multi-tab view
  ruler_y=1;
  gbuffer_start=_ruler ? 2 : 1;
  draw_multi_tabs(loaded_files);
 } else {
  ruler_y=0;
  gbuffer_start=1;
 }
 if ( _ruler ) {
  if ( has_color ) attron(COLOR_PAIR(BLACK_ON_WHITE));
  mvprintw(ruler_y,0,
   trunc_fit(
    "0....5...9.A...E....|20..,....|30..,....|....,....|....,....|60..,..70|.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V.........V",
    col-1, 0, 0 )
   );
  if ( has_color ) attroff(COLOR_PAIR(BLACK_ON_WHITE));
 }
 if ( _clock ) {
  if ( _ruler ) {
   static char clockface[15];
   sprintf( clockface, "^^O%s", time_string );
   draw_color_coded(col-strlen(time_string),ruler_y, clockface );
  }
 }
 if ( _help ) {
 }
 selecting[0]='\0';
 if ( active->begin ) {
  if ( active->end ) sprintf( selecting, "Selected" );
  else if ( active->grid ) sprintf( selecting, "Grid" );
  else sprintf( selecting, "Selecting");
 }
 if ( clip && strlen(clip) > 0 ) {
  sprintf( clipboard, "Clipboard" );
 } else clipboard[0]='\0';
 if ( notes ) {
  if ( now - notes->time > notes->duration ) {
   NOTE *n=notes;
   notes=notes->next;
   free(n->content);
   free(n);
  } else {
   attron(COLOR_PAIR(YELLOW_ON_RED));
   mvprintw( row-2, 0, "%s", fit( notes->content, col ) );
   attroff(COLOR_PAIR(YELLOW_ON_RED));
  }
 }
 attron(COLOR_PAIR(YELLOW_ON_MAGENTA));
 if ( active ) {
  sprintf( buf, "%5d/%5d:%5d [%40s] %3s %5s %9s %9s \\%-3d %6s %8s",
   active->r, count_occurrances( active->fileraw, "\n" ), active->c,
   trunc_fit( active->filename, 40, 1, 1 ),
   (active->inserting ? "Ins" : ""),
   "",//(_clock ? time_string : ""),
   selecting,
   clipboard,
   ((active->curc)? (int) *(active->curc) : -1),
   (delbuffer && strlen(delbuffer) > 0 ? "Buffer" : ""),
   (active->modified? "Modified" : "")
   );
  mvprintw( row-1, 0, "%s", trunc_pad( trunc_fit( buf, col, 0, 0 ), col ) );
  attroff(COLOR_PAIR(YELLOW_ON_MAGENTA));
  if ( then % 2 == 1 )
   attron(COLOR_PAIR(BLUE_ON_WHITE));
  else
   attron(COLOR_PAIR(YELLOW_ON_BLUE));
  mvprintw( active->cy, active->cx, "%c",
   ( active->curc ?
    ( char_isof(*(active->curc),"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-=_+[]\\{}|;':\",./<>?`~!@#$%^&*()")
      ? *(active->curc) : ' ' )
    : ' '));
  if ( then % 2 == 1 )
   attroff(COLOR_PAIR(BLUE_ON_WHITE));
  else
   attroff(COLOR_PAIR(YELLOW_ON_BLUE));
 }
 refresh();
}
