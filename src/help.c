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
#include "help.h"
#include "color.h"
#include "graphics.h"

int help_mode=0;
int about_mode=0;

void draw_help() {
 attron(COLOR_PAIR(BLACK_ON_GREEN));
 mvprintw (  3,20, "                        %-35s  ", VERSION );
 attroff(COLOR_PAIR(BLACK_ON_GREEN));
 mvprintw2(  4,20, "^^L  .------------------------[^^M Help ^^L]-----------------------.  " );
 mvprintw2(  5,20, "^^L  |^^M f1 or c-/: Help         c-]: Select           c=ctrl  ^^L|  " );
 mvprintw2(  6,20, "^^L  |^^M f2 or c-L: Load         c-\\: Select grid              ^^L|  " );
 mvprintw2(  7,20, "^^L  |^^M f3 or c-S: Save         c-X: Cut selection            ^^L|  " );
 mvprintw2(  8,20, "^^L  |^^M f4,c-Q: Quit,save y/n   c-V: Paste clipboard          ^^L|  " );
 mvprintw2(  9,20, "^^L  |^^M c-W: Toggle display \\n  c-C: Copy selection           ^^L|  " );
 mvprintw2( 10,20, "^^L  |^^M c-I: Insert file        c-K: Delete to buffer         ^^L|  " );
 mvprintw2( 11,20, "^^L  |^^M c-E: Export copy        c-U: Write deleted buffer     ^^L|  " );
 mvprintw2( 12,20, "^^L  |^^M c-N: New file           c-F: Find/Replace c-R:CRLF=NL ^^L|  " );
 mvprintw2( 13,20, "^^L  |^^M c-Z: Undo c-Y: Redo     c-G: Goto#   c-P,c-O:next tab ^^L|  " );
 mvprintw2( 14,20, "^^L  |^^M f12: free move toggle   c-T: insert a character tab   ^^L|  " );
#if !defined(NVADERS_IN_FED)
 mvprintw2( 15,20, "^^L  |^^M f11: unused             f10 or c-6: fed to background ^^L|  " );
#else
 mvprintw2( 15,20, "^^L  |^^M f11: play nInvaders     f10 or c-6: fed to background ^^L|  " );
#endif
 mvprintw2( 16,20, "^^L  |^^M c-H: Backspace          c-M: Enter                    ^^L|  " );
 mvprintw2( 17,20, "^^L  |^^M Ins: Toggle insert      Del: Delete                   ^^L|  " );
 mvprintw2( 18,20, "^^L  |^^M c-B: strip eol spaces   c-D: PHP Plug-in menu         ^^L|  " );
 mvprintw2( 19,20, "^^L  '--------[^^M a: about or any other key to close ^^L]---------'  " );
 attron(COLOR_PAIR(BLACK_ON_GREEN));
 mvprintw ( 20,20, "                                                             " );
 attroff(COLOR_PAIR(BLACK_ON_GREEN));
}

void draw_about() {
 attron(COLOR_PAIR(BLACK_ON_GREEN));
 mvprintw (  3,20, "                        %-35s  ", VERSION );
 mvprintw (  4,20, "  .-----------------------[ About ]-----------------------.  " );
 mvprintw (  5,20, "  |                                                       |  " );
 attroff(COLOR_PAIR(BLACK_ON_GREEN));
 mvprintw2(  6,20, "^^M  | ^^Q     ___           _  ^^M an open source project         |  " );
 mvprintw2(  7,20, "^^M  | ^^Q    / __)         | | ^^M initiated in February 2011 by  |" );
 mvprintw2(  8,20, "^^M  | ^^Q  _| |__ _____  __| | ^^M H. Elwood Gilliland III        |  " );
 mvprintw2(  9,20, "^^M  | ^^Q (_   __) ___ |/ _  | ^^M                                |  " );
 mvprintw2( 10,20, "^^M  | ^^Q   | |  | ____( (_| | ^^M fed: a linux console editor    |  " );
 mvprintw2( 11,20, "^^M  | ^^Q   |_|  |_____)\\____| ^^M (c)2011 h. elwood gilliland 3  |  " );
 attron(COLOR_PAIR(BLACK_ON_GREEN));
 mvprintw ( 12,20, "  |                                                       |  " );
 mvprintw ( 13,20, "  | 'I had the feeling', Jung declared, 'that I had       |  " );
 mvprintw ( 14,20, "  | pushed to the brink of the world; what was of burning |  " );
 mvprintw ( 15,20, "  | interest to me was null and void for others, and even |  " );
 mvprintw ( 16,20, "  | a cause of dread.  Dread of what?  I could not find   |  " );
 mvprintw ( 17,20, "  | an explanation for this.'                             |  " );
 mvprintw ( 18,20, "  |                                                       |  " );
 mvprintw ( 19,20, "  '---------------------------[ any key to close ]--------'  " );
 mvprintw ( 20,20, "                                                             " );
 attroff(COLOR_PAIR(BLACK_ON_GREEN));
}
