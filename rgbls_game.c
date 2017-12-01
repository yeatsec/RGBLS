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

int max = 10;

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
    
    if (myPlayer.x < rightBorder) {
        (myPlayer.x)++;
    }
    
}
void moveLeft() {
    
    if (myPlayer.x > leftBorder) {
        (myPlayer.x)--;
    }
    
}

void updateObstacles() {
    
    for (int i = 0; i < numObstacles; ++i) {
        if (obstacleArray[i].y > 0) {
            (obstacleArray[i].y)--;
        } else {
            initObstacle(i);
        }
    }
    
}

void initObstacle(int i) {
    
    struct obstacle tmp;
   
        int r = rand() % WIDTH;
        tmp.x = r;
	tmp.y = TOP - 1;
        obstacleArray[i] = tmp;
	printf("obstacle initialized\n");
    
}

void initPlayer() {
    
    myPlayer.x = 0;
    myPlayer.y = 3;
    
}


void detectCollision() {
    
   	for (int i = 0; i < numObstacles; ++i) {
		if (obstacleArray[i].x == myPlayer.x && obstacleArray[i].y == myPlayer.y) {
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
<<<<<<< HEAD
	if (numObstacles < 10) {
=======
	if (numObstacles < max) {
>>>>>>> 17cad59a39cddf4078c3e8298b4d6daa6d085064
		numObstacles++;
		initObstacle(numObstacles-1);
	}
}
}

void removeObstacle()
{
	if (numObstacles > 0) {
		numObstacles--;
	}
}


void setObstacle() {
	numObstacles = 3;
	struct obstacle1;
	obstacle1.x = 2;
	obstacle1.y = 30;
	obstacleArray[0] = obstacle1;
	struct obstacle2;
	obstacle2.x = 1;
	obstacle2.y = 35;
	obstacleArray[1] = obstacle2;
	struct obstacle3;
	obstacle1.x = 0;
	obstacle1.y = 25;
	obstacleArray[2] = obstacle3;
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
	

	
