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
#include "strings.h"

int from_last_eol( char *start, char *position ) {
 char *a=position;
 int count=0;
 while ( a > start && *a != '\n' ) { a--; count++; }
 return count;
}

void backspace_char( char **in, char *at ) {
 if ( !in || !(*in) || !at ) return;
 {
  char *new = malloc(sizeof(char)*strlen(*in));
  char *p=*in;
  char *q=new;
  while ( *p != '\0' ) {
   if ( (p+1) != at ) { *q=*p; q++; }
   p++;
  }
  *q='\0';
  free(*in);
  *in=new;
 }
}

void delete_char( char **in, char *at ) {
 if ( !in || !(*in) || !at ) return;
 {
  char *new = malloc(sizeof(char)*strlen(*in));
  char *p=*in;
  char *q=new;
  while ( *p != '\0' ) {
   if ( p != at ) { *q=*p; q++; }
   p++;
  }
  *q='\0';
  free(*in);
  *in=new;
 }
}

void insert_char ( char **in, char *at, char c ) {
 if ( !in ) return;
 if ( !(*in) ) return;
 if ( !at ) return;
 if ( *at == '\0' ) {
  char_append( in, c ); return;
 }
 {
  char *new = malloc(sizeof(char)*strlen(*in)+2);
  char *p=*in;
  char *q=new;
  while ( *p != '\0' ) {
   if ( p==at ) { *q=c; q++; *q=*p; q++; p++; }
   else { *q=*p; p++; q++; }
  }
  *q='\0';
  free(*in);
  *in=new;
 }
}

int longest( char *c ) {
 char *p;
 int clength=0;
 int glength=0;
 p=c;
 if ( *p=='\n' ) {
  if ( clength > glength ) glength=clength;
  clength=0;
 }
 while ( *p != '\0' ) {
  p++;
  if ( *p=='\n' || *p=='\0' ) {
   if ( clength > glength ) glength=clength;
   clength=0;
  } else clength++;
 }
 return glength;
}

bool char_isof( char c, char *list ) {
  while ( *list != '\0' ) if ( c==*list ) return TRUE; else list++;
  return FALSE;
}

char *char_at_pos( char *content, int x, int y, int *overall_result ) {
 int result;
 char *p=go_to_line( content, y, &result );
 while ( x > 0 && *p!='\0' && *p!='\n' ) { x--; p++; }
 *overall_result=result || (*p == '\n' || *p == '\0' ); // outside editable area
 return p;
}

int eolcol_line( char *content, int linenumber ) {
 int eol=0,line=0;
 char *p=content;
 while ( *p != '\0' && line < linenumber ) { p++; if ( *p == '\n' ) line++; }
 while ( *p != '\0' && *p != '\n' ) { eol++; p++; }
 return eol;
}

char *go_to_line( char *content, int linenumber, int *result ) {
 char *p=content;
 int line=0;
 while ( p && *p != '\0' && line < linenumber ) { if ( *p == '\n' ) line++; p++; }
 *result = (line == linenumber);
 return p;
}

char *go_to_line_ofs( char *content, int linenumber, int c, int *result ) {
 char *p=content;
 int line=0;
 while ( p && *p != '\0' && line < linenumber ) { if ( *p == '\n' ) line++; p++; }
 if ( p && *p != '\0' ) {
  int ofs=0;
  while ( *p != '\0' && *p != '\n' && ofs < c ) { ofs++; p++; }
  *result= ( ofs == c );
 } else *result=0;
 return p;
}

char *trunc_pad( char *argument, int length ) {
 static char buf[1024];
 int len=strlen(argument);
 if ( len >= length ) sprintf( buf, "%s", argument );
 else {
  int add=length-len;
  int i;
  char *p=argument;
  for ( i=0; i<length; i++ ) { buf[i]=*p; p++; }
  for ( i=0; i<add; i++ ) buf[i+(length)]=' ';
  buf[add+length]='\0';
 }
 return buf;
}

