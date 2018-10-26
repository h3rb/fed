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

/* This file contains fed plug-in loading and execution functions */

#include "fed.h"
#include "b64.h"
#include "io.h"
#include "plugin.h"
#include "strings.h"
#include "color.h"
#include "display.h"
#include "input.h"
#include "graphics.h"

PLUGIN *plugins=NULL;

RESULT *plugin_results=NULL;

char *process_plugin( PLUGIN *p, char *data ) {
 char *literalized=literalize(data),*encoded;
 char call[1024], script[strlen(p->content)+strlen(literalized)+256];
// printf( "\r\n --Code: \n%s\n", p->content );
// printf( "\r\n --Literalized: \n%s\n", literalized );
 // Copy and insert data
 sprintf( script, "<?php $INCOMING='%s'; ?>%s", literalized, p->content );
 free(literalized);
// printf( "\r\n --Script: \n%s\n", script );
 write_ascii( script, ".fedscript.php" );
 sprintf( call, "php -f .fedscript.php > .fedout 2> .fedlog" );
 system( call );
 unlink( ".fedscript.php" );
 unlink( ".fedlog" );
 encoded=fread_ascii( ".fedout" );
// printf( "\r\n --Encoded: \n%s\n", encoded );
 unlink( ".fedout" );
 // decode the encoded result for each bound piece
 return encoded;
}

void decode_plugin_output( char *encoded ) {
 char *p,*accumulator=str_dup("");
 RESULT *r,*next;
 r=plugin_results;
 while (r) { next=r->next; if ( r->option ) free( r->option ); if ( r->value ) free(r->value); free(r); r=next; }
 plugin_results=NULL;
 p=encoded;
 r=NULL;
 while ( p && *p != '\0' ) {
  if ( *p == '-' ) {
   if ( equals( p, "--fedboundary--" ) ) {
    p+=strlen("--fedboundary-");
    if ( !r ) {
     r=malloc(sizeof(RESULT));
     r->option=(accumulator);
     r->value=NULL;
     r->next=plugin_results;
     plugin_results=r;
     accumulator=str_dup("");
    } else {
     r->value=(accumulator);
     accumulator=str_dup("");
     r=NULL;
    }
   } else char_append(&accumulator,*p);
  } else {
   char_append(&accumulator,*p);
  }
  p++;
 }
 if ( strlen(accumulator) > 0 ) {
  if ( !r ) {
   r=malloc(sizeof(RESULT));
   r->option=(accumulator);
   r->value=NULL;
   r->next=plugin_results;
   plugin_results=r;
   accumulator=str_dup("");
  } else {
   r->value=(accumulator);
   accumulator=str_dup("");
   r=NULL;
  }
 } else free(accumulator);
 free(encoded);
}

void read_plugin_folder( char *path, int report_it ) {
 PLUGIN *new;
 char pkey[256];
 char filename[2048];
 char *list,*p,*q,*desc,*code;
 if ( report_it ) printf( "fed: Loading plugins from %s\n", path );
 sprintf( filename, "%s/index.txt", path );
 list =fread_ascii( filename );
 if ( !list || strlen(list)==0 ) { printf( "%s/index.txt not found or empty!\n", path ); return; }
 p=list;
 while ( *p != '\0' ) {
  q=pkey;
  while ( *p != '\n' && *p != '\0' ) {
   *q=*p;
   q++; p++;
  }
  *q='\0';
  if ( report_it )
  {
   bool found=false;
   PLUGIN *old;
   for ( old=plugins; old; old=old->next ) if ( equals(pkey,old->name) ) found=true;
   if ( found ) {
    printf( "Plugin of a similar name already loaded, skipping.\n" );
    if ( *p == '\n' ) p++;
    continue;
   }
  }
  sprintf( filename, "%s/%s.txt", path, pkey );
  desc=fread_ascii( filename );
  sprintf( filename, "%s/%s.php", path, pkey );
  code=fread_ascii( filename );
  new=malloc(sizeof(PLUGIN));
  new->name=str_dup(pkey);
  new->content=code ? code : str_dup("");
  new->description=desc ? desc : str_dup("");
  new->next=plugins;
  plugins=new;
  if ( report_it ) printf( " Loaded plugin '%s'                   \n", pkey );
  if ( *p == '\n' ) p++;
 }
}

RESULT *result_selected=NULL;

void draw_plugin_result_menu( void ) {
 RESULT *r=plugin_results;
 int y=0,i;
 i=0;
 if ( result_selected ) while ( r != result_selected ) { r=r->next; i++; }
 if ( i * 3 + 6 + 3 < row-16 ) i=0;
 {
  RESULT *a=plugin_results;
  char buf[256];
  int result_count=0;
  while ( a ) { result_count++; a=a->next; }
  sprintf( buf,
   "^^E.------------------[%5d results]-----------------------------[^^Fc-X^^E]--.", result_count);
  draw_color_coded( 3, 3, buf );
 }
 draw_color_coded( 3, 4,
  "^^E|^^A Pick Desired Result                               (Enter to Select) ^^E|");
 draw_color_coded( 3, 5,
  "^^E+---------------------------------------------------------------------+");
 r=plugin_results;
 while ( r ) if ( i > 0 ) { i--; r=r->next; } else {
  attron( r == result_selected ? COLOR_PAIR(GREEN_ON_BLUE) : COLOR_PAIR(YELLOW_ON_RED) );
  mvprintw( 6+y, 3, "|%c%-67s%c|", r==result_selected?'>':' ', fit(r->option,67), r==result_selected?'<':' '); y++;
  attroff( r == result_selected ? COLOR_PAIR(GREEN_ON_BLUE) : COLOR_PAIR(YELLOW_ON_RED) );
  r=r->next;
  if ( y >= row-16 ) break;
 }
 draw_color_coded( 3, 6+y,
  "^^E+-------------------------------------------------[^^FUp^^E]--[^^FDown^^E]--------+" );
}

PLUGIN *selected=NULL;

void draw_plugin_menu( void ) {
 PLUGIN *p=plugins;
 int y=0,i,j;
 i=0;
 if ( selected ) while ( p != selected ) { p=p->next; i++; }
 if ( i * 3 + 6 + 3 < row-16 ) i=0; else i-=1;
 draw_color_coded( 3, 3,
  "^^E.--------------------------------------------------------------[^^Fc-X^^E]--.");
 draw_color_coded( 3, 4,
  "^^E|^^A Loaded PHP Plug-ins                               (Enter to Select) ^^E|");
 draw_color_coded( 3, 5,
  "^^E+---------------------------------------------------------------------+");
 p=plugins;
 while ( p ) if ( i > 0 ) { i--; p=p->next; } else {
  attron( p == selected ? COLOR_PAIR(GREEN_ON_BLUE) : COLOR_PAIR(YELLOW_ON_RED) );
  mvprintw( 6+y, 3, "|%c%-67s%c|", p==selected?'>':' ', fit(p->name,67), p==selected?'<':' '); y++;
  for ( j=0; j<3; j++ )
  draw_color_coded( 3, 6+y+j,
     "|                                                                     |");
  draw_color_coded( 3+2, 6+y, p->description ); y++; y++; y++; // Three-line mini description.
  attroff( p == selected ? COLOR_PAIR(GREEN_ON_BLUE) : COLOR_PAIR(YELLOW_ON_RED) );
  p=p->next;
  if ( y >= row-16 ) break;
 }
 draw_color_coded( 3, 6+y,
  "^^E+-------------------------------------------------[^^FUp^^E]--[^^FDown^^E]--------+" );
}

void open_plugin_menu( void ) {
 plugin_menu=1;
 selected=plugins;
}
