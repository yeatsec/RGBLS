#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h> // memset

#include "fftlib.h"
#include "opc_client.h"
#include "rgbls_game.h"

// constants
#define	BUFF_BITS	5
#define	BUFF_SIZE	32
#define	STRIP_LENGTH	60
#define	NUM_STRIPS	4
#define	MATRIX_STRIP_LENGTH	256
#define MATRIX_WIDTH	32	// number of columns
#define MATRIX_HEIGHT	8	// number of rows

//int GAME_MODE = 1;
int GAME_INITIALIZED = 0;

#define	PORT	7890
#define	SERVER_ADDRESS	"::1"
#define	ADC0_PATH	"/sys/bus/iio/devices/iio:device0/in_voltage0_raw"

#define USEC	1000	// desired adc sampling period in microseconds

// double-buffer data members
double buff[2][BUFF_SIZE];
sem_t adc_finished;
sem_t fft_finished;

// rgb_strip resources
rgb strips[NUM_STRIPS][STRIP_LENGTH];
rgb matrix[MATRIX_STRIP_LENGTH];
rgb total_strip[5 * MATRIX_STRIP_LENGTH];

// sampling resources
FILE * f0;

// control adc
sem_t timer_sem;

// wrapper to account for snaking-strip structure of the LED matrix
void matrix_wrapper_write(unsigned int col, unsigned int row, rgb * _color)
{
    unsigned int index = 0;
    if (col%2==0)	// col is even-indexed
    {
        // col must be going 'down'
        index = (col+1)*MATRIX_HEIGHT - row - 1;
    }
    else
    {
        // col must be going 'up'
        index = col*MATRIX_HEIGHT + row;
    }
    //printf("index: %u\n", index);
    //printf("matrix: red %d green %d blue %d\n",(int) _color->red,(int) _color->green,(int) _color->blue);
    matrix[index].red = _color->red;
    matrix[index].green = _color->green;
    matrix[index].blue = _color->blue;
}

/*
 * int adc(unsigned int chan)
 * This function was written with help from Robert Edwards' blog.
 * Available: embedded-basics.blogspot.com/2014/10/beaglebone-black-analog-input-pins.html
 */

int adc(void)
{
    char value_str[7];
    fread(&value_str, 6, 6, f0);
    rewind(f0);
    return strtol(value_str, NULL, 0);
}

rgb negate_color(rgb * orig)
{
    rgb temp;
    temp.red = orig->blue;
    temp.green = orig->red;
    temp.blue = orig->green;
    return temp;
}

rgb calculate_color(unsigned int index)
{
    rgb temp;
    if (index < 4) // red full
    {
        temp.red = 0xFF;
        temp.green = 0x00;
        temp.blue = index * 63;
    }
    else if (index < 8)
    {
        temp.red = 0xFF - ((index-4)*63);
        temp.green = 0x00;
        temp.blue = 0xFF;
    }
    else if (index < 12)
    {
        temp.red = 0;
        temp.green = (index - 8) * 63;
        temp.blue = 0xFF;
    }
    else
    {
        temp.red = 0;
        temp.green = 0xFF;
        temp.blue = 0xFF - ((index - 12) * 63);
    }
    return temp;
}

static void timersignalhandler(int sig)
{
    sem_post(&timer_sem);
}

static void timersignalignore(int sig)
{
}

static void * adc_routine(void * arg)
{
    // initialize both sigaction structs to configure each timersignal handler
    static struct sigaction sa_ignore;
    memset(&sa_ignore, 0, sizeof(sa_ignore));
    sa_ignore.sa_handler = timersignalignore;
    static struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = timersignalhandler;
    if(sigaction(SIGALRM, &sa, NULL))
        printf("set to timersignalhandler fail\n");
    printf("entered adc_routine, registering timersignalhandler and starting timer\n");
    // start timer
    struct itimerval tval = {
        .it_interval = { .tv_sec = 0, .tv_usec = USEC},
        .it_value = { .tv_sec = 0, .tv_usec = USEC}
    };
    setitimer(ITIMER_REAL, &tval, (struct itimerval*)0); /* start timer */
    unsigned int buff_index = 0; // initialize to first buffer
    unsigned int elem_index = 0; // first elem
    while(1)
    {
        int rc = sem_wait(&timer_sem);
        if (rc == -1 && errno == EINTR)
            continue;
        if (rc == -1)
        {
            printf("timer_routine failed on sem_wait\n");
            exit(-1);
        }
        // read the adc and update double-buffer
        double value = (double) adc();
        //printf("%f\n", value);
        buff[buff_index][elem_index] = value;
        elem_index = (++elem_index)%BUFF_SIZE;
        //printf("%f\n");				// REMOVE LATER
        // if the double-buffer is full, wait for fft to send data and then swap
        if (!elem_index)	// looped around and is full
        {
            if(sigaction(SIGALRM, &sa_ignore, NULL))
                printf("set to timersignalignore fail\n");
            // printf("registered SIGALARM to ignore\n");
            sem_post(&adc_finished);	// signal that the adc buffer is filled
            int fftr = sem_wait(&fft_finished);	// block if fft not finished
            if (fftr == -1 && errno != EINTR)
            {
                printf("sem_wait(&fft_finished) error\n");
                exit(420);
            }
            //printf("adc thread through\n");
            // do swap
            buff_index = (++buff_index)%2;
            // finished with swap
            if(sigaction(SIGALRM, &sa, NULL))
                printf("set to timersignalhandler fail\n");
        }
    }
    return NULL;
}

