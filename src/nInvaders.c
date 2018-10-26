#if defined(NVADERS_IN_FED)
/**
 * THIS FILE IS MODIFIED, VISIT THE PROJECT PAGE FOR THE ORIGINAL VERSION
 **************************************************************************
 * nInvaders - a space invaders clone for ncurses
 * Copyright (C) 2002-2003 Dettus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * homepage: http://ninvaders.sourceforge.net
 * mailto: ninvaders-devel@lists.sourceforge.net
 *
 */

#include "nInvaders.h"

#define MAJOR    0
#define MINOR    1
#define RELEASE  1

#ifdef WIN32
#define usleep(x) Sleep(x/1000)
#endif

#define FPS 24

#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define CYAN 5
#define MAGENTA 6
#define WHITE 7

int finished=0;
int playing_game=0;

typedef struct Player Player;

struct Player {
	int posX;	  // horizontal position of player
	int posY;	  // vertical position of player
	int speed;	  // 0: no movement; 1: one per turn; etc.
	int missileFired; // 0: missile not running; 1: missile running
	int missileX;	  // horizontal position of missile
	int missileY;	  // vertical position of missile
};

Player player;

WINDOW *wBattleField=NULL;
WINDOW *wEmpty=NULL;
WINDOW *wScores=NULL;
WINDOW *wPlayer=NULL;
WINDOW *wPlayerMissile=NULL;
WINDOW *wAliens=NULL;
WINDOW *wAliensMissile=NULL;
WINDOW *wBunkers=NULL;
WINDOW *wGameOver=NULL;
WINDOW *wUfo=NULL;
WINDOW *wStatus=NULL;
WINDOW *wTitleScreen=NULL;

// todo: let's try to not having to declare these "public"
int weite;
int level;
int skill_level;
int row,col;
int lives;
long score;
int status; // status handled in timer

#define GAME_LOOP 1
#define GAME_NEXTLEVEL 2
#define GAME_PAUSED 3
#define GAME_OVER 4
#define GAME_EXIT 5
#define GAME_HIGHSCORE 6

Aliens aliens;

int shipnum;

#define ALIENS_MAX_NUMBER_X 10
#define ALIENS_MAX_NUMBER_Y 5
#define ALIENS_MAX_MISSILES 10

// todo: move to structure
int lowest_ship[ALIENS_MAX_NUMBER_X];
int alienshotx[ALIENS_MAX_MISSILES];
int alienshoty[ALIENS_MAX_MISSILES];
int alienshotnum;
int alienBlock[ALIENS_MAX_NUMBER_Y][ALIENS_MAX_NUMBER_X];

int bunker[BUNKERHEIGHT][BUNKERWIDTH + 1];

/**
 * initialize level: reset attributes of most units
 */
void initLevel(void)
{
	playerReset();
	aliensReset();
	ufoReset();
	bunkersReset();
	render();
	drawscore();
}


/**
 * evaluate command line parameters 
 */
void evaluateCommandLine(int argc, char **argv)
{

	// -l : set skill level
	if (argc == 3 && strcmp(argv[1], "-l") == 0) {
		if (argv[2][0] >= '0' && argv[2][0] <= '9') {
			skill_level = argv[2][0] - 48;
		} else {
			argc = 2;
		}
	}

	// -gpl : show GNU GPL
	if (argc == 2 && strcmp(argv[1], "-gpl") == 0) {
		showGpl();
	}

	// wrong command line: show usage
	if (argc == 2 || (argc == 3 && strcmp(argv[1], "-l") != 0)) {
		showVersion();
		showUsage();
		exit(1);
	}
}


