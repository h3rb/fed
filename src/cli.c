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

/* This file is for fed to parse arguments from the command line */

#include "fed.h"
#include "cli.h"
#include "display.h"
#include "io.h"
#include "strings.h"
#include "lang.h"
#include "input.h"

char *cli_help = ""
"+-------------------------+\n"
"|      ___           _    | Usage: fed [options] <filenames>\n"
"|     / __)         | |   |\n"
"|   _| |__ _____  __| |   | Options: --help,-h,-ruler,+clock,+old\n"
"|  (_   __) ___ |/ _  |   |          -plugins,-fed,-root,+free,-lf,\n"
"|    | |  | ____( (_| |   |          +fedhome=path,+mintab=#,+undo=#,+count,\n"
"|    |_|  |_____)\\____|   |         +lang=path,+plugins=path\n"
"| a linux console editor! | \n"
"+-------------------------+ fed --help displays verbose help\n"
"(c)2011 h. elwood gilliland iii URL: http://sourceforge.net/projects/fed/\n";

char *cli_verbose = ""
"+-------------------------+\n"
"|      ___           _    | Usage: fed [options] <filenames>\n"
"|     / __)         | |   |\n"
"|   _| |__ _____  __| |   | Options: --help,-h,-ruler,+clock,+old\n"
"|  (_   __) ___ |/ _  |   |          -plugins,-fed,-root,+free,-lf,\n"
"|    | |  | ____( (_| |   |          +fedhome=path,+mintab=#,+undo=#,+count,\n"
"|    |_|  |_____)\\____|   |         +lang=path,+plugins=path\n"
"| a linux console editor! | \n"
"+-------------------------+ fed --help displays verbose help\n"
"(c)2011 h. elwood gilliland iii URL: http://sourceforge.net/projects/fed/\n"
"\n"
"Usage: fed [options] [filename(s)...]\n"
"\n"
"Where 'options' is one or more of:\n"
"\n"
"--help         Displays verbose help\n"
"-h             Displays brief help\n"
"-ruler         Suppresses the column ruler\n"
"+clock         Adds a clock\n"
"+old           Saves copy of the file(s) as name.old on load from command line\n"
"+plugins=path  Loads additional plugin folder (total of 1 extra path)\n"
"-plugins       Loads no plugins\n"
"+lang=path     Loads alternative language definition files\n"
"-lang          Suppresses language definition and syntax highlighting\n"
"-fed           Ignores .fed configuration files (both root and home)\n"
"-root          Ignores root .fed if it exists\n"
"+count         Fed will simply load and count the lines for each file, then exit\n"
"+b64           transforms fed into Bob Trower's b64 utility\n"
"+free          fed starts with \"Free Move\" mode on\n"
"+fedhome=path  Sets the path to fed's home (/usr/bin and /etc/fed as default)\n"
"+mintab=#      Number of spaces that mimic a \t (5 is default)\n"
"+lang=path     Changes to an alternative language folder\n"
"\n";

 int _help=1;
 int _fallback=1;
 int _ruler=1;
 int _clock=0;
 int _lang=1;
 int _plugins=1;
 int _fed=1;
 int _root=1;
 int _binary=0;
 int _invert=0;
 int _clearscreen=1;
 int _statusbar=1;
 int _undo=50;
 int _mintab=12;
 int _tabspaces=5;
 int _eat_lf=0;
 int _old=0;

char *lang_home=NULL;
char *fed_home=NULL;
char *plugins_folder=NULL;

