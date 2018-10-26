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


#include <sys/types.h>
#include <sys/uio.h>
#include <sys/unistd.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "fed.h"
#include "errors.h"
#include "io.h"

void save_the_world( void ) {
 char buf[2048];
 FEDFILE *f;
 for ( f=files; f; f=f->next ) {
  sprintf( buf, "%s.crashed", f->filename );
  write_ascii( f->fileraw, buf );
 }
}

void error_handler( int s ) {

 switch ( s ) {
//   case SIGINT: break;
  case SIGSEGV:
    save_the_world();
   break;
//  case SIGABRT: break;
 }
 psignal( s, "Signal thrown: " );

}


void init_error_handler(void) {

    /*
     * Signals we handle.
     */

//   signal( SIGINT,    error_handler ); // Erodes the interrupt Ctrl-C functionality to "kill" this internally from the console, Ctrl-C=hard reboot
   signal( SIGSEGV,   error_handler ); // Ooops, common problems.
//   signal( SIGABRT,   error_handler ); // Just in case.
/*
   signal( SIGALRM,   error_handler );
   signal( SIGFPE,    error_handler );
   signal( SIGHUP,    error_handler );
   signal( SIGILL,    error_handler );
   signal( SIGKILL,   error_handler );
   signal( SIGPIPE,   error_handler );
   signal( SIGQUIT,   error_handler );
   signal( SIGTERM,   error_handler );
   signal( SIGUSR1,   error_handler );
   signal( SIGUSR2,   error_handler );
   signal( SIGCHLD,   error_handler );
   signal( SIGCONT,   error_handler );
   signal( SIGSTOP,   error_handler );
   signal( SIGTSTP,   error_handler );
   signal( SIGTTIN,   error_handler );
   signal( SIGTTOU,   error_handler );
   signal( SIGBUS,    error_handler );
   signal( SIGPOLL,   error_handler );
   signal( SIGPROF,   error_handler );
   signal( SIGSYS,    error_handler );
   signal( SIGTRAP,   error_handler );
   signal( SIGURG,    error_handler );
   signal( SIGVTALRM, error_handler );
   signal( SIGXCPU,   error_handler );
   signal( SIGXFSZ,   error_handler );
 */
}

void error_message (char * fmt, ...)
{
        char out [1024];
        va_list args;
        va_start (args, fmt);
        vsprintf (out, fmt, args);
        va_end (args);

        printf ("%s",out);
}


