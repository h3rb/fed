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
#include "clipboard.h"
#include "strings.h"
#include "color.h"
#include "dashboard.h"
#include "gbuffer.h"

extern int gbuffer_start;
int show_clipboard=0;
int clip_grid=0;
char *clip=NULL;
char *delbuffer=NULL;
extern FEDFILE *active;

bool find_constraint( char **content, char **start, char **end ) {
 if ( !(*start) ) {
  *start=active->curc;
 }
 if ( !(*end) ) {
  int result;
  *end = char_at_pos( *content, (active->ex=active->ofsx+active->cx), (active->ey=active->ofsy+active->cy-gbuffer_start), &result );
  if ( result ) {
  } else {
   if ( active->ex==active->sx && active->ey==active->sy ) { // "select" the character at this location
    if ( **start == '\0' ) *end=*start;
    else *end=*start+1;
    if ( **start == '\n' ) { active->ex=0; active->ey=active->sy+1; }
    else { active->ex=active->sx; active->ey=active->sy; }
   }
  }
 }
 if ( *end < *start ) {
  char *temp=*end;
  int tx,ty;
  *end=*start;
  *start=temp;
  tx=active->ex;  ty=active->ey;
  active->ex=active->sx;  active->ey=active->sy;
  active->sx=tx;  active->sy=ty;
  return true;
 }
 return false;
}

void copy_area    ( char **content, char *start, char *end, int use_grid  ) {
 static int result;
 char /**c,*/*t;
 char *buffer;
 int total=0;
 bool was_left;
 if ( !(*content) || !start ) return;
 was_left=find_constraint(content,&start,&end);
 t=start;
 while ( t!=end+1 ) { t++; total++; }
 buffer=malloc( sizeof(char)*(total+count_occurrances( start, "\n" )+1) );
 if ( clip ) free(clip);
 if ( use_grid ) {
  int colwidth=abs(active->ex-active->sx);
  int rowheight=abs(active->ey-active->sy);
  int i,j,k=0;
  for ( j=0; j<rowheight; j++ ) {
   i=0;
   while ( i<colwidth ) {
    char *p=char_at_pos( *content, UMIN(active->sx,active->ex)+i++,UMIN(active->sy,active->ey)+j, &result );
    if ( !result || *p == '\0' || *p == '\n' ) { buffer[k++]=' '; } else { buffer[k++]=*p; }
   }
   buffer[k++]='\n';
  }
  buffer[k]='\0';
  clip_grid=1;
 } else {
  char *p=start;
  int i=0;
  while ( p!=(end+1) ) { buffer[i++]=*p; p++; }
  buffer[i]='\0';
  clip_grid=0;
 }
 clip=buffer;
 show_clipboard=1;
 // Clear selection
 active->begin=NULL; active->end=NULL; active->grid=FALSE;
}

