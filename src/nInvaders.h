#if defined(NVADERS_IN_FED)
/**
 * THIS FILE IS MODIFIED, VISIT THE PROJECT PAGE FOR THE ORIGINAL VERSION
 *******************************************************************************
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


/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )            ( )
#define COMMAND( fun )          void fun( )
#define DECLARE_HIT_FUN( fun )          void fun( )
#else
#define args( list )            list
#define COMMAND( fun )          CMD_FUN   fun
#define DECLARE_HIT_FUN( fun )          HIT_FUN   fun
#endif

/*
 * Short scalar types.
 */
#if    !defined(FALSE)
#define FALSE     0
#endif

#if    !defined(TRUE)
#define TRUE     1
#endif

#if    defined(_AIX)
#if    !defined(const)
#define const
#endif
#define unix
#else
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ncurses.h>

#define SCREENHEIGHT row
#define SCREENWIDTH col

#define PLAYERWIDTH 5
#define PLAYERPOSY (SCREENHEIGHT-2)

#define BUNKERWIDTH 80
#define BUNKERHEIGHT 4
#define BUNKERX 0
#define BUNKERY (SCREENHEIGHT-8)

#define ALIENS_MAX_NUMBER_X 10
#define ALIENS_MAX_NUMBER_Y 5

#define UFOWIDTH 5
#define UFOPOSY 0

typedef struct Aliens Aliens;

struct Aliens {
	int posX;	  // horizontal position of aliens
	int posY;	  // vertical position of aliens
	int right;
	int left;
	int bottom;
	int speed;	  // 0: no movement; 1: one per turn; etc.
};

typedef struct Ufo Ufo;

struct Ufo {
	int posX;	  // horizontal position of aliens
	int posY;	  // vertical position of aliens
};

Ufo ufo;

void initLevel args((void));
void evaluateCommandLine args( (int argc, char **argv) );
void showGpl args((void));
void showVersion args((void));
void showUsage args((void));
void FinalScore args((int sig));
void showGplShort args((void));
void readInput args((void));
void setUpTimer args((void));
void handleTimer args((int sig));
void doSleep args((int microseconds));
void battleFieldInit args((void));
void statusInit args((void));
void playerInit args((void));
void playerMissileInit args((void));
void graphicEngineInit args((void));
void finish args((int sig));
void aliensClear args((int x, int y, int wid, int hgt));
void aliensDisplay args((int x, int y, int wid, int hgt));
void aliensMissileClear args((int x, int y));
void aliensMissileDisplay args((int x, int y));
void aliensRefresh args((int level, int *pAliens));
void battleFieldClear args((void));
void bunkersClear args((void));
void bunkersClearElement args((int x, int y));
void bunkersDisplay args((int *pBunker));
void gameOverDisplay args((void));
void playerClear args((int x, int y));
void playerDisplay args((int x, int y));
void playerExplosionDisplay args((int x, int y));
void playerMissileClear args((int x, int y));
void playerMissileDisplay args((int x, int y));
void titleScreenClear args((void));
void titleScreenDisplay args((void));
void ufoInit args((void));
void ufoClear args((int x, int y));
void ufoDisplay args((int x, int y));
void ufoRefresh args((void));
void main_nVaders args(( int argc, char **argv ) );
void statusDisplay args((int level, int score, int lives));
void refreshScreen args((void));
void waitForReturn args((void));
void aliensReset args((void));
void bunkersReset args((void));
int aliensMove args((void));
int aliensMissileMove args((void));
void render args((void));
int aliensHitCheck args((int shotx, int shoty));
int bunkersHitCheck args((int shotx, int shoty));
void ufoReset args((void));
int ufoShowUfo args((void));
void ufoMoveLeft args((void));
int ufoHitCheck args((int shotX, int shotY));
void playerReset args((void));
void playerMoveLeft args((void));
void playerMoveRight args((void));
void playerTurboOn args((void));
void playerTurboOff args((void));
int playerHitCheck args((int hostileShotX, int hostileShotY));
void playerLaunchMissile args((void));
int playerMoveMissile args((void));
void playerExplode args((void));

#define UFO_ALIEN_TYPE   0
#define RED_ALIEN_TYPE   1
#define GREEN_ALIEN_TYPE 2
#define BLUE_ALIEN_TYPE  3

#if !defined(NVADERS_IN_FED)
int main args((int argv, char **argc));	// hey! it has to start somewhere!
#endif

void game_over args((int a));
void drawscore args((void));
void doScoring args((int alienType));
void inVaders_main args((int argc, char **argv));
#endif
