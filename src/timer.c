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
#include "timer.h"

time_t then;
time_t now;
time_t last_changed;
struct tm  *ts;
char time_string[80];

extern int changing; // from display.c

void init_timer( void ) {
 now = time(NULL);
 ts = localtime(&now);
}

void update_timer( void ) {
 then=now;
 now = time(NULL);
 ts = localtime(&now);
 if ( now % 2 == 1 ) strftime(time_string, sizeof(time_string), "%l:%M%p", ts);
 else strftime(time_string, sizeof(time_string), "%l %M%p", ts);
 if ( last_changed - now > UPDATE_CLOCK_SECONDS ) change();
}

void change( void ) {
 changing=0;
 last_changed=now;
}
