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

void limit_undos args( ( FEDFILE *file ) );
void push_undo args( ( FEDFILE *file ) );
void undo args( ( FEDFILE *file ) );
void redo args( ( FEDFILE *file ) );
void freedos args( ( FEDFILE *file ) );
