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

/* This file contains file loading routines unique to fed */

#include "fed.h"
#include "errors.h"
#include "io.h"
#include "strings.h"
#include "cli.h"

FEDFILE *files=NULL;

int report_io=1;

extern int _eat_lf;

void sbeep() {
 printf( "\07" );
}

char char_by_num( int progress ) {
 switch ( progress ) {
  case 0: return '-';
  case 1: return '\\';
  case 2: return '|';
  case 3: return '/';
  case 4: return '-';
  case 5: return '\\';
  case 6: return '|';
  case 7: return '/';
 default: return '.';
 }
}

// Read an ascii file
char *fread_ascii( char *filepath ) {
 FILE *fp;
 struct stat st;
 stat(filepath, &st);
// printf( "%d bytes\r", (int) st.st_size );
 fp = fopen( filepath, "rb" );
 if ( !fp ) return str_dup( "" );
 if ( _eat_lf ) {
  int len=st.st_size+1,newlen;
  int i,j=0;
  int slashtabs=0;
  int reported=0;
  char reading=' ';
  char *readed=malloc(sizeof(char)*(len)),*p;
  char *detabbed;
  p=readed;
  while ( ( reading=fgetc(fp) ) != EOF && !feof(fp) ) if ( reading != '\r' ) {
   *p=reading;
   p++;
   if ( report_io ) { reported++; printf( "Loading %-10d bytes %c\r", reported, char_by_num((reported)%8) ); }
  }
  *p='\0';
  slashtabs=count_occurrances(readed,"\t");
  detabbed=malloc(sizeof(char)*(newlen=(len+slashtabs*_mintab)));
  for ( i=0; i<len; i++ ) {
   if ( readed && readed[i] == '\t' ) {
    int w;
    for ( w=0; w<_mintab; w++ ) detabbed[j+w]=' ';
    j+=_mintab;
   } else if ( readed ) {
    detabbed[j]=readed[i];
    j++;
   }
  }
  free(readed);
  return detabbed;
 } else
 {
  int len=st.st_size+1,newlen;
  int i,j=0;
  int slashtabs=0;
  char *readed=malloc(sizeof(char)*(len)),*p;
  char *detabbed;
  for ( i=0; i<len; i++ ) readed[i]=' ';
  readed[len-1]='\0';
#if defined(USE_FREAD)
  size_t read_in;
  read_in=fread(readed,sizeof(char),len-1,fp);
  readed[read_in]='\0';
  readed[len-1]='\0';
#else
  int reported=0;
  char reading=' ';
  p=readed;
  while ( ( reading=fgetc(fp) ) != EOF && !feof(fp) ) if ( reading != '\r' ) {
   *p=reading;
   p++;
   if ( report_io ) { reported++; printf( "Loading %-10d bytes %c\r", reported, char_by_num((reported)%8) ); }
  }
  *p='\0';
#endif
  slashtabs=count_occurrances(readed,"\t");
  detabbed=malloc(sizeof(char)*(newlen=(len+slashtabs*_mintab)));
  for ( i=0; i<len; i++ ) {
   if ( readed && readed[i] == '\t' ) {
    int w;
    for ( w=0; w<_mintab; w++ ) detabbed[j+w]=' ';
    j+=_mintab;
   } else if ( readed ) {
    detabbed[j]=readed[i];
    j++;
   }
  }
  free(readed);
  return detabbed;
 }
}

// Read a binary file
char *fread_binary( char *filepath ) {

 FILE *fp;
 char *s=NULL,*p=NULL;
 size_t r=0;
 char reading=' ';

 fp = fopen( filepath, "rb" );
 if ( !fp ) return NULL;
 while ( ( reading=fgetc(fp) ) != EOF && !feof(fp) ) {
  if ( !s ) {
   s=malloc( sizeof(char)*1 );
   *s=r;
   p=s;
   r=1;
  } else {
   s=realloc(s,++r);
   p++;
   *p=reading;
  }
 }

 s=realloc(s,++r);
 p++;
 *p='\0';

 return s;

}

