#include <signal.h>

#include "fed.h"
#include "input.h"
#include "timer.h"
#include "strings.h"
#include "color.h"
#include "gbuffer.h"
#include "graphics.h"
#include "display.h"
#include "clipboard.h"
#include "undo.h"
#include "help.h"
#include "dashboard.h"
#include "cli.h"
#include "plugin.h"
#include "io.h"

#define KEY_CTRL_A 1
#define KEY_CTRL_B 2
#define KEY_CTRL_C 3
#define KEY_CTRL_D 4
#define KEY_CTRL_E 5
#define KEY_CTRL_F 6
#define KEY_CTRL_G 7
#define KEY_CTRL_H 8
#define KEY_CTRL_I 9
#define KEY_CTRL_J 10
#define KEY_CTRL_K 11 // could cause issues
#define KEY_LF     11 // on terms that send both CRLF
#define KEY_CTRL_L 12
#define KEY_CTRL_M 13
#define KEY_CTRL_N 14
#define KEY_CTRL_O 15
#define KEY_CTRL_P 16
#define KEY_CTRL_Q 17
#define KEY_CTRL_R 18
#define KEY_CTRL_S 19
#define KEY_CTRL_U 21
#define KEY_CTRL_V 22
#define KEY_CTRL_W 23
#define KEY_CTRL_X 24
#define KEY_CTRL_Y 25
#define KEY_CTRL_Z 26
#define KEY_CTRL_BACKSLASH 28
#define KEY_CTRL_RBRACKET 29
#define KEY_CTRL_6 30
#define KEY_CTRL_SLASH 31
#define KEY_F4 126
#define KEY_F10 274
#define KEY_F11 275
#define KEY_F12 276
#define KEY_NUMPAD_DEL 330 // unused for some reason
#define KEY_INS 331
//#define KEY_NUMPAD_INS 112 is also "P"

int key=0;
extern FEDFILE *files;
extern int gbuffer_start;
extern int show_clipboard;

// Meta-keys not handled by NCurses properly
int capturing=0;
int captured[5];

// Find / Replace feature
int quit_now=0;
int find_mode=0;
int finding=1;
int find_all_files=0;
int scrolled=0,num_selected=0;
int max_showable=0;
SEARCH_RESULT *results=NULL;
SEARCH_RESULT *selected_result=NULL;
char keywords[256];
char replacement[16384];

// Plug-in Menu
int plugin_menu=0;
int plugin_result_menu=0;

// Save requests
SAVE_REQUEST *saving=NULL;

// Heading toward exit, processing save requests...
int exitting=0;

// Export...
int export_mode=0;

// Free mode toggle
int free_mode=0;

void get_input( void ) {
 key=getch();
 if ( key == KEY_RESIZE ) {
  resized();
  display_resize();
  key=-1;
 }
}

void calculate_cursor_position( FEDFILE *file, char *position ) {
 int r=0,c=0;
 char *p=file->fileraw;
 while ( *p != '\0' && p != position ) {
  if ( *p == '\n' ) { r++; c=0; } else c++;
  p++;
 }
 file->r=r;
 file->c=c;
}

void find_file_position( FEDFILE *file, char *position, int *r, int *c ) {
 char *p=file->fileraw;
 *r=0; *c=0;
 while ( *p != '\0' && p != position ) {
  if ( *p == '\n' ) { (*r)++; *c=0; } else (*c)++;
  p++;
 }
}

void refind( void ) {
 SEARCH_RESULT *new,*next;
 FEDFILE *searching;
 for ( new=results; new; new=next ) {
  next=new->next;
  free(new);
 }
 results=NULL; next=NULL;
 selected_result=NULL;
 scrolled=num_selected=0;
 if ( strlen(keywords) <= 0 ) return;
 if ( find_all_files ) searching = files; else searching=active;
 while ( searching ) {
  char *p=searching->fileraw;
  while ( *p != '\0' ) {
   if ( *p == keywords[0] ) {
    char *q=keywords;
    char *r=p;
    bool matching=true;
    while ( *q != '\0' ) if ( *r == *q ) { r++; q++; } else { matching=false; break; }
    if ( matching ) {
     new=malloc(sizeof(SEARCH_RESULT));
     new->file=searching;
     new->position=p;
     new->touched=0;
     find_file_position( searching, p, &(new->line_no), &(new->col_no) );
     if ( !next )
     {
      new->next=results;
      results=new;
      next=new;
     } else {
      new->next=NULL;
      next->next=new;
      next=new;
     }
    }
   }
   p++;
  }
  if ( !find_all_files ) break;
  searching=searching->next;
 }
}

