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

struct location {
    int x;
    int y;
};

struct player {
    struct location currPos;
};

struct obstacle {
	struct location currPos;
};

struct obstacle obstacleArray[MAX_OBSTACLES];

static struct player myPlayer;

static int numObstacles = 0;

void controlPlayer(int i);

void updateObstacles();

void initObstacle(int i);

void initPlayer();

void detectCollision();

void displayGame();

void addObstacle();

void removeObstacle();

#endif /* rgbls_game_h */
