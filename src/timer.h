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

#define UPDATE_CLOCK_SECONDS 45

extern time_t then;
extern time_t now;
extern struct tm  *ts;
extern char time_string[80];

void init_timer args( ( void ) );

void update_timer args( ( void ) );

void change args( ( void ) );
