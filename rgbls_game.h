//
//  rgbls_game.h
//  
//
//  Created by emherron on 11/30/17.
//
//

#ifndef rgbls_game_h
#define rgbls_game_h

#define	MAX_OBSTACLES	10

#include <stdio.h>

struct player {
    int x;
    int y;
};

struct obstacle {
	int x;
	int y;
};

struct obstacle obstacleArray[MAX_OBSTACLES];

struct player myPlayer;

int numObstacles;

void controlPlayer(int i);

void updateObstacles();

void initObstacle(int i);

void initPlayer();

void detectCollision();

void displayGame();

void addObstacle();

void removeObstacle();

void setObstacles();

#endif /* rgbls_game_h */
