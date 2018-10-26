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
#include "config.h"
#include "errors.h"
#include "display.h"
#include "input.h"
#include "timer.h"
#include "color.h"
#include "lang.h"
#include "io.h"
#include "plugin.h"
#include "dashboard.h"

int has_color=0;
int go;

struct winsize old_window_size;
struct winsize window_size;
int row,col; // ncurses

void resized( void ) {
 get_window_size();
 endwin();
 init();
 display_update();
 refresh();
}

void get_window_size( void ) {
 old_window_size=window_size;
 ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size);
 getmaxyx(stdscr,row,col);
}

void init(void) {
 ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size);
 printf( "fed: Window size is %d by %d.\n", window_size.ws_row, window_size.ws_col );
 getmaxyx(stdscr,row,col);
 initscr(); // ncurses - initialize
 raw();
 noecho();  // ncurses - disable echoing
 keypad(stdscr, TRUE); // ncurses - enable the keypad
 meta(stdscr,TRUE);
 curs_set(0);
 timeout(100);
 if ( (has_color=has_colors()) ) {
  start_color();
  init_pair(RED,               COLOR_RED,     COLOR_BLACK);
  init_pair(GREEN,             COLOR_GREEN,   COLOR_BLACK);
  init_pair(YELLOW,            COLOR_YELLOW,  COLOR_BLACK);
  init_pair(BLUE,              COLOR_BLUE,    COLOR_BLACK);
  init_pair(CYAN,              COLOR_CYAN,    COLOR_BLACK);
  init_pair(MAGENTA,           COLOR_MAGENTA, COLOR_BLACK);
  init_pair(WHITE,             COLOR_WHITE,   COLOR_BLACK);
  init_pair(BOLD_BLUE,         COLOR_BLUE,  COLOR_BLACK);
  init_pair(BOLD_RED,          COLOR_RED,   COLOR_BLACK);
  init_pair(BLACK_ON_WHITE,    COLOR_BLACK,   COLOR_WHITE);
  init_pair(WHITE_ON_BLUE,     COLOR_WHITE,   COLOR_BLUE);
  init_pair(CYAN_ON_BLUE,      COLOR_CYAN,    COLOR_BLUE);
  init_pair(YELLOW_ON_RED,     COLOR_YELLOW,  COLOR_RED);
  init_pair(GREY_ON_BLACK,     COLOR_WHITE,   COLOR_BLACK);
  init_pair(BLACK_ON_RED,      COLOR_BLACK,   COLOR_RED);
  init_pair(RED_ON_WHITE,      COLOR_RED,     COLOR_WHITE);
  init_pair(YELLOW_ON_BLUE,    COLOR_YELLOW,  COLOR_BLUE);
  init_pair(RED_ON_BLUE,       COLOR_RED,     COLOR_BLUE);
  init_pair(YELLOW_ON_MAGENTA, COLOR_YELLOW,  COLOR_MAGENTA);
  init_pair(BLUE_ON_RED,       COLOR_BLUE,    COLOR_RED);
  init_pair(BLACK_ON_BLUE,     COLOR_BLACK,   COLOR_BLUE);
  init_pair(YELLOW_ON_GREEN,   COLOR_YELLOW,  COLOR_GREEN);
  init_pair(BLACK_ON_GREEN,    COLOR_BLACK,   COLOR_GREEN);
  init_pair(BLUE_ON_WHITE,     COLOR_BLUE,    COLOR_WHITE);
  init_pair(GREEN_ON_BLUE,     COLOR_GREEN,   COLOR_BLUE);
  init_pair(RED_ON_GREEN,      COLOR_RED,     COLOR_GREEN);
 }
}

void loop(void) {
 go=1;
 get_window_size();
 while ( go ) {
  update_timer();
  display_update();
  refresh(); // ncurses - refresh the display
  get_input();
  process_input();
 }
}

char *gnu_getcwd (void) {
 static char buffer[1000];
 while (1) {
  if (getcwd (buffer, 1000) == buffer) return buffer;
  if (errno != ERANGE) return 0;
 }
}

char *cwd=NULL;

int main( int argc, char **argv )
{
   init_timer();
   cwd=gnu_getcwd();
   load_config( !command_line_has( argc, argv, "-fed" ), !command_line_has( argc, argv, "-root" ) );
   command_line( argc, argv, 1 );
   if ( _plugins ) {
    read_plugin_folder( "/etc/fed/plugins", 1 );
    if ( plugins_folder != NULL ) read_plugin_folder( plugins_folder, 1 );
   }
   report_io=false;

   init_error_handler();
   init();
   new_note( "Press Ctrl-? or F1 for help.", 3 );
   loop(); // our loop()
   curs_set(1);
   endwin();  // ncurses - deinitialize (must be called at all post-initscr() exit points)
   return 0;
}
