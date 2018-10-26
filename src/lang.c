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
#include "lang.h"
#include "io.h"
#include "strings.h"
#include "cli.h"

/*
  Looks for a file in the language directory called "index.txt"
  Language file specification:
   insensitive (turns off case sensitivity)
   <symbol> <color#>
  Where color# corresponds to colors 0-9 in color.h
  If <symbol> is all alphabetic, it will match only when padded by non-alphabetic characters
  The following definitions should appear at the end of the file, in this order:
  If symbol is set to "", or '', or `` this indicates '<string>' coloring
  If the symbol is set to //, <!---->, #comment, ;comment, or / * * / (without spaces) , this indicates comment coloring
 */

LANG *langs=NULL;
bool langs_loaded=false;

LANG *find_lang( char *lang ) {
 LANG *search=langs;
 while ( search ) {
  if ( equals(lang,search->name) ) return search;
  search=search->next;
 }
 return NULL;
}

bool not_in_comment( char *location, char *code ) {
 char *p=code;
 int in_comment=0;
 int in_eol_comment=0;
 int in_squote=0, in_dquote=0;
 while ( *p != '\0' ) {
  if ( *p == '"' &&  (p==code || *(p-1) != '\\') && (!in_comment&& !in_eol_comment/*&& !in_squote*/) )
   { if ( !in_dquote ) in_dquote++; else in_dquote--; }
//  if ( *p == '\'' && (p==code || *(p-1) != '\\') && (!in_comment&& !in_eol_comment&& !in_dquote) )
//   { if ( !in_squote ) in_squote++; else in_squote--; }   use as apostrophe makes this worthless
  if ( *p == '/'  && *(p+1) == '*' )  { in_comment++;     }
  if ( *p == '/'  && *(p+1) == '/' )  { in_eol_comment++; }
  if ( *p == '#' )                    { in_eol_comment++; }
  if ( *p == '*'  && *(p+1) == '/' )  { in_comment--;     }
  if ( *p == '\n' && in_eol_comment ) { in_eol_comment=0; }
  if ( location==p ) return !(in_eol_comment || in_comment || in_dquote || in_squote);
  p++;
 }
 return true;
}

bool alphaisolated( char *key, char *position, char *code ) {
 bool leftok=true,rightok=true;
 int i;
 int len=strlen(key);
 bool iscompletelyalpha=true;
 for ( i=0; i<len; i++ ) if ( !char_isof(LOWER(key[i]),"abcdefghijklmnopqrstuvwxyz") ) iscompletelyalpha=false;
 if ( !iscompletelyalpha ) return true;
 if ( position==code ) leftok=true;
 else leftok= !char_isof(LOWER(*(position-1)),"abcdefghijklmnopqrstuvwxyz");
 if ( strlen(code) < (position-code+len) ) rightok=true;
 else rightok= !char_isof(LOWER(*(position+len)),"abcdefghijklmnopqrstuvwxyz");
 return (leftok && rightok);
}