void insert_area ( char **content, char *start, char *end, int use_grid, char *ins ) {
 static int result;
 bool was_left=false;
 if ( !ins || !content || strlen(ins)==0 || strlen(*content)==0 ) return;
 if ( (clip_grid && ins==clip) && use_grid ) {
  new_note( "Temporarily unavailable, sorry.", 3 );
 }
 if ( !start ) {
  // insert at position (before that) of cursor
  start=char_at_pos( *content, (active->sx=active->ofsx+active->cx), (active->sy=active->ofsy+active->cy-gbuffer_start), &result );
  end=start;
  active->ex=active->sx; active->ey=active->sy;
  use_grid=clip_grid; // user may be pasting a grid, but has not selected one
 } else was_left=find_constraint(content,&start,&end);
 {
  char *buffer=malloc(sizeof(char)*(strlen(*content)+strlen(ins)+count_occurrances(ins,"\n")*UMIN(active->sx,active->ex)));
  char *p=buffer;
  char *q=*content;
  while ( q!=start ) {
   *p=*q;
   p++; q++;
  }
  // User has copied a grid area and is pasting into a grid area
  if ( (clip_grid && ins==clip) && use_grid ) {
   char *area_start, *area_end;
   int area_longest=0, first_line_offset=0;
   char **temp;
   int lines=0;                   // number of lines in the clip board
   //int grid_col=UMIN(active->sx,active->ex);      // beginning column of grid
   int grid_width=longest(ins);  // longest clipboard width
   int sel_grid_width=abs(active->ex-active->sx); // width of currently selected grid
   int i,j;
   char *s,*t;
   char *r=ins;
   // Determine y-size of clipped grid
   lines++; // first line
   while ( *r != '\0' ) {
    r++; if ( *r=='\n') lines++; // new line
   }
   lines++; // last line
   // Create a temporary buffer of size lines
   temp=malloc(sizeof(char *)*lines);
   // From start, walk back to the beginning of the line, counting the offset,
   // then count each line length for the target # of lines, calculating area_longest
   r=start;
   while ( *r != '\n' && r != *content ) { r--; first_line_offset++; }
   if ( *r == '\n' ) r++;
   area_start=r;
   i=0; j=0;
   area_longest=0;
   while ( i < lines ) {
    while ( *r != '\n' && *r != '\0' ) { j++; r++; }
    if ( j > area_longest ) area_longest=j;
    j=0; i++;
   }
   area_end=r;
   // Allocate enough to store each line + grid_width
   // Fill the temporary buffer with the existing text
   r=area_start;
   for ( i=0; i<lines; i++ ) {
    temp[i]=malloc(sizeof(char)*(area_longest+grid_width));
    s=temp[i];
    j=0;
    while ( *r != '\0' && *r != '\n' ) { // copy the line removing text to replace
     j++;
     if ( !(j >= first_line_offset && j<= first_line_offset+sel_grid_width) ) { *s=*r; s++; }
     r++;
    }
    *s='\0';
    s=temp[i];
   }
   // Pad any lines that are too short
   for ( i=0; i<lines; i++ ) if ( strlen(temp[i]) < first_line_offset ) {
    s=temp[i];
    j=0;
    while ( *s != '\0' ) { s++; j++; }
    for ( ; j < first_line_offset; j++ ) { *s=' '; s++; }
    *s='\0';
   }
   // Insert clipboard sections normalized to length grid_width
   r=ins;
   for ( i=0; i<lines; i++ ) {
    int len=(strlen(temp[i])-first_line_offset);
    int k;
    char remaining[len];
    s=temp[i];
    for ( j=0; j<first_line_offset; j++ ) s++; // advance to insertion point
    t=s;
    for ( k=0; k<len; k++ ) { remaining[k]=*t; t++; } // store remaining to end of line
    j=0;
    while ( *r != '\0' && *r != '\n' ) {
     *s=*r;
     r++;
     s++;
     j++;
    }
    for ( ; j<grid_width; j++ ) { // pad unevenly copied grid lines
     *s=' '; s++;
    }
    // Insert remaining to eol
    for ( j=0; j<len; j++ ) { *s=remaining[j]; s++; }
    *s='\0';
   }
   // Replace area_start to area_end with temp buffer, adding \n
   q=*content;
   p=buffer;
   while ( q != area_start ) { *p=*q; p++; q++; }
   for ( i=0; i<lines; i++ ) {
    s=temp[i];
    while ( *s != '\0' ) { *p=*s; p++; s++; }
    *p='\n';
    p++;
   }
   q=area_end;
   while ( *q!='\0' ) { *p=*q; p++; q++; }
   *p='\0';
   // De-Allocate temp buffer
   for ( i=0; i<lines; i++ ) free(temp[i]);
   free(temp);
  } else { // User is pasting into a non-grid area with a non-grid, or a grid
   char *r=ins;
   while ( q!=end ) { q++; }
   while ( *r != '\0' ) { *p=*r; p++; r++; }
   while ( *q!='\0' ) { *p=*q; p++; q++; }
   *p='\0';
  }
  free(*content);
  *content=buffer;
 }
}

void append_delbuffer ( char **content, int r ) {
 if ( !content || !(*content) ) return;
 {
  static int result;
  int len;
  char buffer[len=strlen(*content)];
  char *p=go_to_line( *content, r, &result ), *q=p;
  int i=0;
  while ( *q != '\n' && *q != '\0' ) { buffer[i++]=*q; q++; }
  buffer[i++]='\n'; if ( *q=='\n' ) q++; // ignore trailing \n
  buffer[i]='\0';
  {
   int newlen;
   char new[(newlen=(strlen(*content)-strlen(buffer)+1))];
   char *s=*content;
   int j=0;
   while ( s!=p ) {     new[j++]=*s; s++; }
   s=q;
   while ( *s!='\0' ) { new[j++]=*s; s++; }
   new[j]='\0';
   free(*content);
   *content=str_dup(new);
  }
  if ( !delbuffer ) delbuffer = str_dup( buffer );
  else
  {
   int append=strlen(buffer);
   int newlen;
   char new[(newlen=(append+strlen(delbuffer)+1))];
   int j=strlen(delbuffer); i=0;
   sprintf( new, "%s", delbuffer );
   while ( i < append ) new[j++]=buffer[i++];
   new[j]='\0';
   free(delbuffer);
   delbuffer=str_dup(new);
  }
 }
}

void insert_delbuffer ( char **content, char *start, char *end ) {
 char *p=delbuffer;
 if ( !p ) {
  new_note( "Deletion buffer is empty, use Ctrl-K to fill with deleted lines", 3 );
  return;
 }
 if ( start ) {
  bool was_left; was_left=find_constraint(content,&start,&end);
  clear_area( content, start, end, active->grid );
  cursor_in_file( active, col, row );
  while ( *p != '\0' ) { insert_char( content, active->curc, *p ); active->curc++; p++; }
 } else {
  while ( *p != '\0' ) {
   cursor_in_file( active, col, row );
   insert_char( content, active->curc, *p );
   if ( *p=='\n' ) { active->c=0; active->r++; } else active->c++;
   cursor_in_file( active, col, row );
   p++;
  }
 }
}

