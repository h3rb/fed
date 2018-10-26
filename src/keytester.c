#include <ncurses.h>

int main( int argc, char **argv ) {
 int key;
 int line=0;
 initscr();
 raw();
 noecho();
 keypad(stdscr,TRUE);
 mvprintw( 10, 10, "ncurses key testing utility: Press 'Q' to quit." );
 while( (key=getch())!='Q') {
  mvprintw( line++,0, "Key pressed was: %3d", key );
  endwin();
  if ( key == 'p' ) system ( "php -f " );
  initscr();
  raw();
  noecho();
  keypad(stdscr,TRUE);
  refresh();
  if ( line > 25 ) { clear(); line=0;
   mvprintw( 10, 10, "ncurses key testing utility: Press 'q' to quit." );
  }
 }
 endwin();
 printf( "'q' was typed, so I exitted.\n" );
}