void command_line( int argc, char **argv, int load_files ) {
 int i;
 /*
 printf( "fed: Invoked with the following command line:\n" );
 for ( i=0; i<argc; i++ ) printf( "%s ", argv[i] );
  */
 printf( "\n" );
 if ( argc > 1 ) for ( i=load_files?1:0; i<argc; i++ ) {
  char *a=argv[i];
  a=skip_spaces(a);
  if ( ( a[0]=='-' && a[1]=='h' && a[2]=='\0' ) ) {
   printf( "\n%s\n%s\n", VERSION, cli_help );
   exit(0);
  }
  else
  if ( equals( a, "--help" ) ) {
   printf( "\n%s\n%s\n", VERSION, cli_verbose );
   exit(0);
  }
  else
  if ( *a == 'b' && *(a+1) == '+' && *(a+2) == '\0' ) {
   printf( "Creating new binary file.\n" );
   append_fedfile_new( files, "/", 1 );
  }
  else
  if ( prefix( a, "+count" ) ) {
   i++;
   if ( i == argc ) {
    printf( "fed +count was used but no files were provided.\n"
            "Usage: fed +count <text file names in a list each separated by a space>\n\n" );
    exit(0);
   }
   while ( i < argc ) {
    FILE *exists=fopen( argv[i], "r" );
    if ( !exists ) printf( "fed: file \"%s\" was not found\n", argv[i] );
    else {
     int counted;
     char *content;
     fclose(exists);
     content=fread_ascii(argv[i]);
     counted=count_occurrances( content, "\n" );
     printf( "\n" );
     printf( "fed: file \"%s\"\n", argv[i] );
     if ( counted > 0 )
      printf( " contains %d new line characters (\\n)\n", counted );
     counted=count_occurrances( content, "\r" );
     if ( counted > 0 )
      printf( " contains %d carrier returns (\\r)\n", counted );
     counted=count_occurrances( content, "\r\n" );
     if ( counted > 0 )
      printf( " contains %d CRLFs (\\r\\n)\n", counted );
     counted=count_occurrances( content, "\n\r" );
     if ( counted > 0 )
      printf( " contains %d LFCRs (\\n\\r)\n", counted );
    }
    i++;
   }
   exit(0);
  }
  else
  if ( equals( a, "-lf" ) ) {
   printf( "fed: Munching LFs for breakfast.  This ain't no MS-DOS!\n" );
   _eat_lf=1;
  } else
  if ( equals( a, "+b64" ) ) {
   printf( "fed: Transform into Bob Trower's b64, and roll out! (cool sound effect here)\n" );
  }
/*  else
  if ( equals( a, "-help" ) ) {
   printf( "fed: Disabling help dashboard cheat sheet.\n" );
   _help=1;
  }*/
  else
  if ( equals( a, "+fallback" ) ) {
   printf( "fed: Enabling fallback interface.\n" );
   _fallback=1;
  }
  else
  if ( equals( a, "-ruler" ) ) {
   printf( "fed: Disabling the ruler.\n" );
   _ruler=0;
  }
  else
  if ( equals( a, "+clock" ) ) {
   printf( "fed: Enabling the clock.\n" );
   _clock=1;
  }
  else
  if ( equals( a, "-lang" ) ) {
   printf( "fed: Disabling syntax highlighting support.\n" );
   _lang=0;
  }
  else
  if ( equals( a, "+old" ) ) {
   printf( "fed: Enabling .old file auto-backup.\n" );
   _lang=0;
  }
  else
  if ( equals( a, "-plugins" ) ) {
   printf( "fed: Disabling loading and use of fed PHP plug-ins.\n" );
   _plugins=0;
  }
  else
  if ( equals( a, "-fed" ) ) {
   printf( "fed: Ignoring any available .fed configurations\n" );
   _fed=0;
  }
  else
  if ( equals( a, "-root" ) ) {
   printf( "fed: Ignoring root .fed configuration.\n" );
   _root=0;
  }
  else
  if ( equals( a, "+free" ) ) {
   printf( "fed: Enabling free move mode.\n" );
   free_mode=1;
  }
  else
  if ( equals( a, "+binary" ) ) {
   printf( "fed: Forcing binary mode for all files loaded by command line.\n" );
   _binary=1;
  }
  else
  if ( equals( a, "-clearscreen" ) ) {
   printf( "fed: Disabling the use of \"clear screen\" codes.\n" );
   _clearscreen=0;
  }
  else
  if ( equals( a, "-invert" ) ) {
   printf( "fed: Forcing light text on dark background.\n" );
   _invert=0;
  }
  else
  if ( equals( a, "+invert" ) ) {
   printf( "fed: Forcing dark text on light background.\n" );
   _invert=1;
  }
  else
  if ( equals( a, "-1" ) ) {
   printf( "fed: Enabling status bar mode 1.\n" );
   _statusbar=1;
  }
  else
  if ( equals( a, "-2" ) ) {
   printf( "fed: Enabling status bar mode 2.\n" );
   _statusbar=2;
  }
  else
  if ( prefix( a, "+tabsize" ) ) {
   if ( contains(a,'=') ) {
    while ( *a != '=' ) a++; a++;
    _tabspaces=atoi(a);
    if ( _tabspaces < 1 ) _tabspaces=1;
    printf( "fed: tab size set to %d spaces\n", _tabspaces );
   } else
    printf( "fed: +tabsize=value wasn't used properly.  Using setting: %d characters\n", _tabspaces );
  }
  else
  if ( prefix( a, "+undo" ) ) {
   if ( contains(a,'=') ) {
    while ( *a != '=' ) a++; a++;
    _undo=atoi(a);
    if ( _undo < 10 ) _undo=10;
    printf( "fed: maximum undo levels are %d\n", _undo );
   } else
    printf( "fed: +undo=value wasn't used properly.  Using setting: %d levels\n", _undo );
  } else
  if ( prefix( a, "+plugins" ) ) {
   if ( contains(a,'=') ) {
    while ( *a != '=' ) a++; a++;
    printf( "fed: Additional plugins folder set to '%s'\n", a );
    if ( plugins_folder ) free( plugins_folder );
    plugins_folder=str_dup(a);
   } else {
    printf( "fed: Error, +fedhome does not refer to a path.  Exitting.\n" );
    exit(1);
   }
  } else
  if ( prefix( a, "+fedhome" ) ) {
   if ( contains(a,'=') ) {
    while ( *a != '=' ) a++; a++;
    printf( "fed: Setting fed's home location to '%s'\n", a );
    if ( fed_home ) free(fed_home);
    fed_home=str_dup(a);
   } else {
    printf( "fed: Error, +fedhome does not refer to a path.  Exitting.\n" );
    exit(1);
   }
  } else
  if ( prefix( a, "+mintab" ) ) {
   if ( !contains(a,'=') ) {
    printf( "fed: +mintab=value wasn't used properly.  Using setting: %d characters\n", _mintab );
   } else {
    while ( *a != '=' ) a++; a++;
    _mintab=atoi(a);
    printf( "fed: Setting minimum multi-tab element size to %d.\n", _mintab );
   }
  } else
  if ( prefix( a, "+lang" ) ) {
   if ( !contains(a,'=') ) {
    printf( "fed: Enabling lang files located at %s\n", lang_home ? lang_home : "/etc/fed/lang" );
    _lang=1;
    load_langs();
   } else {
    while ( *a != '=' ) a++; a++;
    printf( "fed: Enabling lang syntax highlighting files in '%s'\n", a );
    lang_home=str_dup(a);
    _lang=1;
    load_langs();
   }
  }
  else if ( load_files ) { // Must be a filename.
   int load_mode=0; // 1=binary 0=ascii
   if ( !langs_loaded ) {
    load_langs();
   }
   if ( *(a+1) == '+' ) {
    switch ( LOWER(*a) ) {
     case 'b': load_mode=1; break;
     case 'a': load_mode=0; break;
    }
    if ( _binary ) load_mode=_binary;
    a++;a++;
   }
   printf( "fed: File '%s' being loaded as %s\n", a, (load_mode ? "binary" : "ascii") );
   files=prepend_fedfile( files, a, load_mode );
   if ( !active ) active=files;
  }
 }
 // Try to find a place to start editing
 active=files;
 if ( !active && load_files ) {
   printf( "fed: Creating an empty %s file.\n", (_binary ? "binary" : "ascii") );
   active=files=append_fedfile_new( files, "new", _binary );
 }
}

bool command_line_has( int argc, char **argv, char *test ) {
 int i;
 for ( i=0; i<argc; i++ ) if ( equals(argv[i],test) ) return true;
 return false;
}
