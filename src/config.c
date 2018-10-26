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
#include "config.h"
#include "io.h"
#include "strings.h"
#include "cli.h"

void load_config( bool use_dot_fed, bool use_root_dot_fed ) {
 char *home=getenv("HOME");
 int i;
 if ( use_dot_fed ) {
  char *contents=fread_ascii(".fed");
  char buf[2048];
  if ( contents ) {
   if ( inside(contents,"-root") ) use_root_dot_fed=false;
   free(contents);
  }
  sprintf( buf, "%s/.fed", home );
  contents=fread_ascii(home);
  if ( contents ) {
   if ( inside(contents,"-root") ) use_root_dot_fed=false;
   free(contents);
  }
 }
 if ( use_root_dot_fed ) {
  char *dotfed=fread_ascii( "/etc/fed/.fed" );
  int count;
  char **a=wordlist( dotfed, &count );
  if ( dotfed ) {
   if ( a ) {
    printf( "Configuring using /etc/fed/.fed                   \n" );
    command_line( count, a, 0 );
    for ( i=0; i<count; i++ ) free( a[i] );
    free(a);
   }
   free(dotfed);
  }
 }
 if ( use_dot_fed ) {
  char buf[2048];
  char *dotfed=fread_ascii( ".fed" );
  if ( !dotfed ) dotfed=fread_ascii( "./.fed" );
  if ( dotfed ) {
   int count;
   char **a=wordlist( dotfed, &count );
   if ( a ) {
    printf( "Configuring using ./.fed                         \n" );
    command_line( count, a, 0 );
    for ( i=0; i<count; i++ ) free( a[i] );
    free(a);
   }
   free(dotfed);
  }
  sprintf( buf, "%s/.fed", home );
  dotfed=fread_ascii( buf );
  if ( dotfed ) {
   int count;
   char **a=wordlist( dotfed, &count );
   if ( a ) {
    printf( "Configuring using %s/.fed                \n", home );
    command_line( count, a, 0 );
    for ( i=0; i<count; i++ ) free( a[i] );
    free(a);
   }
   free(dotfed);
  }
 }
}