void clear_delbuffer  ( void ) {
 if ( delbuffer ) { free(delbuffer); delbuffer=NULL; }
}

extern int has_color;
void draw_clipboard ( void ) {
 if (!clip) { show_clipboard=0; return; }
 {
  char buf[1024];
  int i,j;
  int drawy=3,drawx=active->cx+2;
  int width=longest(clip)+6;
  int height=row/2, lines=count_occurrances(clip,"\n");
  int result;
  if ( has_color ) attron(COLOR_PAIR(YELLOW_ON_RED));
  if ( drawx > col/2 ) drawx=2;
  if ( width > col-drawx ) width=col-drawx-1;
  if ( height > lines+2 ) height = lines+2;
  for (i=0; i<width; i++ ) buf[i]='-';
  buf[0]='.';
  buf[i++]='.';
  buf[i]='\0';
  mvprintw( drawy, drawx, "%s", buf );
  if ( width/3 >= 14 ) mvprintw( drawy, drawx+width/3, "clipboard" );
  else
  if ( width/3 >= 4 ) mvprintw( drawy, drawx+width/3, "%s", fit("clipboard",width/3) );
  buf[0]='L';
  buf[i-1]='\'';
  buf[i]='\0';
  mvprintw( drawy+height+1, drawx, "%s", buf );
  for (j=0; j<height; j++ ) {
   char *cline=go_to_line( clip, j, &result );
   mvprintw( drawy+j+1, drawx, "| " );
   mvprintw( drawy+j+1, drawx+width-1, " |" );
   for (i=0; i<=width-2-1; i++ )
    if ( *cline != '\n' && *cline !='\0' ) {
     mvprintw( drawy+j+1, drawx+2+i, "%c", *cline );
     cline++;
    }
    else mvprintw( drawy+j+1, drawx+2+i, " " );
  }
  if ( has_color) attroff(COLOR_PAIR(YELLOW_ON_RED));
 }
}

void cut_area    ( char **content, char *start, char *end, int use_grid, int show_clip  ) {
 bool was_left;
 show_clipboard=show_clip;
 if ( clip ) free(clip);
 clip=str_dup("");
 if ( !start || !content || !clip || strlen(*content)==0 || strlen(clip)==0 ) return;
 was_left=find_constraint( content, &start, &end );
 if ( use_grid ) { // cut a grid selection
  char *buffer=malloc( sizeof(char)*(strlen(*content)+1) );
  char *p=buffer;
  char *q=*content;
  char *r=clip;
  int sel_grid_width=abs(active->sx-active->ex);
  int sel_grid_height=abs(active->sy-active->ey);
  int i;
  int line=0;
  int left=UMIN(active->sx,active->ex);
  if ( !clip ) r=clip=str_dup("");
  while ( *q != '\0' && q!= start ) { *p=*q; p++; q++; }
  for ( line=0; line<sel_grid_height; line++ ) {
   for ( i=0; i<sel_grid_width; i++ ) {
    if ( *q=='\n' ) break;
    if ( *q != '\0' ) { q++; char_append(&clip,*q); }
   }
   char_append(&clip,'\n');
   while ( *q != '\n' && *q != '\0' ) { *p=*q; p++; q++; } // grab to eol
   if ( *q=='\n' ) { *p='\n'; q++; p++; }
   for ( i=0; i<left; i++ ) { // advances to leftmost column in area of grid selection
    if ( *q == '\n' ) break;
    else if ( *q != '\0' ) q++;
   }
  }
  while ( *q != '\0' ) { *p=*q; p++; q++; }
  *p='\0';
  free(*content);
  *content=str_dup(buffer);
  free(buffer); // gets rid of "extra" memory
  clip_grid=1;
 } else { // not a grid selection
  char *buffer=malloc( sizeof(char)*(strlen(*content)+1) );
  char *p=buffer;
  char *q=*content;
  while ( *q != '\0' && q!= start ) { *p=*q;  p++; q++; }
  while ( q!= end && *q!='\0' ) { char_append(&clip, *q); q++; }
  while ( *q!= '\0') { *p=*q; p++; q++; }
  *p='\0';
  free(*content);
  *content=str_dup(buffer);
  free(buffer); // gets rid of "extra" memory
  clip_grid=0;
 }
}


void clear_area    ( char **content, char *start, char *end, int use_grid  ) {
 char *oldclip=clip;
 clip=NULL;
 cut_area( content, start, end, use_grid, 0 );
 free(clip);
 clip=oldclip;
}