char *colorize( char *code, LANG *lang ) {
 if ( !code ) return str_dup("7");
 int len=strlen(code);
 char *colored=malloc(sizeof(char)*(len+1));
 int i;
 for ( i=0; i<len; i++ ) if ( code[i] == '\n' ) colored[i]='\n'; else colored[i]='7';
 colored[len]='\0'; // end of file
 if ( !lang ) return colored;
 i=0;
 while ( i < lang->symbols ) {
  // Quotation rules
  if ( equals(lang->symbol[i],"''" ) ) {
   int idx=0;
   char *p=code;
   while ( *p != '\0' ) {
    if ( *p == '\'' && not_in_comment(p,code) ) {
     if ( idx != 0 && *(p-1) == '\\' ) { p++; idx++; }
     else {
      int total=0,j;
      p++; total++; // advance past starting '
      while ( *p != '\0' ) {
       if ( *p == '\'' && (*(p-1) != '\\') ) { p++; total++; break; }
       p++; total++;
      }
      for( j=0; j<total; j++ ) if ( colored[idx+j] != '\n' ) colored[idx+j]=num_to_char(lang->color[i]);
      idx+=total;
//      p++; idx++;
     }
    } else {
     p++;
     idx++;
    }
   }
  } else
  if ( equals(lang->symbol[i],"\"\"" ) ) {
   int idx=0;
   char *p=code;
   while ( *p != '\0' ) {
    if ( *p == '"' ) {
     if ( idx != 0 && *(p-1) == '\\' ) {
      p++; idx++;
     } else {
      int total=0,j;
      p++; total++; // advance past starting "
      while ( *p != '\0' ) {
       if ( *p == '"' && (idx==0 || *(p-1) != '\\') ) { p++; total++; break; }
       p++; total++;
      }
      for( j=0; j<total; j++ ) if ( colored[idx+j] != '\n' ) colored[idx+j]=num_to_char(lang->color[i]);
      idx+=total;
     }
    } else {
     p++;
     idx++;
    }
   }
  } else
  if ( equals(lang->symbol[i],"``" ) ) {
   int idx=0;
   char *p=code;
   while ( *p != '\0' ) {
    if ( *p == '`' ) {
     int total=0,j;
     p++; total++; // advance past starting `
     while ( *p != '\0' ) {
      if ( *p == '`' ) { p++; total++; break; }
      p++; total++;
     }
     for( j=0; j<total; j++ ) if ( colored[idx+j] != '\n' ) colored[idx+j]=num_to_char(lang->color[i]);
     idx+=total;
    } else {
     p++;
     idx++;
    }
   }
  } else
  // Comment rules
  if ( equals(lang->symbol[i],"/**/") ) {
   int idx=0;
   char *p=code;
   while ( *p != '\0' ) {
    if ( *p == '/' && *(p+1) == '*' ) {
     int total=0,j;
     while ( *p != '\0' && !(*p == '*' && *(p+1) == '/') ) {
      p++; total++;
     }
     if ( *p != '\0' ) { if ( *p == '*' ) { p++; total++; } if ( *p == '/' ) { p++; total++; } }
     for ( j=0; j<total; j++ ) if ( colored[idx+j] != '\n' ) colored[idx+j]=num_to_char(lang->color[i]);
     idx+=total;
    } else {
     p++; idx++;
    }
   }
  } else
  if ( equals(lang->symbol[i],"//" ) ) {
   int idx=0;
   char *p=code;
   while ( *p != '\0' ) {
    while ( *p != '\0' && !(*p == '/' && *(p+1) == '/') ) { p++; idx++; }
    if ( *p == '/' && *(p+1) == '/' ) {
     while ( *p != '\n' && *p != '\0' ) {
      if ( colored[idx] != '\n' ) colored[idx]=num_to_char(lang->color[i]);
      idx++;
      p++;
     }
    } else { p++; idx++; }
   }
  } else
  if ( equals(lang->symbol[i],"<!---->" ) ) {
   int idx=0;
   char *p=code;
   while ( *p != '\0' ) {
    if ( *p == '<' && *(p+1) == '!' && *(p+2) == '-' && *(p+3) == '-' ) {
     int total=0,j;
     while ( *p != '\0' && *p != '-' && *(p+1) != '-' && *(p+2) != '>' ) {
      p++; total++;
     }
     if ( *p != '\0' ) { while ( *p != '>' ) { p++; total++; } }
     for ( j=0; j<total; j++ ) if ( colored[idx+j] != '\n' ) colored[idx+j]=num_to_char(lang->color[i]);
     idx+=total;
    } else {
     p++; idx++;
    }
   }
  } else
  if ( equals(lang->symbol[i],"#comment" ) ) {
   int idx=0;
   char *p=code;
   while ( *p != '\0' ) {
    while ( *p != '\0' && *p != '#' ) { p++; idx++; }
    if ( *p == '#' ) {
     while ( *p != '\n' && *p != '\0' ) {
      if ( colored[idx] != '\n' ) colored[idx]=num_to_char(lang->color[i]);
      idx++;
      p++;
     }
    } else { p++; idx++; }
   }
  } else
  if ( equals(lang->symbol[i],";comment" ) ) {
   int idx=0;
   char *p=code;
   while ( *p != '\0' ) {
    while ( *p != '\0' && *p != ';' ) { p++; idx++; }
    if ( *p == ';' ) {
     while ( *p != '\n' && *p != '\0' ) {
      if ( colored[idx] != '\n' ) colored[idx]=num_to_char(lang->color[i]);
      idx++;
      p++;
     }
    } else { p++; idx++; }
   }
  }
  // Specific rules
  else {
   char *p=code;
   int idx=0;
   int slen=strlen(lang->symbol[i]);
   while ( *p != '\0' && idx < len-slen ) {
    char buf[slen+1];
    char *q=buf;
    int j;
    for ( j=0; j<slen; j++ ) {
     *q=*(p+j);
     q++;
    }
    *q='\0';
    if ( lang->case_sensitive ) {
     if ( cequals( buf, lang->symbol[i] ) && alphaisolated( buf, p, code ) )
      for ( j=0; j<slen; j++ ) colored[j+idx]=num_to_char(lang->color[i]);
    } else {
     if ( equals( buf, lang->symbol[i] ) && alphaisolated( buf, p, code ) )
      for ( j=0; j<slen; j++ ) colored[j+idx]=num_to_char(lang->color[i]);
    }
    idx++;
    p++;
   }
  }
  i++;
 }
 return colored;
}

