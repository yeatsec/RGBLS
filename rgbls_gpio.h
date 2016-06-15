//
//  gpio.h
//  
//
//  Created by emherron on 11/30/17.
//
//

#ifndef rgbls_gpio_h
#define rgbls_gpio_h

#include <stdio.h>

void gpio_initialize();

int checkLeftButton(void* arg);

int checkRightButton(void* arg);



#endif /* rgbls_gpio_h */
