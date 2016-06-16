//
//  rgbls_game.c
//  
//
//  Created by emherron on 11/30/17.
//
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "rgbls_game.h"

#define TOP	60
#define	WIDTH	4

int rightBorder = 4;
int leftBorder = 0;

void moveRight();
void moveLeft();
void controlPlayer(int i) {
    
    //check if right button has been pressed
	if (i == 2)
		moveRight();
    //check if left button has been pressed
	if (i == 1)
		moveLeft();
    //if no button has been pressed - do nothing
    
}
void moveRight() {
    
    if (myPlayer.currPos.x < rightBorder) {
        (myPlayer.currPos.x)++;
    }
    
}
void moveLeft() {
    
    if (myPlayer.currPos.x > leftBorder) {
        (myPlayer.currPos.x)--;
    }
    
}

void updateObstacles() {
    
    for (int i = 0; i < numObstacles; ++i) {
        if (obstacleArray[i].currPos.y > 0) {
            (obstacleArray[i].currPos.y)--;
        } else {
            initObstacle(i);
        }
    }
    
}

void initObstacle(int i) {
    
    struct obstacle tmp;
   
        int r = rand() % WIDTH;
        tmp.currPos.x = r;
	tmp.currPos.y = TOP - 1;
        obstacleArray[i] = tmp;
	printf("obstacle initialized\n");
    
}

void initPlayer() {
    
    myPlayer.currPos.x = 0;
    myPlayer.currPos.y = 3;
    
}


void detectCollision() {
    
   	for (int i = 0; i < numObstacles; ++i) {
		if (obstacleArray[i].currPos.x == myPlayer.currPos.x && obstacleArray[i].currPos.y == myPlayer.currPos.y) {
			initPlayer();
			numObstacles = 0;
           	 }
        }
    
    
}

//increase level functionality?


void displayGame() {
    
    //might need this to work with the lights, but i am not sure how to do this
    
}

void addObstacle() {
	if (numObstacles < 10) {
		numObstacles++;
		initObstacle(numObstacles-1);
	}
}

void removeObstacle()
{
	if (numObstacles > 0) {
		numObstacles--;
	}
}

/*int main(void) {
    
    //initialize player
    initPlayer();
    
    //initialize obstacles
    for (int i = 0; i < numObstacles; ++i) {
        initObstacle(i);
    }
    
    while (myPlayer.lives != 0) {
        diplayGame() //?
        usleep(500);
        detectCollision();
        controlPlayer();
        updateObstacles();
        //level functionality would go here
    }
    
}
*/
