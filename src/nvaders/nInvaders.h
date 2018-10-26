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

#define BUNKERWIDTH row
#define BUNKERHEIGHT 4
#define BUNKERX 0
#define BUNKERY (SCREENHEIGHT-8)

#define ALIENS_MAX_NUMBER_X 10
#define ALIENS_MAX_NUMBER_Y 5

#define UFOWIDTH 5
#define UFOPOSY 0

void graphicEngineInit();
void finish(int sig);
void aliensClear(int x, int y, int wid, int hgt);
void aliensDisplay(int x, int y, int wid, int hgt);
void aliensMissileClear(int x, int y);
void aliensMissileDisplay(int x, int y);
void aliensRefresh(int level, int *pAliens);
void battleFieldClear();
void bunkersClear();
void bunkersClearElement(int x, int y);
void bunkersDisplay(int *pBunker);
void gameOverDisplay();
void playerClear(int x, int y);
void playerDisplay(int x, int y);
void playerExplosionDisplay(int x, int y);
void playerMissileClear(int x, int y);
void playerMissileDisplay(int x, int y);
void titleScreenClear();
void titleScreenDisplay();
void ufoClear(int x, int y);
void ufoDisplay(int x, int y);
void ufoRefresh();

void statusDisplay(int level, int score, int lives);
void refreshScreen();

extern void doSleep();

extern void showUsage();
extern void showVersion();
extern void showGplShort();
extern void showGpl();

typedef struct Aliens Aliens;

struct Aliens {
	int posX;	  // horizontal position of aliens
	int posY;	  // vertical position of aliens
	int right;
	int left;
	int bottom;
	int speed;	  // 0: no movement; 1: one per turn; etc.
};

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

void aliensReset();
void bunkersReset();	
int aliensMove();
int aliensMissileMove();
void render();
int aliensHitCheck(int shotx, int shoty);
int bunkersHitCheck(int shotx, int shoty);
	
// methods that handle graphic display, from view.c
extern void aliensDisplay(int x, int y, int wid, int hgt);
extern void aliensClear(int x, int y, int wid, int hgt);
extern void aliensRefresh(int level, int *pAliens);
extern void aliensMissileDisplay(int x, int y);
extern void aliensMissileClear(int x, int y);
extern void bunkersClearElement(int x, int y);
extern void bunkersClear();
extern void bunkersRefresh();

typedef struct Ufo Ufo;
		
struct Ufo {
	int posX;	  // horizontal position of aliens
	int posY;	  // vertical position of aliens
};
	
Ufo ufo;

void ufoReset();
int ufoShowUfo();
void ufoMoveLeft();
int ufoHitCheck(int shotX, int shotY);
	
// methods that handle graphic display, from view.c
extern void ufoDisplay(int x, int y);
extern void ufoRefresh();
extern void ufoClear(int x, int y);

void playerReset();

void playerMoveLeft();
void playerMoveRight();
void playerTurboOn();
void playerTurboOff();
int playerHitCheck(int hostileShotX, int hostileShotY);
void playerLaunchMissile();
int playerMoveMissile();
void playerExplode();

// methods that handle graphic display, from view.c
extern void playerInit();
extern void playerDisplay(int x, int y);
extern void playerClear(int x, int y);
extern void playerMissileInit();
extern void playerMissileDisplay(int x, int y);
extern void playerMissileClear(int x, int y);
extern void playerExplosionDisplay(int x, int y);
extern void bunkersClearElement(int x, int y);

extern void doScoring(int alienType);

#define UFO_ALIEN_TYPE   0
#define RED_ALIEN_TYPE   1
#define GREEN_ALIEN_TYPE 2
#define BLUE_ALIEN_TYPE  3

int main(int argv, char **argc);	// hey! it has to start somewhere!

extern void render(void);

void game_over(int a);
void drawscore();

void doScoring(int alienType);

// todo: let's try to not having to declare these "public"
int weite;
int level;
int skill_level;

// included from globals.h
extern void doSleep();
extern void showUsage();
extern void showVersion();
extern void showGplShort();
extern void showGpl();

void inVaders_main(int argc, char **argv);
