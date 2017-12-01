//
//  gpio.c
//  
//
//  Created by emherron on 11/30/17.
//
//

#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define LEFT_BUTTON_IN = 1;  //Linux ID number for GPIO you wish to use
#define RIGHT_BUTTON_IN = 2;
#define MODE_INPUT_IN = 3;
#define ADC_INPUT_IN = 4;

gpio *left_button;
gpio *right_button;
gpio *mode_input;
gpio *adc_input;

void gpio_initialize() {

    left_button = libsoc_gpio_request(LEFT_BUTTON_IN,LS_GPIO_SHARED);
    right_button = libsoc_gpio_request(RIGHT_BUTTON_IN,LS_GPIO_SHARED);
    mode_input = libsoc_gpio_request(MODE_INPUT_IN,LS_GPIO_SHARED);
    adc_input = libsoc_gpio_request(ADC_INPUT_IN,LS_GPIO_SHARED);
    
    if (left_button==NULL || right_button==NULL || mode_input==NULL || adc_input==NULL) {
        printf("libsoc_gpio_request failed");
        exit();
    }
    
    libsoc_gpio_set_direction(left_button, INPUT);
    libsoc_gpio_set_direction(right_button, INPUT);
    libsoc_gpio_set_direction(mode_input, INPUT);
    libsoc_gpio_set_direction(adc_input, INPUT);
    
    libsoc_gpio_set_level(left_button, HIGH);
    libsoc_gpio_set_level(right_button, HIGH);
    libsoc_gpio_set_level(mode_input, HIGH);
    libsoc_gpio_set_direction(adc_input, HIGH);
    
    libsoc_gpio_callback_interrupt(left_button, &checkLeftButton, (void*)NULL);
    libsoc_gpio_callback_interrupt(right_button, &checkRightButton, (void*)NULL);
    libsoc_gpio_callback_interrupt(mode_input, &checkMode, (void*)NULL);
    libsoc_gpio_callback_interrupt(adc_input, &checkADC, (void*)NULL);

}

int* checkLeftButton(void* arg) {
    
    gpio_level currLev = libsoc_gpio_get_level(left_button);
    if (currLev == LOW) {
        //this means button has been pressed and we need to move to the left
    }
    
}

int* checkRightButton(void* arg) {
    
    gpio_level currLev = libsoc_gpio_get_level(right_button);
    if (currLev == LOW) {
        //this means button has been pressed and we need to move to the right
    }
    
}

int* checkMode(void* arg) {
    
    gpio_level currLev = libsoc_gpio_get_level(mode_input);
    if (currLev == LOW) {
        //this means button has been pressed and we need to move to the mode that we are not currently on
    }
}


int* checkADC(void* arg) {
    
    gpio_level currLev = libsoc_gpio_get_level(adc_input);
    if (currLev == LOW) {
        //this means button has been pressed and we need to switch our source of music
    }
    
}





