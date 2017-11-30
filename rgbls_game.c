//
//  rgbls_game.c
//  
//
//  Created by emherron on 11/30/17.
//
//
#include <stdlib.h>
#include <unistd.h>
#include "rgbls_game.h"


struct player myPlayer;

int numObstacles = 5;
struct obstacles[numObstacles] obstacleArray;
int rightBorder = 5;
int leftBorder = 0;
int top = 10;

void moveRight();
void moveLeft();
void controlPlayer() {
    
    //check if right button has been pressed
    moveRight();
    //check if left button has been pressed
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
   
        int r = rand() % top;
        if (r != myPlayer.currPos.x) {
            tmp.currPos.x = r;
        }
        r = rand() % top;
        if (r != myPlayer.currPos.y) {
            tmp.currPos.y = r;
        }
        obstacleArray[i] = tmp;
    
}

void initPlayer() {
    
    myPlayer.currPos.x = 2;
    myPlayer.currPos.y = 3;
    myPlayer.lives = 3;
    
}


void detectCollision() {
    
    for (int i = 0; i < numObstacles; ++i) {
        if (obstacleArray[i].currPos.x == myPlayer.currPos.x && obstacleArray[i].currPos.y == myPlayer.currPos.y) {
            (myPlayer.lives)--;
            if (myPlayer.lives == 0) {
                //GAME OVER
                //should we just restart the game?
                //how do we want to handle end of game functionality
            }
        }
    }
    
}

//increase level functionality?


void displayGame() {
    
    //might need this to work with the lights, but i am not sure how to do this
    
}

int main(void) {
    
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