char *trunc_fit( char *argument, int length, int from_left, int dot_dot_dot )
{
    static char buf[1024];
    int x,len;

    buf[0]='\0';
    if ( length>1023 || argument == NULL ) return buf;
    for ( x=0; x<1024; x++ ) buf[x]=' ';
    buf[1023]='\0';
    len=strlen(argument);

    if ( from_left ) { // ...some characters
     int left=length-len;
     char *p;
     if ( left > 0 ) left=0;
     else left=abs(left)+1;
     p=argument; p+=left;
     for ( x=0; ( x<length ); x++ ) if ( *p != '\0' ) {
      buf[x]=*p; p++;
     }
     buf[length-1]='\0';
     if ( len>length && dot_dot_dot ) {
      buf[0]='.';
      buf[1]='.';
      buf[2]='.';
     }
     buf[length-1]='\0';
    } else {
     char *p=argument;
     for ( x = 0;  (x < length);  x++ ) if ( *p != '\0' ) {
       buf[x] = *p; p++;
     }
     if ( len>length && dot_dot_dot ) {
      buf[length]='\0';
      buf[length-1]='.';
      buf[length-2]='.';
      buf[length-3]='.';
     } else buf[length] = '\0';
    }
    return buf;
}

char *fit( char *argument, int length )
{
    static char buf[1024];
    char *p=argument;
    int x;
    if ( length == 0 || length > 1023 ) { buf[0]='\0'; return buf; }
    if ( argument == NULL ) return argument;
    for ( x=0; x<1024; x++ ) buf[x]=' ';
    for ( x = 0;  (x < length);  x++ ) if ( *p != '\0' ) {
        buf[x] = *p; p++;
    }
    buf[length] = '\0';
    return buf;
}

bool contains( char *a, char c )
{
    char *p;
    p=a;
    while ( *p != '\0' ) if ( *(p++) == c ) return true;
    return false;
}

char *skip_spaces( char *a ) {
 while ( *a != '\0' && *a == ' ' ) a++;
 return a;
}

bool equals( char *a, char *b ) {
    if ( a == NULL || b == NULL ) return true;
    for ( ; *a || *b; a++, b++ ) if ( LOWER(*a) != LOWER(*b) ) return false;
    return true;
}

bool cequals( char *a, char *b ) {
    if ( a == NULL || b == NULL ) return true;
    for ( ; *a || *b; a++, b++ ) if ( *a != *b ) return false;
    return true;
}

bool prefix( char *a, char *b )
{
 if ( a == NULL || b == NULL ) return false;
 {
  char lowered_a[strlen(a)+1];
  char lowered_b[strlen(b)+1];
  char *p;
  int i;
  i=0; p=a; while ( *p != '\0' ) { lowered_a[i++]=LOWER(*p); p++; } lowered_a[i]='\0';
  i=0; p=b; while ( *p != '\0' ) { lowered_b[i++]=LOWER(*p); p++; } lowered_b[i]='\0';
  p=strstr(lowered_a,lowered_b);
  if ( p==lowered_a ) return true;
  return false;
 }
}

bool inside( char *a, char *b ) {
 if ( a == NULL || b == NULL ) return false;
 {
  char lowered_a[strlen(a)+1];
  char lowered_b[strlen(b)+1];
  char *p;
  int i;
  i=0; p=a; while ( *p != '\0' ) { lowered_a[i++]=LOWER(*p); p++; } lowered_a[i]='\0';
  i=0; p=b; while ( *p != '\0' ) { lowered_b[i++]=LOWER(*p); p++; } lowered_b[i]='\0';
  p=strstr(lowered_a,lowered_b);
  if ( p==NULL ) return false;
  return true;
 }
}

int words( char *argument )
{
    int total;
    char *s;
    total = 0;
    s = argument;
    while ( *s != '\0' )
    {
        while ( *s == '\n' || *s == '\r' || *s == ' ' ) s++;
        if ( *s != ' ' )
        {
            total++;
            while ( *s != ' ' && *s != '\0' && *s != '\n' && *s != '\r' ) s++;
        }
        else s++;
    }
    return total;
}

