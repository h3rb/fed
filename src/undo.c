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
#include "undo.h"
#include "strings.h"
#include "cli.h"

void limit_undos( FEDFILE *file ) {
 UNDO *u;
 int count=0;
 for ( u=file->undos; u; u=u->next ) count++;
 if ( count >= _undo ) {
  for ( u=file->undos; u && u->next; u=u->next ) ;
  if ( u->next ) {
   free(u->next->fileraw);
   free(u->next);
   u->next=NULL;
  }
 }
}

void push_undo( FEDFILE *file ) {
 UNDO *new=malloc( sizeof(UNDO) ), *next;
 limit_undos(file);
 new->fileraw=file&&file->fileraw?str_dup(file->fileraw):str_dup("");
 new->next=file->undos;
 file->undos=new;
 // Drop redos
 for ( new=file->redos; new; new=next ) {
  next=new->next;
  free(new->fileraw);
  free(new);
 }
 file->redos=NULL;
}

void undo( FEDFILE *file ) {
 UNDO *top=file->undos;
 UNDO *re;
 if ( !top ) return;
 file->undos=file->undos->next;
 re=malloc( sizeof(UNDO) );
 re->fileraw=file->fileraw;
 re->next=file->redos;
 file->redos=re;
 file->fileraw=top->fileraw;
 free(top);
 file->begin=file->end=NULL;
}

void redo( FEDFILE *file ) {
 UNDO *top=file->redos;
 UNDO *un;
 if ( !top ) return;
 file->redos=file->redos->next;
 un=malloc( sizeof(UNDO) );
 un->fileraw=file->fileraw;
 un->next=file->undos;
 file->undos=un;
 file->fileraw=top->fileraw;
 free(top);
 file->begin=NULL;
}

void freedos( FEDFILE *file ) {
 UNDO *new,*next;
 // Drop redos
 for ( new=file->redos; new; new=next ) {
  next=new->next;
  free(new->fileraw);
  free(new);
 }
 // Drop undos
 for ( new=file->undos; new; new=next ) {
  next=new->next;
  free(new->fileraw);
  free(new);
 }
 file->undos=file->redos=NULL;
 file->begin=NULL;
}
