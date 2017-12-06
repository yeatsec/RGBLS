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

int rightBorder = 3;
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
    
   	//check if myPlayer has reached the boundary
    if (myPlayer.x < rightBorder) {
	    //if myPlayer has not reached the boundary, move it one position to the right 
        (myPlayer.x)++;
    }
    
}
void moveLeft() {
    
	//check if myPlayer has reached the boundary
    if (myPlayer.x > leftBorder) {
	    //if myPlayer has not reached the boundary, move it on position to the left 
        (myPlayer.x)--;
    }
    
}

void updateObstacles() {
    
	//move all the obstacles down one position, until the obstacle has reached the bottom 
    for (int i = 0; i < numObstacles; ++i) {
        if (obstacleArray[i].y > 0) {
            (obstacleArray[i].y)--;
        } else {
		//if obstacle has reached the bottom, reinitialize the obstacle 
            initObstacle(i);
        }
    }
    
}

void initObstacle(int i) {
    
    struct obstacle tmp;
   
	//generate random number within the boundaries of the width
        int r = rand() % WIDTH;
	//set that random number as the horizontal position of myPlayer
        tmp.x = r;
	//set the top of the board as the vertical position of myPlayer 
	tmp.y = TOP - 1;
        obstacleArray[i] = tmp;
    
}

void initPlayer() {
    
	//starting position of myPlayer
    myPlayer.x = 0;
    myPlayer.y = 3;
    
}


void detectCollision() {
	
	//check if obstacle is in the same position as myPlayer
   	for (int i = 0; i < numObstacles; ++i) {
		if (obstacleArray[i].x == myPlayer.x && obstacleArray[i].y == myPlayer.y) {
			//if there is a collision, reinitialize myPlayer and clear asteroids (aka restart the game)
			initPlayer();
			numObstacles = 0;
           	 }
        }
    
    
}


void displayGame() {
    
}

void addObstacle() {

	//if the max number of obstacles has not been reached, add another obstacle to the game
	if (numObstacles < max) {
		numObstacles++;
		initObstacle(numObstacles-1);
	}
}


void removeObstacle()
{
	//if there is one or more obstacle in the game, remove an obstacle from the game
	if (numObstacles > 0) {
		numObstacles--;
	}
}


void setObstacles() {
	
	//initialize 3 obstacles at once in different positions on the board
	numObstacles = 3;
	struct obstacle obstacle1;
	obstacle1.x = 2;
	obstacle1.y = 30;
	obstacleArray[0] = obstacle1;
	struct obstacle obstacle2;
	obstacle2.x = 1;
	obstacle2.y = 35;
	obstacleArray[1] = obstacle2;
	struct obstacle obstacle3;
	obstacle3.x = 0;
	obstacle3.y = 25;
	obstacleArray[2] = obstacle3;
	
}

	

	