char **wordlist( char *in, int *count ) {
 char **a;
 char *s=in;
 int current;
 if ( !in || strlen(in)<=0 ) { count=0; return NULL; }
 *count=words(in);
 if ( *count == 0 ) return NULL;
 a=malloc(sizeof(char *)*(*count));
 for ( current=0; current < *count; current++ ) a[current]=str_dup("");
 current=0;
 while ( *s != '\0' && current < *count ) {
  while ( *s == '\n' || *s == '\r' || *s == ' ' ) s++;
  if ( *s != '\0' ) {
   while ( *s != ' ' && *s != '\0' && *s != '\n' && *s != '\r' ) {
    char_append( &(a[current]), *s );
    s++;
   }
   current++;
  }
 }
 return a;
}

void char_append( char **s, char c ) {
 if ( !s ) return;
 if ( !(*s) ) return;
 {
  char *new=malloc( sizeof(char) * ( strlen(*s)+2 ) );
  char *p=*s;
  char *q=new;
  while ( *p != '\0' ) { *q=*p; q++; p++; }
  *q=c;
  q++;
  *q='\0';
  free(*s);
  *s=new;
 }
}

char *literalize( char *in ) {
 char *p=in;
 char *new=str_dup("");
 while ( *p != '\0' ) {
  if ( *p == '\'' ) char_append( &new, '\\' );
  else
  if ( *p == '\\' ) char_append( &new, '\\' );
  char_append( &new, *p );
  p++;
 }
 return new;
}

char *str_dup( char *in ) {
 if ( !in ) return str_dup("");
 {
  char *new=malloc(sizeof(char)*(strlen(in)+1));
  char *p=in, *q=new;
  while ( *p != '\0' ) { *q=*p; p++; q++; }
  *q='\0';
  return new;
 }
}

int *int_append( int *s, int c ) {
 int count=0;
 int *p=s;
 while ( *p != -666 ) { p++; count++; }
 count++; //?
 s=realloc( s, sizeof(int) * (count + 1) );
 s[count-1]=c;
 s[count]=-666;
 return s;
}

int count_occurrances( char *a, char *needle ) {
 int count=0;
 char *p;
 p=a;
 while ( *p != '\0' ) {
  if ( *p==*needle ) {
   char *q=needle;
   bool f=true;
   while ( *q != '\0' && f ) {
    f=(LOWER(*q)==LOWER(*p));
    q++;
    p++;
   }
   if ( f ) count++;
  } else p++;
 }
 return count;
}

int char_to_num( char c ) {
 if ( c == '0' ) return 0;
 if ( c == '1' ) return 1;
 if ( c == '2' ) return 2;
 if ( c == '3' ) return 3;
 if ( c == '4' ) return 4;
 if ( c == '5' ) return 5;
 if ( c == '6' ) return 6;
 if ( c == '7' ) return 7;
 if ( c == '8' ) return 8;
 if ( c == '9' ) return 9;
 if ( c == 'A' ) return 10;
 if ( c == 'B' ) return 11;
 if ( c == 'C' ) return 12;
 if ( c == 'D' ) return 13;
 if ( c == 'E' ) return 14;
 if ( c == 'F' ) return 15;
 return 0;
}

char num_to_char( int c ) {
 if ( c == 0 ) return '0';
 if ( c == 1 ) return '1';
 if ( c == 2 ) return '2';
 if ( c == 3 ) return '3';
 if ( c == 4 ) return '4';
 if ( c == 5 ) return '5';
 if ( c == 6 ) return '6';
 if ( c == 7 ) return '7';
 if ( c == 8 ) return '8';
 if ( c == 9 ) return '9';
 if ( c == 10) return 'A';
 if ( c == 11) return 'B';
 if ( c == 12) return 'C';
 if ( c == 13) return 'D';
 if ( c == 14) return 'E';
 if ( c == 15) return 'F';
 return '7';
}
