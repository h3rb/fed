void encodeblock args( ( unsigned char in[3], unsigned char out[4], int len ) );
void encode args( ( FILE *infile, FILE *outfile, int linesize ) );
void decodeblock args( ( unsigned char in[4], unsigned char out[3] ) );
void decode args( ( FILE *infile, FILE *outfile ) );
char *b64_message args( ( int errcode ) );
int b64 args( ( int opt, char *infilename, char *outfilename, int linesize ) );
void showuse args( ( void ) );
void b64main args( ( int argc, char **argv ) );
