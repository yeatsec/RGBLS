//
//  gpio.h
//  
//
//  Created by emherron on 11/30/17.
//
//

#ifndef gpio_h
#define gpio_h

#include <stdio.h>

void gpio_initialize();

int* checkLeftButton(void* arg);

int* checkRightButton(void* arg);

int* checkMode(void* arg);


#endif /* gpio_h */
