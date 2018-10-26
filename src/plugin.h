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

/* This file contains fed plug-in loading and execution routines */

extern PLUGIN *plugins;
extern PLUGIN *selected;
extern RESULT *plugin_results;
extern RESULT *result_selected;
extern int plugin_menu;

void add_result args( ( RESULT *result, char *data ) );
char *process_plugin args( ( PLUGIN *p, char *data ) );
void decode_plugin_output args( ( char *encoded ) );
void read_plugin_folder args( ( char *path, int report_it ) );


void draw_plugin_result_menu args( ( void ) );

void draw_plugin_menu args( ( void ) );
void open_plugin_menu args( ( void ) );
