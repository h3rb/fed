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

/* This file contains graphics buffering routines for fed's gbuffer struct */

#include "fed.h"
#include "gbuffer.h"
#include "graphics.h"
#include "strings.h"
#include "color.h"
#include "cli.h"
#include "lang.h"

extern int ruler_y;
extern FEDFILE *active;

int show_eol_eof=0;

GBUFFER *new_gbuffer( int w, int h ) {
 int x,y;
 GBUFFER *new=malloc(sizeof(GBUFFER));
 new->content=malloc(sizeof(char)*w*h);
 new->colormap=malloc(sizeof(int)*w*h);
 for ( x=0; x<w; x++ ) for ( y=0; y<h; y++ ) { new->content[x+y*w]=' '; new->colormap[x+y*w]=7; }
 new->w=w;
 new->h=h;
 return new;
}

void free_gbuffer( GBUFFER *old ) {
 free(old->content);
 free(old->colormap);
 free(old);
}

GBUFFER *clone_gbuffer( GBUFFER *in, int copyit ) {
 GBUFFER *out=new_gbuffer( in->w, in->h );
 if ( copyit ) {
  int x,y;
  for ( x=0; x< in->w; x++ ) for ( y=0; y< in->h; y++ ) {
   out->content[x+y*out->w] = in->colormap[x+y*in->w];
   out->colormap[x+y*out->w] = in->colormap[x+y*in->w];
  }
 }
 return out;
}

void copy_region( GBUFFER *src, GBUFFER *dst, int sx, int sy, int w, int h ) {
 int x=sx, y=sy, x2=sx+w, y2=sy+h;
 if ( x<0 ) x=0;
 if ( y<0 ) y=0;
 if ( x2>= src->w ) x2=src->w;
 if ( x2>= dst->w ) x2=dst->w;
 if ( y2>= src->h ) y2=src->h;
 if ( y2>= dst->h ) y2=dst->h;
 for ( ; x<x2; x++ ) for ( ; y<y2; y++ ) {
  dst->content[x+y*w] = src->content[x+y*w];
  dst->colormap[x+y*w] = src->colormap[x+y*w];
 }
}

/*
 * Offset of the file display if not within the window
 */
extern int ruler_y;
extern int gbuffer_start;
void cursor_in_file( FEDFILE *file, int w, int h ) {
 int screenheight=h-gbuffer_start-1;
 int result;
 file->ofsx=( file->c > (w/2+w/4)     ? (file->c-w/2) : 0 );
 file->ofsy=( file->r < screenheight/2+screenheight/4 ? 0             : file->r-(screenheight/2) );
 file->cx=file->c - file->ofsx;
 file->cy=file->r - file->ofsy + gbuffer_start;
 if ( active == file && file->begin ) {
  file->sx=file->sx - file->ofsx;
  file->sy=file->sy - file->ofsy;
  file->ex=file->ex - file->ofsx;
  file->ey=file->ey - file->ofsy;
 }
 file->curc=char_at_pos( active->fileraw, file->ofsx+file->cx, file->ofsy+file->cy-gbuffer_start, &result );
 if ( !result ) { // we are beyond the end of the file...
 }
}

// Inserts a segment of a file into the gbuffer
GBUFFER *setup_gbuffer( FEDFILE *file, int w, int h ) {
 static int result1,result2;
 int x,y;
 GBUFFER *new;

 if ( !file || !file->fileraw ) return NULL;

 // Calculate changes to highlighting;
 if ( file->changed ) {
  if ( file->highlight ) free(file->highlight);
  file->highlight=colorize( file->fileraw, find_lang( file->lang ) );
  file->changed=false;
 }

 // Create a blank gbuffer
 new=new_gbuffer( w, h );
 // fill with grey ' '
 for ( x=0; x<w; x++ ) {
  for ( y=0; y<h; y++ ) {
   int location=x+y*w;
   new->content[location]=' ';
   new->colormap[location]=7;
  }
 }

 // Copy section of an ascii file
 if ( file->type == 0 ) {
  int showed_eof=0;
  cursor_in_file( file, w, h );
  for ( y=0; y<h; y++ ) {
   char *p=go_to_line_ofs( file->fileraw, file->ofsy+y, file->ofsx, &result1 );
   char *q=NULL;
   if ( file->highlight ) q=go_to_line_ofs( file->highlight, file->ofsy+y, file->ofsx, &result2 );
   /*if ( result1 )*/ {
    for ( x=0; x<w; x++ ) {
     if ( !p || *p=='\0' ) {
      if ( !showed_eof && show_eol_eof ) {
       showed_eof=1;
       new->content[x+y*w]='[';
       new->colormap[x+y*w]=23; x++;
       if ( x < w ) {
        new->content[x+y*w]='E';
        new->colormap[x+y*w]=23; x++;
       }
       if ( x < w ) {
        new->content[x+y*w]='O';
        new->colormap[x+y*w]=23; x++;
       }
       if ( x < w ) {
        new->content[x+y*w]='F';
        new->colormap[x+y*w]=23; x++;
       }
       if ( x < w ) {
        new->content[x+y*w]=']';
        new->colormap[x+y*w]=23; x++;
       }
      }
      while ( x < w ) {
       new->content[x+y*w]=' ';
       new->colormap[x+y*w]=7;
       x++;
      }
      break;
     } else
     if ( p && (*p=='\n' || *p == '\r') ) { // EOL
      if ( show_eol_eof ) {
       new->content[x+y*w]='[';
       new->colormap[x+y*w]=22;
       x++;
       if ( x < w ) {
        new->content[x+y*w]='E';
        new->colormap[x+y*w]=22;
        x++;
       }
       if ( x < w ) {
        new->content[x+y*w]='O';
        new->colormap[x+y*w]=22;
        x++;
       }
       if ( x < w ) {
        new->content[x+y*w]='L';
        new->colormap[x+y*w]=22;
        x++;
       }
       if ( x < w ) {
        new->content[x+y*w]=']';
        new->colormap[x+y*w]=22;
        x++;
       }
      }
      while ( x < w ) {
       new->content[x+y*w]=' ';
       new->colormap[x+y*w]=7;
       x++;
      }
      break;
     }
     else
     if ( p && *p=='\t' ) {
      new->content[x+y*w]=' ';
      new->colormap[x+y*w]=7;
     } else if (p) {
      new->content[x+y*w]=*p;
      if ( q ) new->colormap[x+y*w]=char_to_num(*q);
     }
     if ( p && *p != '\0' ) p++;
     if ( q && *q != '\0' ) q++;
    }
   }
  }
 }
 else // binary
 if ( file->type == 1 ) {
  //
 }
 return new;
}

