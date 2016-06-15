//
//  gpio.c
//  
//
//  Created by emherron on 11/30/17.
//
//

#include "rgbls_game.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "rgbls_gpio.h"

#include "libsoc_gpio.h"
#include "libsoc_debug.h"


#define LEFT_BUTTON_IN	115  //P927 //Linux ID number for GPIO you wish to use
#define RIGHT_BUTTON_IN	117   //P925

gpio *left_button;
gpio *right_button;

void gpio_initialize() {
	libsoc_set_debug(1);

	left_button = libsoc_gpio_request(LEFT_BUTTON_IN,LS_GPIO_SHARED);
	right_button = libsoc_gpio_request(RIGHT_BUTTON_IN,LS_GPIO_SHARED);

	if (left_button==NULL || right_button==NULL) {
 		printf("libsoc_gpio_request failed");
 		exit(1);
	}

	libsoc_gpio_set_direction(left_button, INPUT);
	libsoc_gpio_set_direction(right_button, INPUT);

	//libsoc_gpio_set_level(left_button, HIGH);
	//libsoc_gpio_set_level(right_button, HIGH);

	libsoc_gpio_set_edge(left_button, FALLING);
	libsoc_gpio_set_edge(right_button, FALLING);

	libsoc_gpio_callback_interrupt(left_button, checkLeftButton, (void*)NULL);
	libsoc_gpio_callback_interrupt(right_button, checkRightButton, (void*)NULL);
	printf("INITIALIZED LIBSOC RESOURCES\n");
}

int checkLeftButton(void* arg) {
    	controlPlayer(1);
	printf("left\n");
	return 0;
}

int checkRightButton(void* arg) {
   	controlPlayer(2);
	printf("right\n");
	return 0;
}