static void * fft_routine(void * arg)
{
    printf("entered fft_routine, blocking SIG_ALRM\n");
    // deregister signal for self
    sigset_t no_sigalrm;
    if(sigemptyset(&no_sigalrm))
        printf("unable to empty sig set for fft_thread\n");
    if(sigaddset(&no_sigalrm, SIGALRM))
        printf("unable to configure sigset_t for fft_thread\n");
    if(sigprocmask(SIG_BLOCK, &no_sigalrm, NULL))
        printf("unable to mask sigalrm for fft_thread\n");
    unsigned int buff_index = 1; // initialize to second arr
    while(1)
    {
        // calculate fft
        if (fftlib_spectra(buff[buff_index]))
            continue;
        unsigned int max_index = 1;
        double max = 0.0;
        for (unsigned int i = 1; i < BUFF_SIZE/2; ++i)
        {
            if (buff[buff_index][i] > max)
            {
                max = buff[buff_index][i];
                max_index = i;
            }
        }
        // once fft finished, calculate colors
        //printf("calculate color\n");
        rgb color = calculate_color(max_index);
        //printf("negative\n");
        rgb negative = negate_color(&color);
        //printf("negative calculated\n");
        if(!GAME_INITIALIZED) {
            initPlayer();
            GAME_INITIALIZED = 1;
        }
        for (unsigned int mag2_index = 0; mag2_index < BUFF_SIZE/2; ++mag2_index)
        {
            unsigned int bar_height = MATRIX_HEIGHT * buff[buff_index][mag2_index] / max;
            if (bar_height > 8)
                bar_height = 8;
            for (unsigned int row_pixel = 0; row_pixel < bar_height; ++row_pixel)
            {
                matrix_wrapper_write(2*mag2_index, row_pixel, &color);
                matrix_wrapper_write(2*mag2_index + 1, row_pixel, &color);
            }
            for (unsigned int row_pixel = bar_height; row_pixel < MATRIX_HEIGHT; ++row_pixel)
            {
                matrix_wrapper_write(2*mag2_index, row_pixel, &negative);
                matrix_wrapper_write(2*mag2_index + 1, row_pixel, &negative);
            }
        }
        //printf("setting strips\n");
        // set strips
        for (unsigned int strip_index = 0; strip_index < NUM_STRIPS; ++strip_index)
        {
            for (unsigned int led_index = 0; led_index < STRIP_LENGTH; ++led_index)
            {
                total_strip[led_index + (strip_index*MATRIX_STRIP_LENGTH)].red = color.red;
                total_strip[led_index + (strip_index*MATRIX_STRIP_LENGTH)].green = color.green;
                total_strip[led_index + (strip_index*MATRIX_STRIP_LENGTH)].blue = color.blue;
            }
        }
        if(GAME_INITIALIZED) {
            //controlPlayer();
            if (max_index == 1 || max_index == 2) {
                printf("obstacle added\n");
		addObstacle();
            }
            total_strip[myPlayer.currPos.y + (myPlayer.currPos.x*MATRIX_STRIP_LENGTH)].red = 0xFF;
            total_strip[myPlayer.currPos.y + (myPlayer.currPos.x*MATRIX_STRIP_LENGTH)].green = 0xFF;
            total_strip[myPlayer.currPos.y + (myPlayer.currPos.x*MATRIX_STRIP_LENGTH)].blue = 0xFF;
            for (int i = 0; i < numObstacles; ++i) {
                total_strip[obstacleArray[i].currPos.y + (obstacleArray[i].currPos.x*MATRIX_STRIP_LENGTH)].red = negative.red;
                total_strip[obstacleArray[i].currPos.y + (obstacleArray[i].currPos.x*MATRIX_STRIP_LENGTH)].green = negative.green;
                total_strip[obstacleArray[i].currPos.y + (obstacleArray[i].currPos.x*MATRIX_STRIP_LENGTH)].blue = negative.blue;
            }
            detectCollision();
            updateObstacles();
        }
        //printf("filling up total_strip\n");
        for (unsigned int led_index = 0; led_index < MATRIX_STRIP_LENGTH; ++led_index)
        {
            unsigned int index = led_index + (NUM_STRIPS * MATRIX_STRIP_LENGTH);
            total_strip[index].red = matrix[led_index].red;
            total_strip[index].green = matrix[led_index].green;
            total_strip[index].blue = matrix[led_index].blue;
        }
        //printf("about to send formatted\n");
        if (opc_client_send_formatted((char) 0, 0, total_strip))
            printf("opc_client_send_formatted error\n");
        // wait for adc_thread if needed
        //printf("fft thread made it to post\n");
        sem_post(&fft_finished);
        int adc_ret = sem_wait(&adc_finished);
        if (adc_ret && errno != EINTR)
        {
            printf("sem_wait(&adc_finished) failed\n");	// sem_wait interrupted by signal
        }
        //printf("fft thread through\n");
        // signal buffer swap
        buff_index = (++buff_index)%2;
    }
    return NULL;
}