// g = our previously rendered buffer memory, so we only render delta
// litho = what we intend to write
// clrscr == 1 uses clrscr
// mode == ANSI_MODE or "VT100" mode as a fallback
void render_gbuffer( GBUFFER *g, GBUFFER *litho, int cs, int term, int invert ) {
 int result;
 int x,y;
// curc=char_at_pos( active->fileraw, active->ofsx+cx, active->ofsy+cy-gbuffer_start, &result );
 switch ( term ) {
  case MODE_ANSI:
    {
      for ( x=0; x< litho->w; x++ ) for ( y=0; y< litho->h; y++ ) {
       int ox=active->ofsx+x;
       int oy=active->ofsy+y;
       if ( active->end && active->grid
            && ox>=UMIN(active->sx,active->ex) && oy>=UMIN(active->sy,active->ey)
            && ox<=UMAX(active->sx,active->ex) && oy<=UMAX(active->sy,active->ey)
          ) {
        attron(COLOR_PAIR(CYAN_ON_BLUE));
        mvprintw( y+gbuffer_start, x, "%c", litho->content[x+y*litho->w] );
        attroff(COLOR_PAIR(CYAN_ON_BLUE));
       } else
       if ( active->begin && !active->end && active->grid
            && ox>=UMIN(active->sx,active->cx+active->ofsx) && oy>=UMIN(active->sy,active->cy+active->ofsy-gbuffer_start)
            && ox<=UMAX(active->sx,active->cx+active->ofsx) && oy<=UMAX(active->sy,active->cy+active->ofsy-gbuffer_start)
          ) {
        attron(COLOR_PAIR(CYAN_ON_BLUE));
        mvprintw( y+gbuffer_start, x, "%c", litho->content[x+y*litho->w] );
        attroff(COLOR_PAIR(CYAN_ON_BLUE));
       } else
       if ( active->end && !active->grid ) {
        char *c=char_at_pos( active->fileraw, x+active->ofsx, y+active->ofsy, &result );
        if ( active->begin <= c && c <= active->end ) {
         attron(COLOR_PAIR(CYAN_ON_BLUE));
         mvprintw( y+gbuffer_start, x, "%c", litho->content[x+y*litho->w] );
         attroff(COLOR_PAIR(CYAN_ON_BLUE));
        } else
         mvprintw( y+gbuffer_start, x, "%c", litho->content[x+y*litho->w] );
       } else
       if ( active->begin && !active->grid ) {
        char *c=char_at_pos( active->fileraw, x+active->ofsx, y+active->ofsy, &result );
        char *e=char_at_pos( active->fileraw, active->cx+active->ofsx, active->cy+active->ofsy-gbuffer_start, &result );
        if ( UMIN(e,active->begin) <= c && c <= UMAX(e,active->begin) ) {
         attron(COLOR_PAIR(CYAN_ON_BLUE));
         mvprintw( y+gbuffer_start, x, "%c", litho->content[x+y*litho->w] );
         attroff(COLOR_PAIR(CYAN_ON_BLUE));
        } else
         mvprintw( y+gbuffer_start, x, "%c", litho->content[x+y*litho->w] );
       }
       else // not in selection
       {
        attron(COLOR_PAIR(litho->colormap[x+y*litho->w]));
        if ( litho->colormap[x+y*litho->w] == BOLD_RED
          || litho->colormap[x+y*litho->w] == BOLD_BLUE ) attron(A_BOLD);
        mvprintw( y+gbuffer_start, x, "%c", litho->content[x+y*litho->w] );
        if ( litho->colormap[x+y*litho->w] == BOLD_RED
          || litho->colormap[x+y*litho->w] == BOLD_BLUE ) attroff(A_BOLD);
        attroff(COLOR_PAIR(litho->colormap[x+y*litho->w]));
       }
      }
    }
   break;
  case MODE_ASCII:
  default: // "VT100" mode
    {
      // this is fallback mode, so we don't have gotoxy, and maybe not
      // even a clear picture of how big the terminal is
      // simply redraw a small selection of lines near the cursor
    }
   break;
 }

 copy_region( litho, g, 0, 0, litho->w, litho->h );
}

