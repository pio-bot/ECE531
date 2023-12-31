/*
 * To compile: gcc -o final_project_daemon final_project_daemon.c
 * To run: ./final_project_daemon
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
#include <time.h>
#include <curl/curl.h>
#include <string.h>

#define OK         0
#define ERR_FORK   -1
#define ERR_SETSID -2
#define ERR_CHDIR  -3

// Default config
int time1 = 8, time2 = 12, time3 = 20;
int temp1 = 80, temp2 = 85, temp3 = 78;

// Global curl variables
CURL *curl;
CURLcode res;
char *url = "http://3.21.236.78:80";

char dataOn[20];
char dataOff[20];

void _signal_handler(const int signal){
    switch(signal){
        case SIGHUP:
          break;
        case SIGTERM:
          syslog(LOG_INFO, "Received SIGTERM, exiting");
          closelog();
          exit(OK);
          break;
        default:
          syslog(LOG_INFO, "Receviced unexpected signal");
    }
}

void do_temp_stuff(FILE *temperatureFile, FILE *heaterFile){
    // Open the files
    temperatureFile = fopen("/tmp/temp", "rb");
    heaterFile = fopen("/tmp/status", "wb");

    if (temperatureFile == NULL){
    	syslog(LOG_INFO, "Error in opening temperature file\n");
	    return;
    }
    if (heaterFile == NULL){
        syslog(LOG_INFO, "ERROR in opening heater file\n");
        return;
    }
    
    // Read in the current temp and time
    char buffer[10];
    fgets(buffer, 10, temperatureFile);
    int currentTemp = atoi(buffer);
    int targetTemp;
    time_t currentTime = time(NULL);
    struct tm* ptr = localtime(&currentTime);
    int currentHour = ptr->tm_hour;
    // Set the target temperature
    if (currentHour < time1){
        targetTemp = temp1;
    } else if (currentHour < time2){
        targetTemp = temp2;
    } else if (currentHour < time3){
        targetTemp = temp3;
    } else {
        targetTemp = temp1;
    }

    // Turn the heater on or off
    if (currentTemp < targetTemp){   
        // Turn the heater on 
        syslog(LOG_INFO, "Current temp %d Target temp %d Turning heater on\n", currentTemp, targetTemp);
        snprintf(dataOn, 20,"ON : %d", (int)time(NULL));
	    fputs(dataOn, heaterFile);
    } else {
        //turn the heater off
        syslog(LOG_INFO, "Current temp %d Target temp %d Turning heater off\n", currentTemp, targetTemp);
        snprintf(dataOff, 20,"OFF : %d", (int)time(NULL));
        fputs(dataOff, heaterFile);

    }

    fclose(temperatureFile);
    fclose(heaterFile);

    return;
}

void setup_curl(void){
    // Initialize libcurl
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url);
    }
}


void _do_work(void){
    FILE *temperatureFile;
    FILE *heaterFile;
    int iteration = 0;
    
    while(1){
        syslog(LOG_INFO, "iteration: %d\n", iteration);
        do_temp_stuff(temperatureFile, heaterFile);
        sleep(5);
        iteration++;
    }
    return;
}

int main(int argc, char *argv[]){

    // Parse input
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--config_file") == 0 || strcmp(argv[i], "-c") == 0) {
	        temp1 = atoi(argv[i + 1]);
            temp2 = atoi(argv[i + 2]);
            temp3 = atoi(argv[i + 3]);
            time1 = atoi(argv[i + 4]);
            time2 = atoi(argv[i + 5]);
            time3 = atoi(argv[i + 6]);
            char setpointData[110];
            snprintf(setpointData, 110,"{\"temp1\":\"%d\", \"temp2\":\"%d\",\"temp3\":\"%d\", \"time1\":\"%d\", \"time2\":\"%d\",\"time3\":\"%d\"}", temp1, temp2, temp3, time1, time2, time3);
            setup_curl();
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, setpointData);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            //Send the request
	        res = curl_easy_perform(curl);
            if (res != CURLE_OK){
                
                return 1;
            } 
        // cleanup
	curl_easy_cleanup(curl);
	    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
	        printf("Usage: %s [options]\n", argv[0]);
	        printf("Options:\n");
	        printf("  -c, --config_file    The config file containing the setpoint temperatures\n");
            printf("                       The config file is entered as temp1 temp2 temp3 time1 time2 time3\n");
	        printf("  -h, --help           Show help\n");
	        return 0;
	    } else {
	        // else doesnt matter
	    }
    }

    // Open syslog
    openlog("FinalProjectDaemon", LOG_PID | LOG_NDELAY | LOG_NOWAIT, LOG_DAEMON);
    syslog(LOG_INFO, "starting fp_d");

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

    syslog(LOG_INFO, "Ending program\n");


    return 0;
}