void render_find( void ) {
 char *menu=str_dup("");
 SEARCH_RESULT *r=results;
 int sc= ( active->cx >= col/2 ? 2 : col/2+2 );
 int showing=0;
 int scrolling=0;
 render_gbuffer( previous, current, _clearscreen, mode, _invert );
 max_showable=row-10;
 if ( selected_result && selected_result->file != active ) { selected_result=NULL; num_selected=0; }
 if ( results ) {
  char stripped[256];
  char buf[100];
  while ( r ) if ( r->file != active ) { r=r->next; } else {
   char *s=buf;
   if ( scrolling < scrolled ) { scrolling++; r=r->next; continue; }
   sprintf( stripped, "%s", fit(r->position,255) );
   s=stripped;
   while ( *s != '\0' ) { if ( *s == '\n' ) *s=' '; s++; }
   sprintf( buf, "^^I|%s%c%s%40s^^I%s%c%s|\n",
    r==selected_result ? "^^E" : "",
    r==selected_result ? '>'   : ' ',
    r==selected_result ? "^^J" : "^^D",
    fit(stripped,40),
    r==selected_result ? "^^E" : "^^I",
    r==selected_result ? '<'   : ' ',
    r==selected_result ? "^^I" : ""
    );
   s=buf;
   while ( *s != '\0' ) { char_append(&menu,*s); s++; }
   showing++;
   if ( showing >= max_showable ) break;
   r=r->next;
  }
 }
 attron(COLOR_PAIR(YELLOW_ON_MAGENTA));
 mvprintw( row-5-showing-3, sc, ".-----[Find / Replace]-------------[c-X]---." );
 mvprintw( row-5-showing-2, sc, "| c-N: Next c-S: Change Scope to %s |", find_all_files ? "This File" : "All Files" );
 mvprintw( row-5-showing-1, sc, "| c-R: Replace c-A: Replace All in Scope   |" );
 if ( strlen(menu) > 0 ) draw_color_coded( sc, row-5-showing, menu );
 attron(COLOR_PAIR(YELLOW_ON_MAGENTA));
 mvprintw( row-5, sc,                "|F?                                        |" );
 mvprintw( row-4, sc,                "|R>                                        |" );
 mvprintw( row-3, sc,                "'------------------------------------------'" );
 attroff(COLOR_PAIR(YELLOW_ON_MAGENTA));
 attron(COLOR_PAIR(finding ? GREEN_ON_BLUE : GREEN_ON_BLACK));
 mvprintw( row-5, sc+3, "%38s", trunc_fit( keywords, 38, 1, 0 ) );
 attroff(COLOR_PAIR(finding ? GREEN_ON_BLUE : GREEN_ON_BLACK));
 attron(COLOR_PAIR(!finding ? GREEN_ON_BLUE : GREEN_ON_BLACK));
 mvprintw( row-4, sc+3, "%38s", trunc_fit( replacement,  38, 1, 0 ) );
 attroff(COLOR_PAIR(!finding ? GREEN_ON_BLUE : GREEN_ON_BLACK));
 free(menu);
}

void replace_all( void ) {
}

