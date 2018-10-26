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


int from_last_eol args( ( char *start, char *position ) );
void backspace_char args( ( char **in, char *at ) );
void delete_char args( ( char **in, char *at ) );
void insert_char args( ( char **in, char *at, char c ) );
int longest args( ( char *c ) );
bool char_isof args( ( char c, char *list ) );
char *char_at_pos args( ( char *content, int x, int y, int *overall_result ) );
int eolcol_line args( ( char *content, int linenumber ) );
char *go_to_line args( ( char *content, int linenumber, int *result ) );
char *go_to_line_ofs args( ( char *content, int linenumber, int c, int *result ) );
char *trunc_pad args( ( char *argument, int length ) );
char *trunc_fit args( ( char *argument, int length, int from_left, int dot_dot_dot ) );
char *fit args( ( char *argument, int length ) );
char *skip_spaces args( ( char *a ) );
bool contains args( ( char *a, char c ) );
bool equals args( ( char *a, char *b ) );
bool cequals args( ( char *a, char *b ) );
bool prefix args( ( char *a, char *b ) );
bool inside args( ( char *a, char *b ) );
int words args( ( char *argument ) );
char **wordlist args( ( char *in, int *count ) );
void char_append args( ( char **s, char c ) );
char *literalize args( ( char *in ) );
char *str_dup args( ( char *in ) );
int *int_append args( ( int *s, int c ) );
int count_occurrances args( ( char *a, char *needle ) );
int char_to_num args( ( char c ) );
char num_to_char args( ( int c ) );
