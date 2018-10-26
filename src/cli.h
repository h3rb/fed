
extern int _help;
extern int _fallback;
extern int _ruler;
extern int _clock;
extern int _lang;
extern int _plugins;
extern int _fed;
extern int _invert;
extern int _root;
extern int _binary;
extern int _clearscreen;
extern int _statusbar;
extern int _undo;
extern int _mintab;
extern int _tabspaces;
extern int _old;

extern char *lang_home;
extern char *fed_home;
extern char *plugins_folder;

void command_line args( ( int argc, char **argv, int load_files ) );
bool command_line_has args( ( int argc, char **argv, char *test ) );
