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

/* This file contains input handling routines for fed */


extern int key;
extern int free_mode;
extern int find_mode;
extern int quit_now;

extern int plugin_result_menu;

extern SAVE_REQUEST *saving;

void get_input  args( ( void ) );

void refind args( ( void ) );
void render_find args( ( void ) );
void process_find_mode args( ( void ) );
void replace_all args( ( void ) );

void draw_quit_now( void );
void process_quit_now( void );

void process_plugin_menu args( ( void ) );
void process_plugin_result_menu args( ( void ) );

void queue_save_files args( ( void ) );
void create_save_as_confirm args( ( FEDFILE *a ) );
void create_save_confirm args( ( FEDFILE *a ) );
void draw_save_confirm args( ( void ) );
void process_save_confirm args( ( void ) );

void process_input args( ( void ) );

void find_file_position args( ( FEDFILE *file, char *position, int *r, int *c ) );
void calculate_cursor_position args( ( FEDFILE *file, char *position ) );
void act_on_active args( ( void ) );

#if defined(NVADERS_IN_FED)
void main_nVaders args( ( int argc, char **argv ) );
#endif