void add_to_lang( LANG *lang, int color, char *symbol ) {
 char **symbols=malloc(sizeof(char *)*(lang->symbols+1));
 int *colors=malloc(sizeof(int *)*(lang->symbols+1));
 int i;
 for ( i=0; i<lang->symbols; i++ ) {
  symbols[i]=lang->symbol[i];
  colors[i]=lang->color[i];
 }
 symbols[lang->symbols]=str_dup( symbol );
 colors[lang->symbols]=color;
 lang->symbols++;
 free(lang->symbol);
 lang->symbol=symbols;
 free(lang->color);
 lang->color=colors;
}

#define NOT_END_OF_SEGMENT(c)  ( *c != ' ' && *c != '\n' && *c != '\r' && *c != '\0' )

void load_lang( char *filename, char *lang ) {
 char *q,*r,*s,*file;
 printf( " Loading language definition file '%s' defining '%s'\n", filename, lang );
 file=fread_ascii(filename);
 if ( file && strlen(file) > 0 ) {
  LANG *new=malloc(sizeof(LANG));
  new->symbols=0;
  new->symbol=NULL;
  new->color=NULL;
  new->name=str_dup(lang);
  new->case_sensitive=true;
  new->next=langs;
  langs=new;
  q=file;
  while ( *q != '\0' ) {
   char symbol[1024];
   char color[1024];
   r=symbol;
   s=color;
   if ( *q == '\n' ) q++;
   if ( *q == '\r' ) q++;
   while ( NOT_END_OF_SEGMENT(q) ) {
    *r=*q; r++; q++;
   }
   *r='\0';
   if ( *q == ' ' ) {
    int color_num=7;
    q++;
    while ( NOT_END_OF_SEGMENT(q) ) {
     *s=*q; s++; q++;
    }
    *s='\0';
    if ( equals(symbol,"insensitive") ) new->case_sensitive=false;
    else {
     if ( strlen(color) > 0 ) color_num=atoi(color);
     add_to_lang( new, color_num, symbol );
    }
   }
  }
 }
}

void load_langs() {
 char buf[2048];
 char *idx;
 // Create a default, empty language.
 LANG *new=malloc(sizeof(LANG));
 new->symbols=0;
 new->name=str_dup("default");
 new->symbol=NULL;
 new->color=NULL;
 new->next=NULL;
 new->case_sensitive=false;
 langs=new;
 // Load each language file listed in the index, if there is one.
 sprintf( buf, "%s/index.txt", lang_home ? lang_home : "/etc/fed/lang" );
 printf( "fed: Looking for language definitions in %s/index.txt\n", lang_home ? lang_home : "/etc/fed/lang" );
 idx=fread_ascii( buf );
 if ( idx && strlen(idx) > 0 ) {
  char fn[256];
  char filename[1024];
  char trimmed[256];
  char *p=idx;
  while ( *p != '\0' ) {
   char *q=fn;
   char *Q=trimmed;
   bool found_dot=false;
   while ( *p != '\0' && *p != '\n' ) {
    *q=*p; p++;
    if ( *q == '.' ) found_dot=true;
    if ( !found_dot ) { *Q=*q; Q++; }
    q++;
   }
   *Q='\0';
   *q='\0';
   if ( *p == '\n' ) p++;
   if ( strlen(fn) > 0 ) {
    sprintf( filename, "%s/%s", lang_home ? lang_home : "/etc/fed/lang", fn );
    load_lang( filename, trimmed );
   }
  }
 }
 langs_loaded=true;
}
