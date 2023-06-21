/*
 * To compile: gcc -o daemon_hw daemon_hw.c
 * To run: ./daemon_hw
 * To see output: tail -f /var/log/syslog
 */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>

#define OK         0
#define ERR_FORK   -1
#define ERR_SETSID -2
#define ERR_CHDIR  -3

void _signal_handler(const int signal){
    switch(signal){
        case SIGHUP:
          break;
        case SIGTERM:
          syslog(LOG_INFO, "Received SIFTERM, exiting");
          closelog();
          exit(OK);
          break;
        default:
          syslog(LOG_INFO, "Receviced unexpected signal");
    }
}


void _do_work(void){
    int i = 0;
    for (;;){
        syslog(LOG_INFO, "iteration: %d", i);
	i++;
	sleep(1);
    }
}

int main(void){
    // Open syslog
    openlog("Homework2TimeDaemon", LOG_PID | LOG_NDELAY | LOG_NOWAIT, LOG_DAEMON);
    syslog(LOG_INFO, "starting hw2_d");

    // Fork to a new process
    pid_t pid = fork();

    // Check for error
    if ( pid < 0){
        syslog(LOG_ERR, "Error in FORK");
	return ERR_FORK;
    }
    if (pid > 0){
        return OK;
    }

    if (setsid() < -1){
        syslog(LOG_ERR, "Error in setsid");
	return ERR_SETSID;
    }

    // Close file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Change file mode mask
    //umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    umask(0);

    // Set working directory to root directory
    if (chdir("/") < 0){
        syslog(LOG_ERR, "Error in chdir");
	return ERR_CHDIR;
    }
    
    // Set signal handler
    signal(SIGTERM, _signal_handler);
    signal(SIGHUP, _signal_handler);
    
    // Call the actual stuff you want to do
    _do_work();

    return 0;
}

