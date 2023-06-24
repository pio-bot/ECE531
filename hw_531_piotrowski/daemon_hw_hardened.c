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
#define ERR_LOG    -4
#define ERR_SIG    -5


static int reset_signal_handlers_to_default(void){
    unsigned int i;

    for(i = 1; i < NSIG; i++)
    {
         if(i != SIGKILL && i != SIGSTOP)
             signal(i, SIG_DFL);
    }
    return TRUE;
}

void _signal_handler(const int signal){
    char *str;
    int ret;
    switch(signal){
        case SIGHUP:
        case SIGTERM:
        case SIGILL:
        case SIGINT:
        case SIGFPE:
        case SIGABRT:
        case SIGSEGV:
        case SIGBREAK:
          ret = snprintf(str, 26, "Received %s, exiting", signal);
          if ((ret > 0) && (ret < 30)){
            syslog(LOG_INFO, str);
          } else {
            return ERR_LOG;
          }
          closelog();
          exit(OK);
          break;
        default:
          ret = snprintf(str, 27, "Received unexpected signal", i);
          if ((ret > 0) && (ret < 27)){
            syslog(LOG_INFO, str);
          } else {
            return ERR_LOG;
          }
          closelog();
          exit(ERR_SIG);
    }
}


void _do_work(void){
    int i = 0;
    for (;;){
        char *str;
        int ret;
        ret = snprintf(str, 15, "iteration: %d", i);
        if ((ret > 0) && (ret < 15)){
            syslog(LOG_INFO, str);
        } else {
            return ERR_LOG;
        }
	    i++;
	    sleep(1);
    }
}

int main(void){
    // Open syslog
    openlog("Homework2TimeDaemon", LOG_PID | LOG_NDELAY | LOG_NOWAIT, LOG_DAEMON);

    char *str;
    int ret;
    ret = snprintf(str, 15, "starting hw2_d");
    if ((ret > 0) && (ret < 15)){
        syslog(LOG_INFO, str);
    } else {
        return ERR_LOG;
    }
    // Fork to a new process
    pid_t pid = fork();

    // Check for error
    if ( pid < 0){
        ret = snprintf(str, 14, "Error in FORK");
        if ((ret > 0) && (ret < 14)){
            syslog(LOG_INFO, str);
        } else {
            return ERR_LOG;
        }
	    return ERR_FORK;
    }

    if (pid > 0){
        return OK;
    }

    if (setsid() < -1){
        ret = snprintf(str, 16, "Error in setsid");
        if ((ret > 0) && (ret < 16)){
            syslog(LOG_INFO, str);
        } else {
            return ERR_LOG;
        }
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
        ret = snprintf(str, 15, "Error in chdir");
        if ((ret > 0) && (ret < 15)){
            syslog(LOG_INFO, str);
        } else {
            return ERR_LOG;
        }
	    return ERR_CHDIR;
    }
    
    // Reset the signal handlers
    reset_signal_handlers_to_default();

    // Set signal handler
    signal(SIGTERM, _signal_handler);
    signal(SIGHUP, _signal_handler);
    signal(SIGINT, _signal_handler);
    signal(SIGILL, _signal_handler);
    signal(SIGFPE, _signal_handler);
    signal(SIGSEGV, _signal_handler);
    signal(SIGBREAK, _signal_handler);
    signal(SIGABRT, _signal_handler);
    
    // Call the actual stuff you want to do
    _do_work();

    return 0;
}

