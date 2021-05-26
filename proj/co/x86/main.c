#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#define SET_ANSI_COLOR_CYAN    printf("\x1b[36m")
#define SET_ANSI_COLOR_RESET   printf("\x1b[0m")

#define VERBOSE_OUT 0
#define SEND_RESPONSE

const uint8_t tab[] = {0x02, 0x26, 0xff, 0xf4, 0x16, 0xf9, 0x00, 0x01, 0x16, 0xc2, 0x00, 0x00, 0x16, 0xf9, 0x00, 0x00,
			0x16, 0xf9, 0x00, 0x02, 0x16, 0xc2, 0x00, 0x00, 0x16, 0xf9, 0x00, 0x00, 0x02, 0x18, 0x2c, 0x11};

volatile size_t s = 0;
volatile uint8_t buf[128];

struct itimerspec its;
timer_t timerid;
int fuart;

int armTimer()
{
	its.it_value.tv_sec = 1;
	its.it_value.tv_nsec = 800000000;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	if (timer_settime(timerid, 0, &its, NULL) == -1){
		perror("arm Timer: timer_settime");
		return -1;
	}

return 0;
}

int disarmTimer()
{
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = its.it_value.tv_sec ;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	if (timer_settime(timerid, 0, &its, NULL) == -1){
		perror("disarmTimer: timer_settime");
		return -1;
	}

return 0;
}

#define FAN 0x3F
#define SET_TEMP 0x23
#define TEMP1 0x1E
#define TEMP2 0x26
#define TEMP_SPAL 0x42

void clearScreen()
{
	printf("\e[1;1H\e[2J");
}

static void handler(int sig, siginfo_t *si, void *uc)
{

	disarmTimer();
	static unsigned int frame;
	static uint8_t temp[sizeof(buf)/sizeof(buf[0])];

	if( (s > 0x20) || (VERBOSE_OUT) ){

	#if(!VERBOSE_OUT)
		clearScreen();
	#endif
		printf("Frame: %d len: %02x\n", ++frame, (int)s);

		for(int i = 0; i < s; ++i){

	#if(!VERBOSE_OUT)
			if( (buf[i] != temp[i]) && (frame != 0) ){
				SET_ANSI_COLOR_CYAN;
			}
	#endif

			printf("%02x ", buf[i]);
			if( (i+1) % 8 == 0 ){
				if( (i+1) % 16 == 0 ){
					printf("\n");
				}else{
					printf(" ");
				}
			}
			SET_ANSI_COLOR_RESET;
		}
		printf("\n");
	#if(!VERBOSE_OUT)
		printf("\n");
		for(int i = 0; i < s; ++i){

			switch(i){
				case FAN:
					printf("Fan speed %d\n", buf[i]);
					break;
				case SET_TEMP:
					printf("Set temp %d\n", buf[i]);
					break;
				case TEMP1:
					printf("Temp1 %.1f\n", ((uint16_t) (buf[i] << 8) | buf[i+1]) /10.0);
					break;
				case TEMP2:
					printf("Temp2 %.1f\n", ((uint16_t) (buf[i] << 8) | buf[i+1]) /10.0);
					break;
				case TEMP_SPAL:
					printf("Temp Spal %.1f\n", ((uint16_t) (buf[i] << 8) | buf[i+1]) /10.0);
				default:
					break;
			}
		}
		memcpy((void*) &temp[0], (void*) &buf[0], s);
	#endif

	#ifdef SEND_RESPONSE
		write(fuart, (void*) &tab[0], sizeof(tab)/sizeof(tab[0]));
		printf("Sent response\n");
	#endif

	}
	s = 0;
	printf("\n");
}

void *uartRecv(void *v)
{

	int fuart = *((int*) v);

	for(;;){
		ssize_t r = read(fuart, (void*) &buf[s], 1);
		s++;
		if(its.it_value.tv_sec + its.it_value.tv_nsec == 0){
			armTimer();
		}
	}
}

int initUart(int fuart)
{

	struct termios _termios;

	if( tcgetattr(fuart, &_termios) < 0 ){
		perror("tcgetattr: ");
		return -1;
	}

	_termios.c_cc[VMIN] = 1;
	_termios.c_cc[VTIME] = 0;

	cfmakeraw(&_termios);
	if( tcsetattr(fuart, TCSAFLUSH, &_termios) < 0){
		perror("tcsetattr: ");
		return -1;
	}

	return fuart;
}

int main(int argc, char **argv)
{
	if( argc < 2 ){
		perror("No input device");
		return -1;
	}

	fuart = open(argv[1], O_RDWR);
	if( fuart == -1 ){
		printf("Cannot open input device: %s", argv[1]);
		return -1;
	}

	if( initUart(fuart) == -1 ){
		perror("initUart: ");
		return -1;
	}

	struct sigevent sev;
	sigset_t mask;
	struct sigaction sa;

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handler;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGRTMIN, &sa, NULL) == -1){
        perror("sigaction");
		return -1;
	}

	sigemptyset(&mask);
	sigaddset(&mask, SIGRTMIN);
	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1){
		perror("sigprocmask");
		return -1;
	}
	
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGRTMIN;
	sev.sigev_value.sival_ptr = &timerid;
	if( timer_create(CLOCK_REALTIME, &sev, &timerid) == -1 ){
		perror("Timer create error");
		return -1;
	}

	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	if (timer_settime(timerid, 0, &its, NULL) == -1){
		perror("Timer_settime");
		return -1;
	}

	if(sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1){
		perror("Signal Unblock");
		return -1;
	}
	pthread_t th_id;
	if( pthread_create(&th_id, NULL, uartRecv, (void*) &fuart) == -1 ){
		perror("Thread create");
		return -1;
	}

	pthread_join(th_id, NULL);
	timer_delete(timerid);
	close(fuart);
return 0;
}
