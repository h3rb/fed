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

/* This file contains display routines unique to fed */

#include "fed.h"
#include "cli.h"
#include "dashboard.h"
#include "display.h"
#include "edit.h"
#include "gbuffer.h"
#include "graphics.h"
#include "io.h"
#include "timer.h"
#include "help.h"
#include "clipboard.h"
#include "input.h"
#include "plugin.h"

int changing=1;
FEDFILE *active=NULL;
GBUFFER *current=NULL,*previous=NULL;

void display_resize(void) {
 if ( previous ) free_gbuffer( previous );
 previous=current;
 current=setup_gbuffer( active, WINWIDTH, WINHEIGHT );
 change();
}

void display_update(void) {
 if ( !current ) {
  current=setup_gbuffer( active, WINWIDTH, WINHEIGHT );
 }
 if ( !previous ) {
  previous=setup_gbuffer( active, WINWIDTH, WINHEIGHT );
 }
// if ( changing ) {
//  clear();
  if ( quit_now ) {
   draw_quit_now();
  } else
  if ( saving ) {
   draw_save_confirm();
  } else
  if ( plugin_result_menu ) {
   render_gbuffer( previous, current, _clearscreen, mode, _invert );
   draw_plugin_result_menu();
  } else
  if ( plugin_menu ) {
   render_gbuffer( previous, current, _clearscreen, mode, _invert );
   draw_plugin_menu();
  } else
  if ( find_mode ) {
   render_find();
  } else
  if ( help_mode ) { if ( about_mode ) draw_about(); else draw_help(); } else
  if ( show_clipboard ) draw_clipboard(); else
  render_gbuffer( previous, current, _clearscreen, mode, _invert );
  dashboard( );
  changing=0;
  if ( previous ) free_gbuffer( previous );
  previous=current;
  current=setup_gbuffer( active, WINWIDTH, WINHEIGHT );

// }
}