void FinalScore(int sig)
{
        endwin();
	showGplShort();

	fprintf(stderr,"\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"=========================================================================\n");
	fprintf(stderr,"\n");

	fprintf(stderr,"Final score: %7.7ld, Final level: %2.2d\nFinal rating... ",score,level);
	if (lives>0)
		fprintf(stderr,"Quitter\n\n");
	else if(score<5000)
		fprintf(stderr,"Alien Fodder\n\n");
	else if(score<7500)
		fprintf(stderr,"Easy Target\n\n");
	else if(score<10000)
		fprintf(stderr,"Barely Mediocre\n\n");
	else if(score<12500)
		fprintf(stderr,"Shows Promise\n\n");
	else if(score<15000)
		fprintf(stderr,"Alien Blaster\n\n");
	else if(score<20000)
		fprintf(stderr,"Earth Defender\n\n");
	else if(score>19999)
		fprintf(stderr,"Supreme Protector\n\n");

	showVersion();
        exit(0);
}


void drawscore(void)
{
	statusDisplay(level, score, lives);
}


/**
 * reads input from keyboard and do action
 */
void readInput(void)
{
	int ch;
	static int lastmove;

	ch = getch();		// get key pressed

        if ( ch == KEY_RESIZE ) {
         finish(0); return;
        }

	switch (status) {

	case GAME_PAUSED:

		if (ch == 'p') {
			status = GAME_LOOP;
		}
		break;

	case GAME_HIGHSCORE:

		if (ch == ' ') {
			titleScreenClear();
			level = 0;      // reset level
			score = 0;      // reset score
			lives = 3;      // restore lives
			status = GAME_NEXTLEVEL;
		} else if (ch == 'q') {	// quit game
			status = GAME_EXIT;
		}
		break;

	case GAME_OVER:
		break; // don't do anything

	default:

		if (ch == 'l' || ch == KEY_RIGHT) {	// move player right
			if (lastmove == 'l') {
				playerTurboOn();	// enable Turbo
			} else {
				playerTurboOff();	// disable Turbo
			}
			playerMoveRight();		// move player
			lastmove = 'l';			// remember last move for turbo mode
		} else if (ch == 'h' || ch == KEY_LEFT) {	// move player left 
			if (lastmove == 'h') {
				playerTurboOn();	// enable Turbo
			} else {
				playerTurboOff();	// disable Turbo
			}
			playerMoveLeft();		// move player
			lastmove = 'h';			// remember last move for turbo mode
		} else if (ch == 'k' || ch == ' ') {	// shoot missile
			playerLaunchMissile();
		} else if (ch == 'p') {			// pause game until 'p' pressed again
			// set status to game paused
			status = GAME_PAUSED;
		} else if (ch == 'W') {			// cheat: goto next level
			status = GAME_NEXTLEVEL;
		} else if (ch == 'L') {			// cheat: one more live
			lives++;
			drawscore();
		} else if (ch == 'q') {	// quit game
			status = GAME_EXIT;
		} else {		// disable turbo mode if key is not kept pressed
			lastmove = ' ';
		}

	} // switch

}


/**
 * timer
 * this method is executed every 1 / FPS seconds
 */
void handleTimer(int sig)
{
	static int aliens_move_counter = 0;
	static int aliens_shot_counter = 0;
	static int player_shot_counter = 0;
	static int ufo_move_counter = 0;
	static int title_animation_counter = 0;
	static int game_over_counter = 0;

 if ( finished==1 || !playing_game ) return;

	switch (status) {

	case GAME_NEXTLEVEL:    // go to next level

		level++;	// increase level

		initLevel();	// initialize level

		aliens_move_counter = 0;
		aliens_shot_counter = 0;
		player_shot_counter = 0;
		ufo_move_counter = 0;

		weite = (shipnum+(skill_level*10)-(level*5)+5)/10;

		if (weite < 0) {
			weite = 0;
		}

		// change status and start game!
		status = GAME_LOOP;

	case GAME_LOOP:   	 // do game handling

		// move aliens
		if (aliens_move_counter == 0 && aliensMove() == 1) {
			// aliens reached player
			lives = 0;
			status = GAME_OVER;
		}

		// move player missile
		if (player_shot_counter == 0 && playerMoveMissile() == 1) {
			// no aliens left
			status = GAME_NEXTLEVEL;
		}

		// move aliens' missiles
		if (aliens_shot_counter == 0 && aliensMissileMove() == 1) {
			// player was hit
			lives--;			// player looses one life
			drawscore();	                // draw score
			playerExplode();		// display some explosion graphics
			if (lives == 0) {		// if no lives left ...
				status = GAME_OVER;		// ... exit game
			}
		}

		// move ufo
		if (ufo_move_counter == 0 && ufoShowUfo() == 1) {
			ufoMoveLeft();			// move it one position to the left
		}


		if (aliens_shot_counter++ >= 5) {aliens_shot_counter=0;}     // speed of alien shot
		if (player_shot_counter++ >= 1) {player_shot_counter=0;}     // speed of player shot
		if (aliens_move_counter++ >= weite) {aliens_move_counter=0;} // speed of aliend
		if (ufo_move_counter++ >= 3) {ufo_move_counter=0;}           // speed of ufo

		refreshScreen();
		break;

	case GAME_PAUSED:    // game is paused
		break;

	case GAME_OVER:      // game over
		if (game_over_counter == 100) {
			battleFieldClear();
			status = GAME_HIGHSCORE;
			game_over_counter = 0;
		} else {
			gameOverDisplay();
			game_over_counter++;
		}
		break;

	case GAME_EXIT:      // exit game
		finish(0);
		break;

	case GAME_HIGHSCORE: // display highscore
		if (title_animation_counter == 0) {
			titleScreenDisplay();
		}

		if (title_animation_counter++ >= 6) {title_animation_counter = 0;} // speed of animation
		break;

	}
}


/**
 * set up timer
 */
int timerstarted=0;
void setUpTimer(void)
{
   if ( !timerstarted ) {
	struct itimerval myTimer;
	struct sigaction myAction;
	myTimer.it_value.tv_sec = 0;
	myTimer.it_value.tv_usec = 1000000 / FPS;
	myTimer.it_interval.tv_sec = 0;
	myTimer.it_interval.tv_usec = 1000000 / FPS;
	setitimer(ITIMER_REAL, &myTimer, NULL);
	myAction.sa_handler = &handleTimer;
	myAction.sa_flags = SA_RESTART;
	sigaction(SIGALRM, &myAction, NULL);
        timerstarted=1;
   }
}

void main_nVaders(int argc, char **argv)
{
 playing_game=1;
 	weite = 0;
	score = 0;
	lives = 3;
	level = 0;
	skill_level = 1;

	evaluateCommandLine(argc, argv);	// evaluate command line parameters
	graphicEngineInit();			// initialize graphic engine

	// set up timer/ game handling
	setUpTimer();
	status = GAME_HIGHSCORE;

	// read keyboard input
	do {// do movements and key-checking
	 readInput();
	} while (finished == 0);
 finished=0;
 playing_game=0;
}


void doScoring(int alienType)
{
	int points[4] = {500, 200, 150, 100};   	// 0: ufo, 1:red, 2:green, 3:blue

	score += points[alienType];		// every alien type does different scoring points

	// every 6000 pts player gets a new live
	if (score % 6000 == 0){
		lives++;
	}

	drawscore();	// display score
}


/**
 * sleep for specified time
 */
void doSleep(int microseconds)
{
        usleep(microseconds);
}


/**
 * show version information
 */
void showVersion(void)
{
	fprintf(stderr, "*** nInvaders %i.%i.%i\n", MAJOR, MINOR, RELEASE);
	fprintf(stderr, "*** (C)opyleft 2k2 by Dettus\n");
	fprintf(stderr, "*** dettus@matrixx-bielefeld.de\n");
	fprintf(stderr, "Additional code by Mike Saarna,\n");
	fprintf(stderr, "Sebastian Gutsfeld -> segoh@gmx.net,\n");
	fprintf(stderr, "Alexander Hollinger -> alexander.hollinger@gmx.net and\n");
	fprintf(stderr, "Matthias Thar -> hiast2@compuserve.de\n");
}


/**
 * show usage of command line parameters
 */
void showUsage(void)
{

	fprintf(stderr, "\n\nUsage: nInvaders [-l skill] [-gpl]\n");
	fprintf(stderr, "   where -l 0=NIGHTMARE\n");
	fprintf(stderr, "         -l 1=okay\n");
	fprintf(stderr, "         -l 9=May I play daddy?!\n");
	fprintf(stderr, "\n         -gpl shows you the license file\n");
}


/**
 * wait for input of return to continue
 */
void waitForReturn(void)
{
//	char b[2];
	fprintf(stderr, "...Please press <Enter> to read on...");
        while ( getch() != KEY_ENTER );
//	fgets(b, sizeof(b), stdin);
}

/**
 * show short version of Gnu GPL
 */
void showGplShort(void)
{
	fprintf(stderr,"\n");
	fprintf(stderr,"This program is free software; you can redistribute it and/or modify\n");
	fprintf(stderr,"it under the terms of the GNU General Public License as published by\n");
	fprintf(stderr,"the Free Software Foundation; either version 2 of the License, or\n");
	fprintf(stderr,"(at your option) any later version.\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"This program is distributed in the hope that it will be useful,\n");
	fprintf(stderr,"but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	fprintf(stderr,"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	fprintf(stderr,"GNU General Public License for more details.\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"You should have received a copy of the GNU General Public License\n");
	fprintf(stderr,"along with this program; if not, write to the Free Software\n");
	fprintf(stderr,"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"Use the -gpl  command line switch to see the full license of this program\n");
	fprintf(stderr,"Use the -help command line switch to see who wrote this program \n");
	fprintf(stderr,"\n");
}

/**
 * show GNU GENERAL PUBLIC LICENSE
 */
void showGpl(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "		    GNU GENERAL PUBLIC LICENSE\n");
	fprintf(stderr, "		       Version 2, June 1991\n");
	fprintf(stderr, "\n");
	fprintf(stderr, " Copyright (C) 1989, 1991 Free Software Foundation, Inc.\n");
	fprintf(stderr, " 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n");
	fprintf(stderr, " Everyone is permitted to copy and distribute verbatim copies\n");
	fprintf(stderr, " of this license document, but changing it is not allowed.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "			    Preamble\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  The licenses for most software are designed to take away your\n");
	fprintf(stderr, "freedom to share and change it.  By contrast, the GNU General Public\n");
	fprintf(stderr, "License is intended to guarantee your freedom to share and change free \n");
	fprintf(stderr, "software--to make sure the software is free for all its users. This\n");
	fprintf(stderr, "General Public License applies to most of the Free Software\n");
	fprintf(stderr, "Foundation's software and to any other program whose authors commit to\n");
	fprintf(stderr, "using it.  (Some other Free Software Foundation software is covered by\n");
	fprintf(stderr, "the GNU Library General Public License instead.)  You can apply it to\n");
	fprintf(stderr, "your programs, too.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  When we speak of free software, we are referring to freedom, not\n");
	fprintf(stderr, "price.  Our General Public Licenses are designed to make sure that you\n");
	fprintf(stderr, "have the freedom to distribute copies of free software (and charge for\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "this service if you wish), that you receive source code or can get it\n");
	fprintf(stderr, "if you want it, that you can change the software or use pieces of it\n");
	fprintf(stderr, "in new free programs; and that you know you can do these things.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  To protect your rights, we need to make restrictions that forbid\n");
	fprintf(stderr, "anyone to deny you these rights or to ask you to surrender the rights.\n");
	fprintf(stderr, "These restrictions translate to certain responsibilities for you if you\n");
	fprintf(stderr, "distribute copies of the software, or if you modify it.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  For example, if you distribute copies of such a program, whether\n");
	fprintf(stderr, "gratis or for a fee, you must give the recipients all the rights that\n");
	fprintf(stderr, "you have.  You must make sure that they, too, receive or can get the\n");
	fprintf(stderr, "source code.  And you must show them these terms so they know their\n");
	fprintf(stderr, "rights.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  We protect your rights with two steps: (1) copyright the software, and\n");
	fprintf(stderr, "(2) offer you this license which gives you legal permission to copy,\n");
	fprintf(stderr, "distribute and/or modify the software.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  Also, for each author's protection and ours, we want to make certain\n");
	fprintf(stderr, "that everyone understands that there is no warranty for this free\n");
	fprintf(stderr, "software.  If the software is modified by someone else and passed on, we\n");
	fprintf(stderr, "want its recipients to know that what they have is not the original, so\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "that any problems introduced by others will not reflect on the original\n");
	fprintf(stderr, "authors' reputations.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  Finally, any free program is threatened constantly by software\n");
	fprintf(stderr, "patents.  We wish to avoid the danger that redistributors of a free\n");
	fprintf(stderr, "program will individually obtain patent licenses, in effect making the\n");
	fprintf(stderr, "program proprietary.  To prevent this, we have made it clear that any\n");
	fprintf(stderr, "patent must be licensed for everyone's free use or not licensed at all.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  The precise terms and conditions for copying, distribution and\n");
	fprintf(stderr, "modification follow.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "		    GNU GENERAL PUBLIC LICENSE\n");
	fprintf(stderr, "   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  0. This License applies to any program or other work which contains\n");
	fprintf(stderr, "a notice placed by the copyright holder saying it may be distributed\n");
	fprintf(stderr, "under the terms of this General Public License.  The 'Program', below,\n");
	fprintf(stderr, "refers to any such program or work, and a 'work based on the Program'\n");
	fprintf(stderr, "means either the Program or any derivative work under copyright law:\n");
	fprintf(stderr, "that is to say, a work containing the Program or a portion of it,\n");
	fprintf(stderr, "either verbatim or with modifications and/or translated into another\n");
	fprintf(stderr, "language.  (Hereinafter, translation is included without limitation in\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "the term 'modification'.)  Each licensee is addressed as 'you'.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Activities other than copying, distribution and modification are not\n");
	fprintf(stderr, "covered by this License; they are outside its scope.  The act of\n");
	fprintf(stderr, "running the Program is not restricted, and the output from the Program\n");
	fprintf(stderr, "is covered only if its contents constitute a work based on the\n");
	fprintf(stderr, "Program (independent of having been made by running the Program).\n");
	fprintf(stderr, "Whether that is true depends on what the Program does.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  1. You may copy and distribute verbatim copies of the Program's\n");
	fprintf(stderr, "source code as you receive it, in any medium, provided that you\n");
	fprintf(stderr, "conspicuously and appropriately publish on each copy an appropriate\n");
	fprintf(stderr, "copyright notice and disclaimer of warranty; keep intact all the\n");
	fprintf(stderr, "notices that refer to this License and to the absence of any warranty;\n");
	fprintf(stderr, "and give any other recipients of the Program a copy of this License\n");
	fprintf(stderr, "along with the Program.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "You may charge a fee for the physical act of transferring a copy, and\n");
	fprintf(stderr, "you may at your option offer warranty protection in exchange for a fee.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  2. You may modify your copy or copies of the Program or any portion\n");
	fprintf(stderr, "of it, thus forming a work based on the Program, and copy and\n");
	fprintf(stderr, "distribute such modifications or work under the terms of Section 1\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "above, provided that you also meet all of these conditions:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    a) You must cause the modified files to carry prominent notices\n");
	fprintf(stderr, "    stating that you changed the files and the date of any change.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    b) You must cause any work that you distribute or publish, that in\n");
	fprintf(stderr, "    whole or in part contains or is derived from the Program or any\n");
	fprintf(stderr, "    part thereof, to be licensed as a whole at no charge to all third\n");
	fprintf(stderr, "    parties under the terms of this License.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    c) If the modified program normally reads commands interactively\n");
	fprintf(stderr, "    when run, you must cause it, when started running for such\n");
	fprintf(stderr, "    interactive use in the most ordinary way, to print or display an\n");
	fprintf(stderr, "    announcement including an appropriate copyright notice and a\n");
	fprintf(stderr, "    notice that there is no warranty (or else, saying that you provide\n");
	fprintf(stderr, "    a warranty) and that users may redistribute the program under\n");
	fprintf(stderr, "    these conditions, and telling the user how to view a copy of this\n");
	fprintf(stderr, "    License.  (Exception: if the Program itself is interactive but\n");
	fprintf(stderr, "    does not normally print such an announcement, your work based on\n");
	fprintf(stderr, "    the Program is not required to print an announcement.)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "These requirements apply to the modified work as a whole.  If\n");
	fprintf(stderr, "identifiable sections of that work are not derived from the Program,\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "and can be reasonably considered independent and separate works in\n");
	fprintf(stderr, "themselves, then this License, and its terms, do not apply to those\n");
	fprintf(stderr, "sections when you distribute them as separate works.  But when you\n");
	fprintf(stderr, "distribute the same sections as part of a whole which is a work based\n");
	fprintf(stderr, "on the Program, the distribution of the whole must be on the terms of\n");
	fprintf(stderr, "this License, whose permissions for other licensees extend to the\n");
	fprintf(stderr, "entire whole, and thus to each and every part regardless of who wrote it.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Thus, it is not the intent of this section to claim rights or contest\n");
	fprintf(stderr, "your rights to work written entirely by you; rather, the intent is to\n");
	fprintf(stderr, "exercise the right to control the distribution of derivative or\n");
	fprintf(stderr, "collective works based on the Program.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "In addition, mere aggregation of another work not based on the Program\n");
	fprintf(stderr, "with the Program (or with a work based on the Program) on a volume of\n");
	fprintf(stderr, "a storage or distribution medium does not bring the other work under\n");
	fprintf(stderr, "the scope of this License.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  3. You may copy and distribute the Program (or a work based on it,\n");
	fprintf(stderr, "under Section 2) in object code or executable form under the terms of\n");
	fprintf(stderr, "Sections 1 and 2 above provided that you also do one of the following:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    a) Accompany it with the complete corresponding machine-readable\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "    source code, which must be distributed under the terms of Sections\n");
	fprintf(stderr, "    1 and 2 above on a medium customarily used for software interchange; or,\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    b) Accompany it with a written offer, valid for at least three\n");
	fprintf(stderr, "    years, to give any third party, for a charge no more than your\n");
	fprintf(stderr, "    cost of physically performing source distribution, a complete\n");
	fprintf(stderr, "    machine-readable copy of the corresponding source code, to be\n");
	fprintf(stderr, "    distributed under the terms of Sections 1 and 2 above on a medium\n");
	fprintf(stderr, "    customarily used for software interchange; or,\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    c) Accompany it with the information you received as to the offer\n");
	fprintf(stderr, "    to distribute corresponding source code.  (This alternative is\n");
	fprintf(stderr, "    allowed only for noncommercial distribution and only if you\n");
	fprintf(stderr, "    received the program in object code or executable form with such\n");
	fprintf(stderr, "    an offer, in accord with Subsection b above.)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "The source code for a work means the preferred form of the work for\n");
	fprintf(stderr, "making modifications to it.  For an executable work, complete source\n");
	fprintf(stderr, "code means all the source code for all modules it contains, plus any\n");
	fprintf(stderr, "associated interface definition files, plus the scripts used to\n");
	fprintf(stderr, "control compilation and installation of the executable.  However, as a\n");
	fprintf(stderr, "special exception, the source code distributed need not include\n");
	fprintf(stderr, "anything that is normally distributed (in either source or binary\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "form) with the major components (compiler, kernel, and so on) of the\n");
	fprintf(stderr, "operating system on which the executable runs, unless that component\n");
	fprintf(stderr, "itself accompanies the executable.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "If distribution of executable or object code is made by offering\n");
	fprintf(stderr, "access to copy from a designated place, then offering equivalent\n");
	fprintf(stderr, "access to copy the source code from the same place counts as\n");
	fprintf(stderr, "distribution of the source code, even though third parties are not\n");
	fprintf(stderr, "compelled to copy the source along with the object code.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  4. You may not copy, modify, sublicense, or distribute the Program\n");
	fprintf(stderr, "except as expressly provided under this License.  Any attempt\n");
	fprintf(stderr, "otherwise to copy, modify, sublicense or distribute the Program is\n");
	fprintf(stderr, "void, and will automatically terminate your rights under this License.\n");
	fprintf(stderr, "However, parties who have received copies, or rights, from you under\n");
	fprintf(stderr, "this License will not have their licenses terminated so long as such\n");
	fprintf(stderr, "parties remain in full compliance.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  5. You are not required to accept this License, since you have not\n");
	fprintf(stderr, "signed it.  However, nothing else grants you permission to modify or\n");
	fprintf(stderr, "distribute the Program or its derivative works.  These actions are\n");
	fprintf(stderr, "prohibited by law if you do not accept this License.  Therefore, by\n");
	fprintf(stderr, "modifying or distributing the Program (or any work based on the\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "Program), you indicate your acceptance of this License to do so, and\n");
	fprintf(stderr, "all its terms and conditions for copying, distributing or modifying\n");
	fprintf(stderr, "the Program or works based on it.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  6. Each time you redistribute the Program (or any work based on the\n");
	fprintf(stderr, "Program), the recipient automatically receives a license from the\n");
	fprintf(stderr, "original licensor to copy, distribute or modify the Program subject to\n");
	fprintf(stderr, "these terms and conditions.  You may not impose any further\n");
	fprintf(stderr, "restrictions on the recipients' exercise of the rights granted herein.\n");
	fprintf(stderr, "You are not responsible for enforcing compliance by third parties to\n");
	fprintf(stderr, "this License.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  7. If, as a consequence of a court judgment or allegation of patent\n");
	fprintf(stderr, "infringement or for any other reason (not limited to patent issues),\n");
	fprintf(stderr, "conditions are imposed on you (whether by court order, agreement or\n");
	fprintf(stderr, "otherwise) that contradict the conditions of this License, they do not\n");
	fprintf(stderr, "excuse you from the conditions of this License.  If you cannot\n");
	fprintf(stderr, "distribute so as to satisfy simultaneously your obligations under this\n");
	fprintf(stderr, "License and any other pertinent obligations, then as a consequence you\n");
	fprintf(stderr, "may not distribute the Program at all.  For example, if a patent\n");
	fprintf(stderr, "license would not permit royalty-free redistribution of the Program by\n");
	fprintf(stderr, "all those who receive copies directly or indirectly through you, then\n");
	fprintf(stderr, "the only way you could satisfy both it and this License would be to\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "refrain entirely from distribution of the Program.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "If any portion of this section is held invalid or unenforceable under\n");
	fprintf(stderr, "any particular circumstance, the balance of the section is intended to\n");
	fprintf(stderr, "apply and the section as a whole is intended to apply in other\n");
	fprintf(stderr, "circumstances.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "It is not the purpose of this section to induce you to infringe any\n");
	fprintf(stderr, "patents or other property right claims or to contest validity of any\n");
	fprintf(stderr, "such claims; this section has the sole purpose of protecting the\n");
	fprintf(stderr, "integrity of the free software distribution system, which is\n");
	fprintf(stderr, "implemented by public license practices.  Many people have made\n");
	fprintf(stderr, "generous contributions to the wide range of software distributed\n");
	fprintf(stderr, "through that system in reliance on consistent application of that\n");
	fprintf(stderr, "system; it is up to the author/donor to decide if he or she is willing\n");
	fprintf(stderr, "to distribute software through any other system and a licensee cannot\n");
	fprintf(stderr, "impose that choice.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "This section is intended to make thoroughly clear what is believed to\n");
	fprintf(stderr, "be a consequence of the rest of this License.\n");
	fprintf(stderr, "  8. If the distribution and/or use of the Program is restricted in\n");
	fprintf(stderr, "certain countries either by patents or by copyrighted interfaces, the\n");
	fprintf(stderr, "original copyright holder who places the Program under this License\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "may add an explicit geographical distribution limitation excluding\n");
	fprintf(stderr, "those countries, so that distribution is permitted only in or among\n");
	fprintf(stderr, "countries not thus excluded.  In such case, this License incorporates\n");
	fprintf(stderr, "the limitation as if written in the body of this License.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  9. The Free Software Foundation may publish revised and/or new versions\n");
	fprintf(stderr, "of the General Public License from time to time.  Such new versions will\n");
	fprintf(stderr, "be similar in spirit to the present version, but may differ in detail to\n");
	fprintf(stderr, "address new problems or concerns.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Each version is given a distinguishing version number.  If the Program\n");
	fprintf(stderr, "specifies a version number of this License which applies to it and 'any\n");
	fprintf(stderr, "later version', you have the option of following the terms and conditions\n");
	fprintf(stderr, "either of that version or of any later version published by the Free\n");
	fprintf(stderr, "Software Foundation.  If the Program does not specify a version number of\n");
	fprintf(stderr, "this License, you may choose any version ever published by the Free Software\n");
	fprintf(stderr, "Foundation.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  10. If you wish to incorporate parts of the Program into other free\n");
	fprintf(stderr, "programs whose distribution conditions are different, write to the author\n");
	fprintf(stderr, "to ask for permission.  For software which is copyrighted by the Free\n");
	fprintf(stderr, "Software Foundation, write to the Free Software Foundation; we sometimes\n");
	fprintf(stderr, "make exceptions for this.  Our decision will be guided by the two goals\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "of preserving the free status of all derivatives of our free software and\n");
	fprintf(stderr, "of promoting the sharing and reuse of software generally.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "			    NO WARRANTY\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n");
	fprintf(stderr, "FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n");
	fprintf(stderr, "OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n");
	fprintf(stderr, "PROVIDE THE PROGRAM 'AS IS' WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n");
	fprintf(stderr, "OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n");
	fprintf(stderr, "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n");
	fprintf(stderr, "TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n");
	fprintf(stderr, "PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n");
	fprintf(stderr, "REPAIR OR CORRECTION.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n");
	fprintf(stderr, "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n");
	fprintf(stderr, "REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n");
	fprintf(stderr, "INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n");
	fprintf(stderr, "OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n");
	fprintf(stderr, "TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n");
	fprintf(stderr, "YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n");
	fprintf(stderr, "PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "POSSIBILITY OF SUCH DAMAGES.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "		     END OF TERMS AND CONDITIONS\n");
	fprintf(stderr, "	    How to Apply These Terms to Your New Programs\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  If you develop a new program, and you want it to be of the greatest\n");
	fprintf(stderr, "possible use to the public, the best way to achieve this is to make it\n");
	fprintf(stderr, "free software which everyone can redistribute and change under these terms.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  To do so, attach the following notices to the program.  It is safest\n");
	fprintf(stderr, "to attach them to the start of each source file to most effectively\n");
	fprintf(stderr, "convey the exclusion of warranty; and each file should have at least\n");
	fprintf(stderr, "the 'copyright' line and a pointer to where the full notice is found.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    <one line to give the program's name and a brief idea of what it does.>\n");
	fprintf(stderr, "    Copyright (C) <year>  <name of author>\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    This program is free software; you can redistribute it and/or modify\n");
	fprintf(stderr, "    it under the terms of the GNU General Public License as published by\n");
	fprintf(stderr, "    the Free Software Foundation; either version 2 of the License, or\n");
	fprintf(stderr, "    (at your option) any later version.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    This program is distributed in the hope that it will be useful,\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "    but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	fprintf(stderr, "    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	fprintf(stderr, "    GNU General Public License for more details.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    You should have received a copy of the GNU General Public License\n");
	fprintf(stderr, "    along with this program; if not, write to the Free Software\n");
	fprintf(stderr, "    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Also add information on how to contact you by electronic and paper mail.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "If the program is interactive, make it output a short notice like this\n");
	fprintf(stderr, "when it starts in an interactive mode:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    Gnomovision version 69, Copyright (C) year name of author\n");
	fprintf(stderr, "    Gnomovision comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n");
	fprintf(stderr, "    This is free software, and you are welcome to redistribute it\n");
	fprintf(stderr, "    under certain conditions; type `show c' for details.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "The hypothetical commands `show w' and `show c' should show the appropriate\n");
	fprintf(stderr, "parts of the General Public License.  Of course, the commands you use may\n");
	fprintf(stderr, "be called something other than `show w' and `show c'; they could even be\n");
	fprintf(stderr, "mouse-clicks or menu items--whatever suits your program.\n");
	fprintf(stderr, "\n");

		waitForReturn();

	fprintf(stderr, "\n");
	fprintf(stderr, "You should also get your employer (if you work as a programmer) or your\n");
	fprintf(stderr, "school, if any, to sign a 'copyright disclaimer' for the program, if\n");
	fprintf(stderr, "necessary.  Here is a sample; alter the names:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  Yoyodyne, Inc., hereby disclaims all copyright interest in the program\n");
	fprintf(stderr, "  `Gnomovision' (which makes passes at compilers) written by James Hacker.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  <signature of Ty Coon>, 1 April 1989\n");
	fprintf(stderr, "  Ty Coon, President of Vice\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "This General Public License does not permit incorporating your program into\n");
	fprintf(stderr, "proprietary programs.  If your program is a subroutine library, you may\n");
	fprintf(stderr, "consider it more useful to permit linking proprietary applications with the\n");
	fprintf(stderr, "library.  If this is what you want to do, use the GNU Library General\n");
	fprintf(stderr, "Public License instead of this License.\n");
	fprintf(stderr, "\n");

		waitForReturn();

}



/**
 * initialize aliens attributes
 */
void aliensReset(void)
{
	int i,j;

	// three different types of aliens [5], [10]
	int lvl[ALIENS_MAX_NUMBER_Y][ALIENS_MAX_NUMBER_X]={
		{1,1,1,1,1,1,1,1,1,1},
		{2,2,2,2,2,2,2,2,2,2},
		{2,2,2,2,2,2,2,2,2,2},
		{3,3,3,3,3,3,3,3,3,3},
		{3,3,3,3,3,3,3,3,3,3}
	};

	aliensClear(aliens.posX, aliens.posY, aliens.right, aliens.bottom);	// clear old position of aliens

	// reset alien position
	aliens.posX = 0;
	aliens.posY = 0;
	aliens.right = 0;
	aliens.bottom = 0;
	aliens.left = 0;
	aliens.speed = 1;

	// copy lvl-array to enemy-array 
	for (i=0;i<ALIENS_MAX_NUMBER_X;i++) {
		for (j=0;j<ALIENS_MAX_NUMBER_Y;j++) {
			alienBlock[j][i]=lvl[j][i];
		}
	}

	// reset missiles
	for (i = 0; i < ALIENS_MAX_MISSILES; i++) {
		if (alienshotx[i] != 0) {
			aliensMissileClear(alienshotx[i],alienshoty[i]);	// clear old position
		}
		alienshotx[i] = 0;  // start with zero values
		alienshoty[i] = 0;  // start with zero values
	}
	alienshotnum = 1;	    // one missile at the same time
	alienshotx[0] = 5;	    // x position of first alienshot
	alienshoty[0] = 1;	    // y position of first alienshot

}

/**
 * initialize bunkers attributes
 */
void bunkersReset(void)
{
	int a, b;

	// set position of bunker sprites. user graphical char bunkerd for better visual overview
	// and set according to this the bunker-array
	char bunkerd[BUNKERHEIGHT][BUNKERWIDTH+1] = {
		"        ###                 ###                 ###                 ###         ",
		"       #####               #####               #####               #####        ",
		"      #######             #######             #######             #######       ",
		"      ##   ##             ##   ##             ##   ##             ##   ##       "
	};
	//       12345678901234567890123456789012345678901234567890123456789012345678901234567890
	// 80 characters wide

	// copy graphical "bunkerd" to binary "bunker"
	for (a = 0; a < BUNKERWIDTH; a++) {
		for (b = 0; b < BUNKERHEIGHT; b++) {
			if (bunkerd[b][a] == '#')
				bunker[b][a] = 1;
			else
				bunker[b][a] = 0;
		}
	}

	// display bunkers sprite
	bunkersDisplay(&bunker[0][0]);	
}

/**
 * move aliens and test if they've reached the
 * bottom of the windows or the bunkers.
 */
int aliensMove(void)
{

	int cx,cy;
	int fReachedPlayer=0; 				// return value

	render();
	aliensClear(aliens.posX, aliens.posY, aliens.right, aliens.bottom);	// clear old position of aliens

	aliens.posX = aliens.posX + aliens.speed;			// move aliens left/ right

	// when aliens reached left or right screen-border
	if (aliens.posX == (BUNKERWIDTH + BUNKERX - 5 - aliens.right) || aliens.posX == (BUNKERX + aliens.left)) {

		aliens.posY++;				// move aliens downwards

		// aliens reached player
		if (aliens.posY == SCREENHEIGHT - 2 - aliens.bottom) {
			fReachedPlayer = 1;		// set return value
		}
		
		// aliens reached bunkers //funzt nicht ganz: todo
		if (aliens.posY == BUNKERY - aliens.bottom) {
			// clear bunkers
			for(cx=0;cx<BUNKERWIDTH;cx++) {
				for(cy=0;cy<BUNKERHEIGHT;cy++) { 
					bunker[cy][cx]=0;	
				}
			}
			bunkersClear();	// clear bunkers sprites
		}
		
		aliens.speed = aliens.speed * (-1);		  // change direction of aliens' movements
	}

	aliensDisplay(aliens.posX, aliens.posY, aliens.right, aliens.bottom); // display aliens at new position
	
	return fReachedPlayer;				  // return if aliens reached player
}


/**
 * display alien animation, display remaining parts of aliens and bunker 
 */
void render(void)
{
	int k,r;
	int c=0;

	// calculate left, right, bottom, lowest_ship	
	aliens.left=1;
	aliens.right=-1;
	aliens.bottom=-1;
	shipnum=0;
	for (k=0;k<ALIENS_MAX_NUMBER_X;k++) {
		lowest_ship[k]=-1;
	}
	
	for (r=0;r<ALIENS_MAX_NUMBER_Y*2;r++) {
		if ((r%2)==0){
			for (k=0;k<ALIENS_MAX_NUMBER_X;k++) {
				if (alienBlock[c][k] != 0) {
					lowest_ship[k]=r;
					shipnum++;
					if (aliens.left==1 || -k>aliens.left) {aliens.left=-k;}
					if (aliens.right==-1 || k>aliens.right) {aliens.right=k;}
					if (aliens.bottom==-1 || c>aliens.bottom) {aliens.bottom=c;}
				} 
			}
		} else {
			c++;
		}
	}
	aliens.bottom=aliens.bottom*2;	// every 2nd r is an empty r
	aliens.left=aliens.left*3; // alien sprite is 3 chars wide
	aliens.right=aliens.right*3; // alien sprite is 3 chars wide
	
	// display remaining aliens with animation
	aliensRefresh(level, &alienBlock[0][0]);

}


/**
 * move aliens' missiles and do player/bunker hit testing
 * a zero value in alienshotx indicates that the appropriate missile is loaded, but not fired
 */
int aliensMissileMove()
{
	int i, tmp;
	int fPlayerWasHit = 0;
	int shootThreshold;
	static int alienshot_counter = 0;

	
	// calculate threshold when next missile should be fired
	// it is done here to save calculation time in for-instruction 
	shootThreshold = (skill_level * 8) * (shipnum + 2);
	alienshot_counter = alienshot_counter + 10 ;
	
	// loop all possible missiles
	for (i = 0; i < ALIENS_MAX_MISSILES; i++) {
		
		// if the current missile is flying we should do movements
		if (alienshotx[i] != 0) {
			
			aliensMissileClear(alienshotx[i],alienshoty[i]);	// clear old position
				
			// if missile hit the bunkers	
			if (bunkersHitCheck(alienshotx[i], alienshoty[i]) == 1) {
				alienshotx[i] = 0;		// value of zero reloads missile
			}
			
			alienshoty[i]++;			// move missile downwards
			
			// check if player was hit by an alien missile
			if (playerHitCheck(alienshotx[i], alienshoty[i]) == 1) {
				alienshotx[i] = 0;		// value of zero reloads missile
				fPlayerWasHit = 1;
			}
			
			
		} else {					// missile not launched yet
			
			// start new missile if counter says so
			if (alienshot_counter > shootThreshold && shipnum > 0) {// only shot if there's an alien left
				alienshot_counter = 0;				// reset counter				
				tmp = random() % ALIENS_MAX_NUMBER_X;  		// randomly select one of the ...
				while (lowest_ship[tmp] == -1) {		// ...aliens at the bottom of ...
					tmp = random() % ALIENS_MAX_NUMBER_X;	// ...a column to launch missile
				}
				alienshoty[i]=aliens.posY+lowest_ship[tmp];		// set y position of missile
				alienshotx[i]=aliens.posX+tmp*3;			// set x position of missile
			}
		} // if 
		
		// display missiles if still running or just launched; could have been modified in the above code
		if (alienshotx[i] != 0) {
			// if missile is out of battlefield 
			if (alienshoty[i] == SCREENHEIGHT - 1) {
				alienshotx[i] = 0;	// reload missile	
			} else {
				aliensMissileDisplay(alienshotx[i], alienshoty[i]); // display Missile at new position
			}
		}		
		
	} // for


	return fPlayerWasHit;
}



/**
 * check if missile hit an alien
 */
int aliensHitCheck(int shotx, int shoty)
{
	int alienType = 0;
	int shipx, shipy;
	// if missile is within alien-rectangle 
	if (shotx >= aliens.posX && shotx <= aliens.posX + ALIENS_MAX_NUMBER_X * 3 - 1
	    && shoty >= aliens.posY && shoty <= aliens.posY + (ALIENS_MAX_NUMBER_Y - 1) * 2) {
		// calculate the ship that was hit
		shipx = (shotx - aliens.posX) / 3;
		shipy = (shoty - aliens.posY) / 2;
		// if there is still a ship at this position
		alienType = alienBlock[shipy][shipx];
		if (alienType != 0) {
			alienBlock[shipy][shipx] = 0;	// delete alien ship
		}
	}
	return alienType; 	// returns 0 if no alien was hit, else returns type-code of alien
}

/**
 * check if missile hit an element of bunker
 */
int bunkersHitCheck(int shotx, int shoty)
{
	int adjx, adjy;
	int fBunkerWasHit = 0;
	// if missile is within bunker-rectangle
	if (shotx >= BUNKERX && shotx < BUNKERX + BUNKERWIDTH
	    && shoty >= BUNKERY && shoty < BUNKERY + BUNKERHEIGHT) {
		// calculate the element of the bunker that was hit
		adjy = shoty - BUNKERY; 
		adjx = shotx - BUNKERX;
		// if there is still an element
		if(bunker[adjy][adjx] == 1){
			bunker[adjy][adjx] = 0;	// delete element
			fBunkerWasHit = 1; 		// bunker was hit!
		}
	}
	return fBunkerWasHit;
}


/**
 * initialize player sprites
 */
void playerInit(void)
{
	wPlayer = newpad(1, PLAYERWIDTH);       // new pad with appropriate size
	wclear(wPlayer);			// clear pad
        wattrset(wPlayer,COLOR_PAIR(YELLOW));	// set color
        waddstr(wPlayer,"/-^-\\");	        // set sprite
}


/**
 * display player sprite
 */
void playerDisplay(int x, int y) 
{
	copywin(wPlayer,wBattleField,0,0,y,x,y,x+PLAYERWIDTH-1,0);
}


/**
 * clear player sprite
 */
void playerClear(int x, int y) 
{
	copywin(wEmpty,wBattleField,0,0,y,x,y,x+PLAYERWIDTH-1,0);
}


/**
 * initialize missile sprites
 */
void playerMissileInit(void)
{
	wPlayerMissile = newpad(1, 1);		// new pad with appropriate size
	wclear(wPlayerMissile);			// clear pad
	wattrset(wPlayerMissile,COLOR_PAIR(WHITE));	// set color
	waddch(wPlayerMissile,'!');		// set sprite
	wrefresh(wPlayerMissile);
}


/**
 * display missile sprite
 */
void playerMissileDisplay(int x, int y) 
{
	copywin(wPlayerMissile,wBattleField,0,0,y,x,y,x,0);
}


/**
 * clear missile sprite
 */
void playerMissileClear(int x, int y) 
{
	copywin(wEmpty,wBattleField,0,0,y,x,y,x,0);
}


/**
 * some explosion animation
 */
void playerExplosionDisplay(int x, int y)
{
	WINDOW* wPlayerExplosion;
	char playerExplosionChars[16+1]="@~`.,^#*-_=\\/%{}";
	int t,s;
	
	wPlayerExplosion=newpad(1,PLAYERWIDTH);		// new pad
	wattrset(wPlayerExplosion,COLOR_PAIR(YELLOW));	// set color
	
	for(t=0;t<5;t++){ 			// 5 frames
		wclear(wPlayerExplosion);	// clear pad
		for(s=0;s<PLAYERWIDTH;s++){
			waddch(wPlayerExplosion,playerExplosionChars[rand()%16]);	// sprite
		}

		copywin(wPlayerExplosion,wBattleField,0,0,y,x,y,x+PLAYERWIDTH-1,0); 	// display explostion
		wrefresh(wBattleField); 	// refresh battelfield to display explosion frames
		doSleep(100000);		// play animation not too fast
	}
	

} // todo: kann man bestimmt noch besser machen.


/**
 * initialize aliens sprites
 */
static void aliensInit(void)
{
	wAliens = newpad(ALIENS_MAX_NUMBER_Y*2,ALIENS_MAX_NUMBER_X*3);
	wclear(wAliens);
}


/**
 * display aliens sprite
 */
void aliensDisplay(int x, int y, int wid, int hgt) 
{
	copywin(wAliens,wBattleField,0,0,y,x,y+hgt,x+wid+2,0);
}


/**
 * clear aliens sprite
 */
void aliensClear(int x, int y, int wid, int hgt) 
{
	copywin(wEmpty,wBattleField,0,0,y,x,y+hgt,x+wid+2,0);
}


/**
 * initialize missile sprites
 */
static void aliensMissileInit(void)
{
	wAliensMissile = newpad(1, 1);		// new pad
	wclear(wAliensMissile);			// clear pad
	wattrset(wAliensMissile, COLOR_PAIR(CYAN));	// set color
	waddch(wAliensMissile, ':');			// set sprite
}


/**
 * display missile sprite
 */
void aliensMissileDisplay(int x, int y) 
{
	copywin(wAliensMissile,wBattleField,0,0,y,x,y,x,0);
}


/**
 * clear missile sprite
 */
void aliensMissileClear(int x, int y) 
{
	copywin(wEmpty,wBattleField,0,0,y,x,y,x,0);
}


/**
 * refresh aliens sprite
 */
void aliensRefresh(int lvl, int *pAliens) 
{
	static int frame = 0; // used for animation; mod 2 == 0: frame1, mod2 == 1: frame2
	int k,r;
	int c = 0;
	int alienType = 0;
	char ships[2][9][3+1] = {
		{",^,", "_O-", "-o-",  "o=o", "<O>", "_x_", "*^*", "\\_/", "o o"},
		{".-.", "-O_", "/o\\", "o-o", "<o>", "-x-", "o^o", "/~\\", "oo "}
	};
	int colors[9] = {RED, GREEN, BLUE, RED, YELLOW, WHITE, WHITE, YELLOW, RED};

	wclear(wAliens);						// clear pad
	wattrset(wAliens,COLOR_PAIR(RED));				// set color
	
	frame++;						// next frame
	
	// draw alien if there is one
	for (r = 0; r < ALIENS_MAX_NUMBER_Y*2; r++) {			
		for (k = 0; k < ALIENS_MAX_NUMBER_X; k++) {
			if ((r % 2) == 0) {			// display aliens every even r
				alienType = *(pAliens + c * (ALIENS_MAX_NUMBER_X) + k); 	// get type of alien //alienBlock[c][k]
				
				if (alienType != 0) {		// if there is an alien to display
					wattrset(wAliens,COLOR_PAIR(colors[alienType-1]));		   // set color
					waddch(wAliens,ships[frame%2][alienType-1+(3*((lvl-1)%3))][0]);  // set char1
					waddch(wAliens,ships[frame%2][alienType-1+(3*((lvl-1)%3))][1]);  // set char2
					waddch(wAliens,ships[frame%2][alienType-1+(3*((lvl-1)%3))][2]);  // set char3
					if (alienType > 4) {
						*(pAliens + c * ALIENS_MAX_NUMBER_X + k) = (alienType + 1) % 9;
					} // todo: what's that? If alien_type > 4 then do a modulo operation???
				} else {
					waddstr(wAliens,"   ");	// no alien
				}
				
			} else {
				waddstr(wAliens,"   ");		// no aliens at odd rs
			}
		}
		if ((r % 2) == 1) {c++;} // goto next r at alienblock
	}
}


/**
 * initialize bunkers sprites
 */
static void bunkersInit(void)
{
	wBunkers = newpad(BUNKERHEIGHT, BUNKERWIDTH);		// new pad data
	wclear(wBunkers);
}


/**
 * display bunkers sprite
 * needs pointer to bunker-array
 */
void bunkersDisplay(int *pBunker) 
{
	int l, k;
	wclear(wBunkers);
	wattrset(wBunkers,COLOR_PAIR(CYAN));
	for (l=0;l<BUNKERHEIGHT;l++) {
		for (k=0;k<BUNKERWIDTH;k++) {
			if (*(pBunker + (l * (BUNKERWIDTH + 1)) + k) == 1) {	//if (pBunker[l][k]==1) {
				waddch(wBunkers,'#');
			} else {
				waddch(wBunkers,' ');
			}
		}	
	}
	
	copywin(wBunkers, wBattleField, 0, 0, BUNKERY, BUNKERX, BUNKERY + BUNKERHEIGHT - 1, BUNKERX + BUNKERWIDTH - 1, 0);
}


/**
 * clear bunkers sprite
 */
void bunkersClear(void) 
{
	copywin(wEmpty, wBattleField, 0, 0, BUNKERY, BUNKERX, BUNKERY + BUNKERHEIGHT - 1, BUNKERX + BUNKERWIDTH - 1, 0);
}


/**
 * clear one element of bunkers sprite at position (x, y)
 */
void bunkersClearElement(int x, int y) 
{
	copywin(wEmpty, wBattleField, 0, 0, y, x, y, x, 0);
}


/**
 * set actual sprite for ufo animation
 */
void ufoRefresh(void)
{
	char u[4][6] = {"<o o>", "<oo >", "<o o>", "< oo>"};
	static int frame = 0;

	wclear(wUfo);
        wattrset(wUfo, COLOR_PAIR(MAGENTA));
	waddstr(wUfo, u[frame % 4]);

	frame++;
}


/**
 * initialize ufo sprite
 */
void ufoInit(void)
{
	wUfo = newpad(1, UFOWIDTH);	     // new pad with appropriate size
	wclear(wUfo);    		     // clear pad
        wattrset(wUfo, COLOR_PAIR(MAGENTA)); // set color
}


/**
 * display ufo sprite
 */
void ufoDisplay(int x, int y)
{
	copywin(wUfo, wBattleField, 0, 0, y, x, y, x + UFOWIDTH - 1, 0);
}


/**
 * clear ufo sprite
 */
void ufoClear(int x, int y) 
{
	copywin(wEmpty, wBattleField, 0, 0, y, x, y, x + UFOWIDTH - 1, 0);
}


/**
 * initialize gameover graphic
 */
static void gameOverInit(void)
{
	// init game-over banner
	wGameOver = newpad(13, 31);
	wclear(wGameOver);
	wattrset(wGameOver, COLOR_PAIR(WHITE));
	waddstr(wGameOver, "                               ");
	waddstr(wGameOver, "  #####   ####  ##   ## ###### ");
	waddstr(wGameOver, " ##      ##  ## ####### ##     ");
	waddstr(wGameOver, " ## ###  ###### ## # ## #####  ");
	waddstr(wGameOver, " ##  ##  ##  ## ##   ## ##     ");
	waddstr(wGameOver, "  #####  ##  ## ##   ## ###### ");
	waddstr(wGameOver, "                               ");
	waddstr(wGameOver, "  ####  ##   ## ###### ######  ");
	waddstr(wGameOver, " ##  ## ##   ## ##     ##   ## ");
	waddstr(wGameOver, " ##  ##  ## ##  #####  ######  ");
	waddstr(wGameOver, " ##  ##  ## ##  ##     ##  ##  ");
	waddstr(wGameOver, "  ####    ###   ###### ##   ## ");
	waddstr(wGameOver, "                               ");
}

/**
 * display game over graphic
 */ 
void gameOverDisplay(void)
{
	int x = (SCREENWIDTH / 2) - (31 / 2);
	int y = (SCREENHEIGHT / 2) - (13 / 2);
	copywin(wGameOver, wBattleField, 0, 0, y, x, y + 12, x + 30, 0);
	wrefresh(wBattleField);
}


/**
 * initialize title screen
 */
static void titleScreenInit(void)
{
	wTitleScreen = newpad(SCREENHEIGHT, SCREENWIDTH);
	wclear(wTitleScreen);
}


/**
 * display title screen
 */
void titleScreenDisplay(void)
{
	static int frame = 0;
	int x, y;
	int i;
	WINDOW *wTitleText;
	WINDOW *wAl;
	WINDOW *wStartText;
	char u[4][6] = {"<o o>", "<oo >", "<o o>", "< oo>"};
	char as[2][9][3+1] = {
		{",^,", "_O-", "-o-",  "o=o", "<O>", "_x_", "*^*", "\\_/", "o o"},
		{".-.", "-O_", "/o\\", "o-o", "<o>", "-x-", "o^o", "/~\\", "oo "}
	};
	int s[3] = {200, 150, 100};
	int colors[9] = {RED, GREEN, BLUE, RED, GREEN, BLUE, RED, GREEN, BLUE};
	char buffer[12];
	static int alien_type = 0;

	wTitleText = newpad(4, 41);
	wclear(wTitleText);
	wattrset(wTitleText, COLOR_PAIR(YELLOW));
	waddstr(wTitleText, "        ____                 __          ");
	waddstr(wTitleText, "  ___  /  _/__ _  _____  ___/ /__ _______");
        waddstr(wTitleText, " / _ \\_/ // _ \\ |/ / _ `/ _  / -_) __(_-<");
	waddstr(wTitleText, "/_//_/___/_//_/___/\\_,_/\\_,_/\\__/_/ /___/");

	frame++;
	wAl = newpad(7, 11);
	wclear(wAl);
	snprintf(buffer, sizeof(buffer),"%s = 500", u[frame % 4]);
	wattrset(wAl, COLOR_PAIR(MAGENTA));
	waddstr(wAl, buffer);
	if ((frame = frame % 60) == 0) {
		alien_type = 0;
	} else if (frame == 20) {
		alien_type = 3;
	} else if (frame == 40) {
		alien_type = 6;
	}
	for (i = alien_type; i < alien_type + 3; i++) {
		waddstr(wAl, "           ");
		snprintf(buffer, sizeof(buffer), "%s   = %d", as[frame % 2][i], s[i % 3]);
		wattrset(wAl, COLOR_PAIR(colors[i]));
		waddstr(wAl, buffer);
	}

	wStartText = newpad(1, 20);
	wclear(wStartText);
	wattrset(wStartText, COLOR_PAIR(RED));
	waddstr(wStartText, "Press SPACE to start");
	
	x = (SCREENWIDTH / 2) - (41 / 2);
	y = 0;
	copywin(wTitleText, wTitleScreen, 0, 0, y, x, y + 3, x + 40, 0);

	x = (SCREENWIDTH / 2) - (11 / 2);
	y = 8;
	copywin(wAl, wTitleScreen, 0, 0, y, x , y + 6, x + 10, 0);

	x = (SCREENWIDTH / 2) - (20 / 2);
	y = SCREENHEIGHT - 2;
	copywin(wStartText, wTitleScreen, 0, 0, y, x, y, x + 19, 0);
	
	copywin(wTitleScreen, wBattleField, 0, 0, 0, 0, SCREENHEIGHT-1, SCREENWIDTH-1, 0);
	
	wrefresh(wBattleField);
}


/**
 * clear title screen
 */
void titleScreenClear(void)
{
	battleFieldClear();
}


/**
 * initialize scores
 */
void statusInit(void)
{
	wStatus = newpad(1, 55);
	wclear(wStatus);
}


/**
 * display scores
 */
void statusDisplay(int lvl, int s, int ls)
{
	int t, xOffset;
	char strStatus[55];
	// "Level: 01 Score: 0001450 Lives: /-\ /-\ /-\ /-\ /-\ "
	// "1234567890123456789012345678901234567890123456789012"

	
	xOffset = (SCREENWIDTH / 2) - 24;
	


	sprintf (strStatus, "Level: %2.2d Score: %2.7d Lives: ", lvl, s);

	wclear(wStatus);
	wattrset(wStatus, COLOR_PAIR(RED));
	waddstr(wStatus, strStatus);

	// show maximal five lives
	for (t = 1; ((t <= 5) && (t < ls)); t++){
		waddstr(wStatus, "/-\\ ");
	}
	
	copywin(wStatus, wBattleField, 0, 0, SCREENHEIGHT-1, xOffset, SCREENHEIGHT-1, xOffset + 54, 0);
	

}


/**
 * initialize battlefield
 */
void battleFieldInit(void)
{
	wEmpty = newpad(SCREENHEIGHT, SCREENWIDTH);
	wclear(wEmpty);
	
	wBattleField = newwin(SCREENHEIGHT, SCREENWIDTH, 0, 0);	// new window
	wclear(wBattleField);						// clear window
	mvwin(wBattleField, 0, 0);					// move window
}


/**
 * clear battlefield
 */
void battleFieldClear(void)
{
	copywin(wEmpty,wBattleField,0,0,0,0,SCREENHEIGHT-1,SCREENWIDTH-1,0);
}


/**
 * refresh screen so that modified graphic buffers get visible
 */
void refreshScreen(void)
{
	redrawwin(wBattleField); // needed to display graphics properly at startup on some terminals
	wrefresh(wBattleField);
}


/**
 * do proper cleanup
 */
#define DELWIN(w)   if (w) { wrefresh(w); delwin(w); w=NULL; } 
extern int finished;
void finish(int sig)
{
#if defined(NVADERS_IN_FED)
  finished=1;
DELWIN(wBattleField);
DELWIN(wEmpty);
DELWIN(wScores);
DELWIN(wPlayer);
DELWIN(wPlayerMissile);
DELWIN(wAliens);
DELWIN(wAliensMissile);
DELWIN(wBunkers);
DELWIN(wGameOver);
DELWIN(wUfo);
DELWIN(wStatus);
DELWIN(wTitleScreen);

#else
	endwin();	// <curses.h> reset terminal into proper non-visual mode
	exit(0);
#endif
}


/**
 * initialize n_courses
 */
void graphicEngineInit(void)
{

#if !defined(NVADERS_IN_FED)
	signal(SIGINT, finish);	// <signal.h> on signal "SIGINT" call method "finish"
	initscr();		// <curses.h> do initialization work
	keypad(stdscr, TRUE);		// <curses.h> enable keypad for input
	nonl();			// <curses.h> disable translation return/ newline for detection of return key
	cbreak();		// <curses.h> do not buffer typed characters, use at once
	noecho();		// <curses.h> do not echo typed characters
	start_color();			// <curses.h> use colors
#endif
        getmaxyx(stdscr,row,col);
#if !defined(NVADERS_IN_FED)
	init_pair(RED, COLOR_RED, COLOR_BLACK);		// <curses.h> define color-pair
	init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);	// <curses.h> define color-pair
	init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);	// <curses.h> define color-pair
	init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);	// <curses.h> define color-pair
	init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);	// <curses.h> define color-pair
	init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);	// <curses.h> define color-pair
	init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);	// <curses.h> define color-pair
#endif

	//timeout(0); 			// <curses.h> do not wait for input

	// initialize sprites
	battleFieldInit();
	playerInit();
	playerMissileInit();
	aliensInit();
	aliensMissileInit();
	bunkersInit();
	ufoInit();
	gameOverInit();
	statusInit();
	titleScreenInit();
}

static int fShowUfo = 0;

/**
 * initialize ufo attributes
 */
void ufoReset(void)
{
	ufoClear(ufo.posX, ufo.posY);	// clear old position of player

	fShowUfo = 0;                   // do not show ufo
	ufo.posY = UFOPOSY;	        // set vertical Position
	ufo.posX = SCREENWIDTH - UFOWIDTH;// set horizontal Position
}

/**
 * move ufo horizontally to position posX
 */
static void ufoMove(int posX)
{
	ufoClear(ufo.posX, ufo.posY);   // clear sprite
	ufo.posX = posX;
	ufoRefresh();
	ufoDisplay(ufo.posX, ufo.posY);
}


/**
 * move ufo and check if it reached the left screen border
 */
void ufoMoveLeft(void)
{
	// check if space between ufo and screen border is big enough
	if (ufo.posX > 1) {
		// desired movement is possible
		ufoMove(ufo.posX - 1);
	} else {
		ufoReset();
	}
}

/**
 * check if the first screen line is free for the ufo and
 * display it randomly
 */
int ufoShowUfo(void)
{
	if (aliens.posY > 0 && fShowUfo == 0) { // aliens one line down
		if ((random() % 200) == 0) {
			fShowUfo = 1;
		}
	}

	return fShowUfo;
}

/**
 * check if ufo was hit by player and delete it from screen
 */
int ufoHitCheck(int shotX, int shotY)
{
	int fUfoWasHit = 0;

	// if shot reached vertikal position of ufo
	if (shotY == UFOPOSY) {
		// if shot hits ufo
		if (shotX >= ufo.posX && shotX <= ufo.posX + UFOWIDTH -1) {
			ufoReset();
			fUfoWasHit = 1;
		}
	}

	return fUfoWasHit;
}

/**
 * initialize player attributes
 */
void playerReset(void)
{
	// if missile was fired clear that position
	if (player.missileFired == 1) {
		playerMissileClear(player.missileX, player.missileY);
	}

	playerClear(player.posX, player.posY);	// clear old position of player

	player.posY = PLAYERPOSY;	// set vertical Position
	player.posX = 0;		// set horizontal Position
	player.speed = 1;
	player.missileFired = 0;
	player.missileX=0;
	player.missileY=0;

	playerDisplay(player.posX, player.posY);	// display new position of player
}


/**
 * move player horizontally to position newPosX
 */
static void playerMove(int newPosX)
{
	playerClear(player.posX, player.posY);	 // clear sprite
	player.posX = newPosX;	 // make movement
  	playerDisplay(player.posX, player.posY); // display sprite
}


/**
 * move player left
 */
void playerMoveLeft(void)
{
	// check if space between player and border of screen is big enough 
	if (player.posX > 0 + player.speed) {
		// desired movement is possible
		playerMove(player.posX - player.speed);
	} else {
		// space too small, move to left-most position
		playerMove(0);
	}
}


/**
 * move player right
 */
void playerMoveRight(void)
{
	// check if space between player and border of screen is big enough 
	if (player.posX < SCREENWIDTH - PLAYERWIDTH - player.speed) {
		// desired movement is possible
		playerMove(player.posX + player.speed);
	} else {
		// space too small, move to right-most position
		playerMove(SCREENWIDTH - PLAYERWIDTH);
	}
}


/**
 * toggle turbo mode on (if key is kept pressed)
 */
void playerTurboOn(void) { 	player.speed = 2;}


/**
 * toggle turbo mode off (if key is kept pressed)
 */
void playerTurboOff(void){	player.speed = 1;}


/**
 * player was hit by an alien shot
 */
int playerHitCheck(int hostileShotX, int hostileShotY)
{
	int fPlayerWasHit = 0;
	// if shot reached vertikal position of player
	if (hostileShotY == PLAYERPOSY) {
		// if shot hits player
		if (hostileShotX >= player.posX && hostileShotX <= player.posX + PLAYERWIDTH -1) {
			fPlayerWasHit = 1;		// set return value
		}
	}
	return fPlayerWasHit;				// return if player was hit
}


/**
 * Launch Missile
 */
void playerLaunchMissile(void)
{
	// only launch missile if no other is on its way
	if (player.missileFired == 0) {
		player.missileFired = 1;	// missile is on its way
		player.missileX = player.posX + PLAYERWIDTH / 2;	// launched from the middle of player...
		player.missileY = PLAYERPOSY;	// ...at same horizontal position
	}
}


/**
 * Reload Missile
 */
static void playerReloadMissile(void)
{
	player.missileFired = 0;	// no player missile flying
}


/**
 * move player missile and do alien/bunker hit testing
 * return value - 0: still some aliens left, 1: no aliens left
 */
int playerMoveMissile(void)
{
	int fNoAliensLeft = 0;	// return value
	int alienTypeHit = 0;

	// only do movements if there is a missile to move
	if (player.missileFired == 1) {
		playerMissileClear(player.missileX, player.missileY);		// clear old missile position
		playerDisplay(player.posX, player.posY); // todo: check if this can be removed later if missiled is fired, 
						//the middle of player is cleared
		player.missileY--;						// move missile

		// if missile out of battlefield
		if (player.missileY < 0) {
			playerReloadMissile();				// reload missile
		} else {
			playerMissileDisplay(player.missileX, player.missileY);	// display missile at new position
		}

		// if missile hits an alien (TODO)
		alienTypeHit = aliensHitCheck(player.missileX, player.missileY);
		if (alienTypeHit != 0) {
			doScoring(alienTypeHit);			// increase score
			playerReloadMissile();				// reload missile
			aliensClear(aliens.posX, aliens.posY, aliens.right, aliens.bottom); // clear old posiiton of aliens
			render();
			if (shipnum == 0) {
				fNoAliensLeft = 1;
			}
			// speed of aliens
			weite = (shipnum + (skill_level * 10) - (level * 5) + 5) / 10;
			if (weite < 0) {
				weite = 0;
			}
			playerMissileClear(player.missileX, player.missileY);	// clear old missile position
			aliensDisplay(aliens.posX, aliens.posY, aliens.right, aliens.bottom);  // display aliens
		}

		// if missile hits a bunker
		if (bunkersHitCheck(player.missileX, player.missileY) == 1) {
			bunkersClearElement(player.missileX, player.missileY);	// clear element of bunker
			playerReloadMissile();				// reload player missile
		}

		// if missile hits ufo
		if (ufoHitCheck(player.missileX, player.missileY) == 1) {
			doScoring(UFO_ALIEN_TYPE);
			playerReloadMissile();
		}
	}

	return fNoAliensLeft;

}

/**
 * let player explode
 */
void playerExplode(void){
	playerExplosionDisplay(player.posX, player.posY);
	playerDisplay(player.posX, player.posY);
}



#if !defined(NVADERS_IN_FED)
 int main ( int argc, char **argv ) {
  inVaders_main( argc, argv );
  return 0;
 }
#endif
#endif