char *extension( char *filename ) {
 char *p=filename;
 char *ext=str_dup("");
 while ( *p != '\0' && *p != '.' ) p++;
 if ( *p == '.' ) {
  p++;
  while ( *p != '\0' && *p != ' ' ) { char_append( &ext, *p ); p++; }
 }
 return ext;
}

FEDFILE *append_fedfile ( FEDFILE *list, char *filename, int mode ) {
 FEDFILE *new=malloc( sizeof(FEDFILE) );
 new->undos=new->redos=NULL;
 new->highlight=NULL;
 new->changed=true;
 new->modified=false;
 new->fileraw=( mode == 0 ? fread_ascii( filename ) : fread_binary( filename ) );
 if ( _old ) {
  char fn[2048];
  sprintf( fn, "%s.old", filename );
  write_ascii( new->fileraw, fn );
 }
 new->original=str_dup( new->fileraw );
 new->filename=str_dup( filename );
 new->lang=extension(filename);
 new->grid=0;
 new->begin=NULL;
 new->end=NULL;
 new->type=mode;
 new->inserting=1;
 new->r=new->c=new->cx=new->cy=new->sx=new->sy=new->ex=new->ey=new->ofsx=new->ofsy=0;
 new->curc=NULL;
 new->total_lines=(mode ? 1 : count_occurrances( new->fileraw, "\n" )); // "1" line for binary
 new->next=list;
 return list=new;
}

FEDFILE *prepend_fedfile ( FEDFILE *list, char *filename, int mode ) {
 FEDFILE *last;
 FEDFILE *new=malloc( sizeof(FEDFILE) );
 new->undos=new->redos=NULL;
 new->highlight=NULL;
 new->changed=true;
 new->modified=false;
 new->fileraw=( mode == 0 ? fread_ascii( filename ) : fread_binary( filename ) );
 if ( _old ) {
  char fn[2048];
  sprintf( fn, "%s.old", filename );
  write_ascii( new->fileraw, fn );
 }
 new->original=str_dup( new->fileraw );
 new->filename=str_dup( filename );
 new->lang=extension(filename);
 new->grid=0;
 new->begin=NULL;
 new->end=NULL;
 new->type=mode;
 new->inserting=1;
 new->r=new->c=new->cx=new->cy=new->sx=new->sy=new->ex=new->ey=new->ofsx=new->ofsy=0;
 new->curc=NULL;
 new->total_lines=(mode ? 1 : count_occurrances( new->fileraw, "\n" )); // "1" line for binary
 new->next=NULL;
 last=list; if ( last ) while ( last->next ) last=last->next;
 if ( last ) last->next=new;
 else list=new;
 return list;
}

// Does not load a file, merely creates one named "filename"
FEDFILE *append_fedfile_new ( FEDFILE *list, char *filename, int mode ) {
 FEDFILE *new=malloc( sizeof(FEDFILE) );
 new->undos=new->redos=NULL;
 new->highlight=NULL;
 new->changed=true;
 new->modified=false;
 new->fileraw= (mode == 0 ? str_dup( "" /* "a brave new world" */ ) : NULL);
 new->original=( mode == 0 ? fread_ascii( filename ) : fread_binary( filename ) );
 new->filename= str_dup(filename);
 new->lang=extension(filename);
 new->grid=0;
 new->begin=NULL;
 new->end=NULL;
 new->type=mode;
 new->inserting=1;
 new->r=new->c=new->cx=new->cy=new->sx=new->sy=new->ex=new->ey=new->ofsx=new->ofsy=0;
 new->curc=NULL;
 new->total_lines=0;
 new->next=list;
 return list=new;
}

int count_fedfiles( FEDFILE *feds ) {
 int count=0;
 for ( ; feds; feds=feds->next ) count++;
 return count;
}

int write_ascii( char *data, char *file ) {
 FILE *fp=fopen( file, "w" );
 if ( !fp ) { error_message( "Could not write file! %s\n", file ); return 0; }
 fprintf( fp, "%s", data );
 fclose(fp);
 return 1;
}


int write_binary( char *data, char *file ) {
 FILE *fp=fopen( file, "w" );
 if ( !fp ) { error_message( "Could not write file! %s\n", file ); return 0; }
 fprintf( fp, "%s", data );
 fclose(fp);
 return 1;
}