void process_find_mode( void ) {
 switch ( key ) {
  case -1: break;
  case KEY_ENTER:
  case KEY_CTRL_M:
  case '\n':
   finding=!finding;
  break;
  case KEY_CTRL_P: case KEY_UP: case KEY_LEFT:
    if ( !results ) { selected_result=NULL; num_selected=0; scrolled=0; break; }
    if ( selected_result && selected_result->next != NULL && selected_result != results ) {
     SEARCH_RESULT *beiber=results;
     while ( beiber && beiber->next != selected_result ) beiber=beiber->next;
     selected_result=beiber;
     num_selected--;
     if ( num_selected-scrolled > max_showable ) scrolled=num_selected-2;
     active->curc=selected_result->position;
     active->c=selected_result->col_no;
     active->r=selected_result->line_no;
     active->begin=selected_result->position;
     active->sx=active->c;
     active->sy=active->r;
     active->end=selected_result->position+strlen(keywords)-1;
     active->ex=active->c+strlen(keywords)-1;
     active->ey=active->r;
     cursor_in_file( active, col, row );
     render_gbuffer( previous, current, _clearscreen, mode, _invert );
     render_find();
    } else if ( !selected_result || selected_result == results ) {
     selected_result=results;
     num_selected=1;
     scrolled=0;
     active->curc=selected_result->position;
     active->c=selected_result->col_no;
     active->r=selected_result->line_no;
     active->begin=selected_result->position;
     active->sx=active->c;
     active->sy=active->r;
     active->end=selected_result->position+strlen(keywords)-1;
     active->ex=active->c+strlen(keywords)-1;
     active->ey=active->r;
     cursor_in_file( active, col, row );
     render_gbuffer( previous, current, _clearscreen, mode, _invert );
     render_find();
    }
  break;
  case KEY_CTRL_N: case KEY_RIGHT: case KEY_DOWN:
    if ( !results ) { selected_result=NULL; num_selected=0; scrolled=0; break; }
    if ( !selected_result ) {
     selected_result=results;
     num_selected=1;
     scrolled=0;
     active->curc=selected_result->position;
     active->c=selected_result->col_no;
     active->r=selected_result->line_no;
     active->begin=selected_result->position;
     active->sx=active->c;
     active->sy=active->r;
     active->end=selected_result->position+strlen(keywords)-1;
     active->ex=active->c+strlen(keywords)-1;
     active->ey=active->r;
     cursor_in_file( active, col, row );
     render_gbuffer( previous, current, _clearscreen, mode, _invert );
     render_find();
    }
    else {
     selected_result=selected_result->next;
     if ( selected_result == NULL ) {
      selected_result=results;
      num_selected=0;
     }
     num_selected++;
     if ( num_selected-scrolled > max_showable ) scrolled=num_selected-2;
     active->curc=selected_result->position;
     active->c=selected_result->col_no;
     active->r=selected_result->line_no;
     active->begin=selected_result->position;
     active->sx=active->c;
     active->sy=active->r;
     active->end=selected_result->position+strlen(keywords)-1;
     active->ex=active->c+strlen(keywords)-1;
     active->ey=active->r;
     cursor_in_file( active, col, row );
     render_gbuffer( previous, current, _clearscreen, mode, _invert );
     render_find();
    }
   break;
  case KEY_CTRL_S:
   find_all_files=!find_all_files;
   refind();
  break;
  case KEY_CTRL_A:
   push_undo( active );
   replace_all();
   find_mode=0;
  break;
  case KEY_CTRL_Q:
  case KEY_CTRL_C:
  case KEY_CTRL_X:
  case 27:
   find_mode=0;
  break;
  case KEY_BACKSPACE:
  case KEY_CLEAR:
  case KEY_CTRL_H:
  case KEY_SDC:
  case KEY_DC:
   if ( strlen(keywords) > 0 )
   {
    sprintf( keywords, "%s", fit( keywords, strlen(keywords)-1 ) );
   }
   refind();
  break;
  default:
   if ( char_isof( (char) key, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()-=_+~`[]\\{}|;':\",./<>? " ) && strlen(keywords)<250 ) {
    static char buf[256];
    if ( finding ) {
     sprintf( buf, "%s%c", keywords, (char) key );
     sprintf( keywords, "%s", buf );
     refind();
    } else {
     sprintf( buf, "%s%c", replacement, (char) key );
     sprintf( replacement, "%s", buf );
    }
   }
  break;
 }
}


void draw_quit_now( void ) {
 draw_color_coded( 5, row-8, "^^B.-----------------." );
 draw_color_coded( 5, row-7, "^^B| ^^CQuit now?^^B ^^E*^^Dy^^E/^^Dn^^E*^^D^^B |" );
 draw_color_coded( 5, row-6, "^^B'-----------------'" );
}


void process_quit_now( void ) {
 switch( key ) {
  case 'n': case 'N': quit_now=0; break;
  case 'y': case 'Y': endwin(); exit(0); break;
 }
}


// rest is in plugin.c
void process_plugin_menu( void ) {
 switch( key ) {
  case KEY_DOWN: case KEY_LEFT: case KEY_CTRL_P:
   if ( !selected ) selected=plugins;
   else {
    selected=selected->next;
    if ( !selected ) selected=plugins;
   }
  break;
  case KEY_UP: case KEY_RIGHT: case KEY_CTRL_N:
   if ( !selected ) selected=plugins;
   else {
    PLUGIN *p=plugins;
    if ( selected->next == NULL ) selected=p;
    else { while ( p && p->next != selected ) p=p->next; selected=p; }
   }
  break;
  case KEY_ENTER:
  case KEY_CTRL_M:
  case '\n':
   if ( selected )
   {
    RESULT *r;
    char *data;
    int result_count=0;
    if ( active->begin ) {
     if ( active->end ) {
     } else {
     }
    } else if ( clip && strlen(clip)>0 ) data=str_dup(clip);
    else data=str_dup(active->fileraw);
    endwin();
    decode_plugin_output( process_plugin( selected, data ) );
    plugin_menu=0;
    r=plugin_results;
    while ( r ) { result_count++; r=r->next; }
    if ( result_count < 2 && plugin_results ) {
     if ( clip ) free(clip);
     if ( plugin_results->option
       && (plugin_results->value == NULL || strlen(plugin_results->value)==0) ) clip=str_dup(plugin_results->option);
     else if ( plugin_results->value ) clip=str_dup(plugin_results->value);
     else clip=NULL;
     if ( plugin_results->option ) free(plugin_results->option);
     if ( plugin_results->value ) free(plugin_results->value);
     free(plugin_results);
     plugin_results=NULL;
     show_clipboard=1;
     init();
     display_update();
    } else {
     plugin_result_menu=1;
     init();
     display_update();
     draw_plugin_result_menu();
    }
    refresh();
   }
  break;
  case KEY_CTRL_X: case KEY_CTRL_Q: case 27: plugin_menu = 0; break;
  default: break;
 }
}

// rest is in plugin.c
void process_plugin_result_menu( void ) {
 switch( key ) {
  case KEY_DOWN: case KEY_LEFT: case KEY_CTRL_P:
   if ( !result_selected ) result_selected=plugin_results;
   else {
    result_selected=result_selected->next;
    if ( !result_selected ) result_selected=plugin_results;
   }
  break;
  case KEY_UP: case KEY_RIGHT: case KEY_CTRL_N:
   if ( !result_selected ) result_selected=plugin_results;
   else {
    RESULT *r=plugin_results;
    if ( result_selected->next == NULL ) result_selected=r;
    else { while ( r && r->next != result_selected ) r=r->next; result_selected=r; }
   }
  break;
  case KEY_ENTER:
  case KEY_CTRL_M:
  case '\n':
   if ( result_selected )
   {
    if ( result_selected->value ) {
     if ( clip ) free(clip);
     clip=str_dup(result_selected->value);
    }
    plugin_result_menu=0;
    plugin_menu=0;
    show_clipboard=1;
    display_update();
   }
  break;
  case KEY_CTRL_C: case KEY_CTRL_X: case KEY_CTRL_Q: case 27: plugin_result_menu = plugin_menu = 0; break;
  default: break;
 }
}


void process_input( void ) {
/*
 attron(COLOR_PAIR(YELLOW_ON_BLUE));
 mvprintw( row-2,0, "Key: %d\n", key );
 attroff(COLOR_PAIR(YELLOW_ON_BLUE));
 */
 if ( quit_now ) {
  process_quit_now();
 } else
 if ( saving ) {
  process_save_confirm();
 } else
 if ( plugin_result_menu ) {
  process_plugin_result_menu();
 } else
 if ( plugin_menu ) {
  process_plugin_menu();
 } else
 if ( find_mode ) { // Find Mode handling
  process_find_mode();
 } else
 if ( capturing > 0 ) { // ANSI control code support not handled by NCurses
  if ( capturing < 5 ) {
   captured[capturing]=key;
   capturing++;
   if ( capturing == 3 ) {
    if ( captured[0] == 27 ) {
     if ( captured[1] == 79 ) {
      switch ( captured[2] ) {
       case 108: key='+'; capturing=0; act_on_active(); break; // plus
       case 110: key=KEY_DC; capturing=0; act_on_active(); break; // del
       case 112: key=KEY_INS; capturing=0; act_on_active(); break; // ins
       case 113: key=KEY_END; capturing=0; act_on_active(); break; // end
       case 115: key=KEY_NEXT; capturing=0; act_on_active(); break; // pgdn
       case 119: key=KEY_HOME; capturing=0; act_on_active(); break; // home
       case 121: key=KEY_PPAGE; capturing=0; act_on_active(); break; // pgup
      }
     }
    }
   } else if ( capturing == 4 ) {
    if ( captured[0] == 27 && captured[1] == 91 && captured[2] == 49 && captured[3] == 126 ) { // mid-board Home
     key=KEY_HOME; capturing=0; act_on_active();
    } else
    if ( captured[0] == 27 && captured[1] == 91 && captured[2] == 52 && captured[3] == 126 ) { // mid-board End
     key=KEY_END; capturing=0; act_on_active();
    }
   } else if ( capturing == 5 ) {
    if ( captured[0] == 27 && captured[1] == 91 && captured[2] == 49 ) {
     if ( captured[3] == 49 && captured[4] == 126 ) { // F1
      key=KEY_CTRL_SLASH; capturing=0; act_on_active();
     } else
     if ( captured[3] == 50 && captured[4] == 126 ) { // F2
      key=KEY_CTRL_L; capturing=0; act_on_active();
     } else
     if ( captured[3] == 51 && captured[4] == 126 ) { // F3
      key=KEY_CTRL_S; capturing=0; act_on_active();
     } else
     if ( captured[3] == 52 && captured[4] == 126 ) { // F4
      key=KEY_CTRL_Q; capturing=0; act_on_active();
     }
    }
   } else if ( capturing >= 6 ) capturing=0;
  } else capturing=0;
 } else
 act_on_active();
 refresh();
}

extern FEDFILE *active;

void create_save_confirm( FEDFILE *a ) {
 SAVE_REQUEST *new=malloc(sizeof(SAVE_REQUEST));
 new->file=a;
 new->name=str_dup(a->filename);
 new->enter_pressed=0;
 new->next=saving;
 saving=new;
}

void queue_save_files( void ) {
 FEDFILE *a=files;
 while ( a ) { if ( a->modified ) create_save_confirm( a ); a=a->next; }
}

void create_save_as_confirm( FEDFILE *a ) {
 SAVE_REQUEST *new=malloc(sizeof(SAVE_REQUEST));
 new->file=a;
 new->name=str_dup(a->filename);
 char_append(&(new->name), '.');
 char_append(&(new->name), 'n');
 char_append(&(new->name), 'e');
 char_append(&(new->name), 'w');
 new->enter_pressed=0;
 new->next=saving;
 saving=new;
}

void draw_save_confirm( void ) {
 if ( saving ) {
  char buf[256];
  draw_color_coded( 5, row-10, "^^J.--[^^A Saving ^^J]------------------------------------------------------------[^^Ac-X^^J]--." );
  draw_color_coded( 5, row-9,  "^^J| Save As ^^C[^^B                                                                   ^^D]^^J |" );
  draw_color_coded( 5, row-8,  "^^J| Enter: Save/Overwrite                            c-N: Not This File          ^^J |" );
  draw_color_coded( 5, row-7,  "^^J'-------------------------------------------------------------------------------'" );
  sprintf( buf, "^^E%60s", trunc_fit( saving->name, 60, 1, 0 ) );
  draw_color_coded( 5+12, row-9, buf );
 }
}

void process_save_confirm( void ) {
 char buf[16384];
 if ( !saving && exitting ) {
  clear();
  endwin();
  printf( "fed: Exitting.\n" );
  exit(0);
 }
 switch ( key ) {
  case KEY_CTRL_Q:
  case KEY_CTRL_X:
  case KEY_CTRL_C:
  case 27:
   exitting=0;
   {
    SAVE_REQUEST *a,*next;
    for ( a=saving; a; a=next ) {
     next=a->next;
     free(a->name);
     free(a);
    }
    saving=NULL;
   }
  break;
  case KEY_CTRL_N:
   {
     SAVE_REQUEST *a=saving;
     saving=saving->next;
     if ( saving ) {
      sprintf( buf, "Skipping file (will not be saved..!) '%s'", a->name );
      new_note( buf, 5 );
     } else {
      quit_now=1;
     }
     free(a->name);
     free(a);
   }
  break;
  case KEY_ENTER:
  case KEY_CTRL_M:
  case '\n':
   {
     SAVE_REQUEST *a=saving;
     saving=saving->next;
     write_ascii( a->file->fileraw, a->name );
     a->file->modified=0;
     if ( !cequals(a->name,a->file->filename) ) {
      free(a->file->filename);
      a->file->filename = str_dup(a->name);
     }
     sprintf( buf, "Wrote the file '%s'", a->name );
     new_note( buf, 5 );
     free(a->name);
     free(a);
     if ( !saving && exitting ) process_save_confirm();
   }
  break;
  case -1: break;
  case KEY_BACKSPACE:
  case KEY_CLEAR:
  case KEY_CTRL_H:
  case KEY_SDC:
  case KEY_DC:
   sprintf( buf, "%s", fit(saving->name, (saving->name ? strlen(saving->name) : 1)-1) );
   free(saving->name);
   saving->name=str_dup(buf);
  break;
  default:
   {
    char_append( &(saving->name), (char) key );
   }
  break;
 }
}

void act_on_active( void ) {
 if ( !capturing ) switch ( key ) { // "Specialty" Numpad buttons
  case 265: /* Num-Lock / Mouse Button */ return;
  case 266: key='/'; break; // numpad forward slash
  case 267: key='*'; break; // numpad asterisk
  case 268: key='-'; break; // numpad minus
 }
 if ( about_mode ) {
  if ( key!=-1 ) about_mode=0;
  return;
 } else
 if ( help_mode ) {
  if ( (char) key == 'a' || (char) key == 'A' ) { about_mode=1; return; }
  if ( key!=-1 ) { help_mode=0; return; }
 }
 if ( show_clipboard ) { if ( key!=-1 ) show_clipboard=0; }
 if ( !active ) exit(0); // no more files loaded...
 switch (key) {
  case -1: break;
 // Show Help
  case KEY_F(1):
  case KEY_CTRL_SLASH:
   help_mode=1;
  break;
 // Undo
  case KEY_CTRL_Z:
    undo(active);
   break;
 // Redo
  case KEY_CTRL_Y:
    redo(active);
   break;
 // Find
  case KEY_CTRL_F:
   if ( !find_mode ) {
    find_mode=1;
    finding=1;
    keywords[0]='\0';
    replacement[0]='\0';
   }
  break;
 // Directional Arrows
  case KEY_LEFT:// move_left( active );
    active->c--;
    if ( active->c < 0 ) {
     if ( active->r == 0 ) active->c=0;
     else {
      char *pos;
      int result;
      active->r--;
      active->c=0;
      pos=go_to_line( active->fileraw, active->r, &result );
      while ( *pos != '\0' && *pos != '\n' ) { pos++; active->c++; }
     }
    }
    cursor_in_file( active, col, row );
   break;
  case KEY_RIGHT:// move_right( active );
    if ( free_mode ) {
     active->c++;
    } else {
     int result;
     char *pos=go_to_line( active->fileraw, active->r, &result );
     int c=0;
     active->c++;
     while ( *pos != '\n' && *pos != '\0' ) { pos++; c++; }
     if ( *pos == '\n' && active->c >= c ) { active->r++; active->c=0; }
    }
    cursor_in_file( active, col, row );
   break;
  case KEY_UP:// move_up( active );
    active->r--;
    if ( active->r < 0 ) active->r=0;
    cursor_in_file( active, col, row );
   break;
  case KEY_DOWN:// move_down( active );
    {
      int lines=count_occurrances(active->fileraw,"\n");// + 1;
      active->r++;
      if ( !free_mode && active->r > lines ) active->r=lines;
      cursor_in_file( active, col, row );
    }
   break;
  /* Top of document */
  case KEY_A1:
  case KEY_HOME:
    new_note( "Home", 1 );
    active->c=0; active->r=0;
    cursor_in_file( active, col, row );
   break;
  /* Previous page */
  case KEY_A3:
  case KEY_PPAGE:
  case KEY_PREVIOUS:
    new_note( "Previous page", 1 );
    active->r-=row;
    if ( active->r < 0 ) active->r=0;
    cursor_in_file( active, col, row );
   break;
  /* End of line/doc */
  case KEY_C1:
  case KEY_EOS:
  case KEY_EOL:
  case KEY_END:
    if ( *(active->curc) == '\n' ) {
     new_note( "End of Document", 1 );
     active->r=count_occurrances( active->fileraw, "\n" ); //active->total_lines-1;
     cursor_in_file( active, col, row );
     if ( *active->curc != '\0' ) { // go to end of file
      char *p=active->fileraw;
      active->r=0; active->c=0;
      while ( *p != '\0' ) {
       if ( *p == '\n' ) { active->r++; active->c=0; }
       else active->c++;
       p++;
      }
      cursor_in_file( active, col, row );
     }
    } else {
     char *p=active->curc;
     while ( *p != '\n' && *p != '\0' ) {
      active->c++;
      p++;
     }
     cursor_in_file( active, col, row );
     new_note( "End of Line", 1 );
    }
   break;
  /* Next page */
  case KEY_C3:
  case KEY_NPAGE:
  case KEY_NEXT:
    new_note( "Next page", 1 );
    if ( active->r < count_occurrances( active->fileraw, "\n" ) ) active->r+=row;
   break;
  /* Load or Open a file */
//  case KEY_F(2):
  case KEY_OPEN:
  case KEY_CTRL_L:
   break;
  /* Insert */
  case KEY_SIC:
  case KEY_INS: // insert toggler
    if ( active->inserting ) active->inserting=0;
    else active->inserting=1;
   break;
  /* Quit + Save */
  case KEY_CTRL_Q:
    exitting=1;
    queue_save_files();
    if ( !saving ) process_save_confirm();
   break;
  /* Select All */
  case KEY_CTRL_A:
    {
     char *p=active->fileraw;
     active->begin=active->fileraw;
     active->sx=active->sy=0;
     if ( p ) {
      int r=0,c=0;
      while ( *p != '\0' ) {
       if ( *p=='\n' ) { r++; c=0; } else c++;
       p++;
      }
      active->end=p;
      active->ex=c;
      active->ey=r;
     }
    }
   break;
  /* Select or Mark Begin */
  case KEY_SELECT:
  case KEY_MARK:
  case KEY_CTRL_BACKSLASH:
  case KEY_CTRL_RBRACKET:
   active->grid= (key==KEY_CTRL_BACKSLASH);
   if ( active->begin ) {
    if ( active->end ) {
     new_note( "Deselecting", 1 );
     active->begin=active->end=NULL;
     active->sx=active->sy=active->ex=active->ey=0;
    } else {
     int result;
     new_note( "Locking selection", 1 );
     active->end=char_at_pos( active->fileraw, active->c, active->r, &result );
     if ( active->end < active->begin ) { char *temp=active->end; active->end=active->begin; active->begin=temp; }
     if ( active->c < active->sx ) { int temp=active->sx; active->sx=active->c; active->ex=temp; }
     else active->ex=active->c;
     if ( active->r < active->sy ) { int temp=active->sy; active->sy=active->r; active->ey=temp; }
     else active->ey=active->r;
    }
   } else { // new selection
     int result;
     new_note( "New selection", 1 );
     active->begin=char_at_pos( active->fileraw, active->c, active->r, &result );
     active->sx=active->c;
     active->sy=active->r;
     active->ex=active->ey=0;
   }
   break;
  /* Copy */
  case KEY_CTRL_C:
    if ( active->begin ) {
     if ( !active->end ) {
      int result;
      new_note( "Locking selection", 1 );
      active->end=char_at_pos( active->fileraw, active->c, active->r, &result );
      if ( active->end < active->begin ) { char *temp=active->end; active->end=active->begin; active->begin=temp; }
      if ( active->c < active->sx ) { int temp=active->sx; active->sx=active->c; active->ex=temp; }
      else active->ex=active->c;
      if ( active->r < active->sy ) { int temp=active->sy; active->sy=active->r; active->ey=temp; }
      else active->ey=active->r;
     }
     new_note( "Copying text", 1 );
     copy_area( &active->fileraw, active->begin, active->end, active->grid );
    }// else if ( clip ) { free(clip); clip=NULL; }
    else if ( clip ) {
     new_note( "Nothing selected to copy - clearing existing clipboard", 1 );
     free(clip); clip=NULL;
    } else new_note( "Nothing selected - nothing copied", 1 );
   break;
  /* Paste */
  case KEY_CTRL_V:
    if ( clip && strlen(clip)>0 ) {
     active->modified=1;
     push_undo( active );
     new_note( "Pasted text", 3 );
     insert_area( &active->fileraw, active->begin, active->end, active->grid, clip );
     active->changed=true;
    } else {
     new_note( "Nothing to paste", 1 );
    }
   break;
  /* Backspace */
  case KEY_BACKSPACE:
  case KEY_CTRL_H:
  case KEY_CLEAR:
    active->modified=1;
    push_undo( active );
    if ( active->curc == active->fileraw && !active->begin ) { beep(); } else
    if ( active->begin ) {
     new_note( "Cutting text", 1 );
     cut_area( &active->fileraw, active->begin, active->end, active->grid, 1 );
    }
    else {
     char temp=*(active->curc-1);
     if ( (temp == '\n' || temp == '\r') && active->r != 0 ) {
      int result;
      char *pos;
      active->r-=1;
      result=0;
      pos=go_to_line( active->fileraw, active->r, &result );
      while ( *pos != '\n' && *pos != '\0' ) { pos++; result++; }
      active->c=result-1;
     } else active->c--;
     backspace_char( &active->fileraw, active->curc );
     if ( active->c < 0 ) {
      if ( active->r == 0 ) active->c=0;
      else {
       char *pos;
       int result;
       active->r--;
       active->c=0;
       pos=go_to_line( active->fileraw, active->r, &result );
       while ( *pos != '\0' && *pos != '\n' ) { pos++; active->c++; }
      }
     }
     cursor_in_file( active, col, row );
    }
    active->changed=true;
   break;
  /* Delete */
  case KEY_SDC:
  case KEY_DC:
  case KEY_CTRL_X:
    active->modified=1;
    if ( active->begin ) {
     new_note( "Cutting text", 2 );
     push_undo( active );
     cut_area( &active->fileraw, active->begin, active->end, active->grid, 1 );
     cursor_in_file( active, col, row );
     active->changed=true;
    }
    else delete_char( &active->fileraw, active->curc );
   break;
  /* Delete line like nano */
  case KEY_SDL:
  case KEY_DL:
  case KEY_CTRL_K:
    if ( 1 ) { new_note( "Temporarily unavailable, sorry.", 3 ); } else
    {
    active->modified=1;
    new_note( "Deleting text and placing in deletion buffer", 2 );
    push_undo( active );
    append_delbuffer( &(active->fileraw), active->cy+active->ofsy-gbuffer_start );
    cursor_in_file( active, col, row );
    active->begin=NULL;
    active->changed=true;
    }
   break;
  /* Undelete from buffer like nano */
  case KEY_IL:
  case KEY_CTRL_U:
    if ( 1 ) { new_note( "Temporarily unavailable, sorry.", 3 ); } else
    {
    active->modified=1;
    new_note( "Un-deleting text and inserting at cursor position", 2 );
    push_undo( active );
    insert_delbuffer( &active->fileraw, active->begin, active->end );
    active->changed=true;
    }
   break;
  /* Suspend */
  case KEY_CTRL_6:
  case KEY_F10:
    curs_set(1);
    endwin();
    raise(SIGSTOP);
    init();
    refresh();
    new_note( "Returning from suspension...", 2 );
   break;
  /* Previous tab */
  case KEY_CTRL_P:
    {
     FEDFILE *a=files;
     if ( a != active ) {
      while ( a->next != active ) a=a->next;
      active=a;
     } else {
      while ( a->next !=NULL ) a=a->next;
      active=a;
     }
    }
    cursor_in_file( active, col, row );
    clear();
    display_update();
    refresh();
    new_note( "Previous tab", 1 );
   break;
  /* Next tab */
  case KEY_CTRL_O:
    if ( active->next ) active=active->next;
    else active=files;
    cursor_in_file( active, col, row );
    clear();
    display_update();
    refresh();
    new_note( "Next tab", 1 );
   break;
  /* Easter Egg */
#if defined(NVADERS_IN_FED)
  case KEY_F11:
   {
    char **b=malloc(sizeof(char *)*3);
    char buf0[25], buf1[25], buf2[25];
    sprintf( (b[0]=buf0), "nInvaders" );
    sprintf( (b[1]=buf1), "-l" );
    sprintf( (b[2]=buf2), "%d", (active->c%8)+1 );
    main_nVaders( 3, b );
    free(b);
   }
  break;
#endif
  /* Toggle "Free Move" Mode */
  case KEY_F12:
   {
    free_mode=!free_mode;
    new_note( free_mode ? "Free Movement Activated" : "Free Movement Deactivated", 3 );
   }
  break;
 /* Remove unnecessary EOL spaces */
  case KEY_CTRL_B:
    {
     char report[256];
     int total=0;
     bool found=true;
     char *pos=active->fileraw;
     active->modified=1;
     while ( found ) {
      found = false;
      while ( *pos != '\0' && ( !(*pos == ' ' && *(pos+1) == '\n') || !(*pos == ' ' && *(pos+1) == '\n') ) ) pos++;
      if ( *pos == ' ' && ( *(pos+1) == '\n' || *(pos+1) == '\0' ) ) {
       found =true; total++;
       delete_char( &active->fileraw, pos );
       pos=active->fileraw;
      }
     }
     cursor_in_file( active, col, row );
     sprintf( report, "%d excess end-of-line spaces removed", total );
     new_note( report, 5 );
    }
   break;
 /* Show EOL/EOF toggle */
 case KEY_CTRL_W: show_eol_eof=!show_eol_eof; break;
 /* Plugins */
 case KEY_CTRL_D:
   if ( active->begin ) {
    if ( !active->end ) {
     int result;
     new_note( "Locking selection", 1 );
     active->end=char_at_pos( active->fileraw, active->c, active->r, &result );
     if ( active->end < active->begin ) { char *temp=active->end; active->end=active->begin; active->begin=temp; }
     if ( active->c < active->sx ) { int temp=active->sx; active->sx=active->c; active->ex=temp; }
     else active->ex=active->c;
     if ( active->r < active->sy ) { int temp=active->sy; active->sy=active->r; active->ey=temp; }
     else active->ey=active->r;
    }
    new_note( "Copying text", 1 );
    copy_area( &active->fileraw, active->begin, active->end, active->grid );
   }
   open_plugin_menu();
  break;
 /* Save as... */
 case KEY_CTRL_S:
   create_save_confirm( active );
  break;
 /* Export as... */
 case KEY_CTRL_E:
   create_save_as_confirm( active );
  break;
 /* Read into file at position */
 case KEY_CTRL_R:
  break;
 /* Enter */
  case KEY_ENTER:
  case KEY_CTRL_M:
//  case KEY_LF:
  case '\n':
    {
     int result;
     if ( active->begin ) {
      //clear_area( &(active->fileraw), active->begin, active->end, active->grid );
      active->begin=active->end=NULL;
     }
     active->modified=1;
     push_undo( active );
     insert_char( &active->fileraw, active->curc, '\n' );
     active->c=0;
     active->r++;
     calculate_cursor_position( active, go_to_line( active->fileraw, active->r, &result ) );
     cursor_in_file( active, col, row );
     active->changed=true;
    }
   break;
  // escape character, capture signal.
  case 27:
    capturing=1;
    captured[0]=27;
    captured[1]=0;
    captured[2]=0;
    captured[3]=0;
    captured[4]=0;
   break;
 /* Key entry */
  default:
   if ( char_isof( (char) key, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()-=_+~`[]\\{}|;':\",./<>? " ) )
   {
    if ( active->begin ) {
     clear_area( &(active->fileraw), active->begin, active->end, active->grid );
     active->begin=NULL;
     cursor_in_file( active, col, row );
    }
    active->modified=1;
    push_undo( active );
    if ( !active->inserting ) {
     delete_char( &active->fileraw, active->curc );
     cursor_in_file( active, col, row );
    }
    insert_char( &active->fileraw, active->curc, (char) key );
    if ( active->inserting ) active->c++;
    cursor_in_file( active, col, row );
    active->changed=true;
    if ( delbuffer ) { free(delbuffer); delbuffer=NULL; }
   }
  break;
 }
 key=-1;
}