int main(void)
{
    sem_t program_over;
    if (sem_init(&program_over, 0, 0))
    {
        printf("program_over init failed\n");
        return 1;
    }
    // initialize strip resources
    /*for (int i = 0; i < NUM_STRIPS; ++i)
     {
     opc_client_rgb_strip_init(&(strips[i]), STRIP_LENGTH);
     }
     opc_client_rgb_strip_init(&matrix, MATRIX_STRIP_LENGTH);
     opc_client_rgb_strip_init(&total_strip, MATRIX_STRIP_LENGTH * 5);
     */
    // initialize sampling resources
    //adc_fds[0] = open(ADC0_PATH, O_RDONLY);
    //adc_fds[1] = open(ADC1_PATH, O_RDONLY);
    //sampling_channel = 0;
    f0 = fopen(ADC0_PATH, "r");
    //if (adc_fds[0] < 0 || adc_fds[1] < 0)
    //{
    //	printf("adc open failure\n");
    //	return 1;
    //}
    // set up network connection with OPS server
    if(opc_client_init(PORT, SERVER_ADDRESS))
    {
        return 2;
    }
    // initialize timer semaphore to zero, register signal
    if (sem_init(&timer_sem, 0, 0))
    {
        printf("sem_init error\n");
        return 3;
    }
    printf("registering SIGALRM\n");
    // initialize semaphores
    if (sem_init(&adc_finished, 0, 0))
    {
        printf("adc semaphore init error\n");
        return 4;
    }
    if (sem_init(&fft_finished, 0, 0))
    {
        printf("fft semaphore init error\n");
        return 5;
    }
    //f0 = fopen(ADC0_PATH, "r");
    
    // initialize pthreads and send them on their way
    pthread_t adc_thread;
    pthread_t fft_thread;
    if (pthread_create(&adc_thread, NULL, adc_routine, NULL))
    {
        printf("adc_thread create failed\n");
        return 6;
    }
    sigset_t no_alrm;
    if(sigemptyset(&no_alrm))
        printf("main thread unable to clear no_alrm set\n");
    if(sigaddset(&no_alrm, SIGALRM))
        printf("main thread unable to add no_alrm to set\n");
    if(sigprocmask(SIG_BLOCK, &no_alrm, NULL))
        printf("main thread unable to block SIG_ALRM\n");
    printf("created adc thread\n");
    if (pthread_create(&fft_thread, NULL, fft_routine, NULL))
    {
        printf("fft_thread create failed\n");
        return 7;
    }
    printf("created fft thread \n");
    // TODO - initialize button input
    while(sem_wait(&program_over)==-1 && errno == EINTR)
        continue;
    printf("deallocating resources\n");
    // close server
    if (opc_client_close())
    {
        printf("client close error\n");
        return 6;
    }
    
    //for (int i = 0; i < NUM_STRIPS; ++i)
    //{
    //	opc_client_rgb_strip_destroy(&(strips[i]));
    //}
    //opc_client_rgb_strip_destroy(&matrix);
    fclose(f0);	// close file descriptors for adcs
    return 0;
}
