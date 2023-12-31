/*
 * To compile for x86: gcc -o hw1 hw1.c -lcurl 
 * To compile for arm: /<buildroot directory>/output/host/usr/bin/arm-linux-gcc -o hw1 hw1.c -lcurl -uClibc -lc
 * OR make -f makefile-arm  NOTE: may need to update path to buildroot in makefile
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define BUFFER_SIZE 100

int main(int argc, char *argv[]) {
    CURL *curl;
    CURLcode res;
    char output_buffer[BUFFER_SIZE] = "";
 
    char *url = NULL;
    char *data = NULL;
    int is_get = 0, is_post = 0, is_put = 0, is_delete = 0;
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--url") == 0 || strcmp(argv[i], "-u") == 0) {
            if (i + 1 < argc) {
                url = argv[i + 1];
                i++;
            } else {
                printf("Missing URL argument.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--post") == 0 || strcmp(argv[i], "-o") == 0) {
	    is_post = 1;
	} else if (strcmp(argv[i], "--get") == 0 || strcmp(argv[i], "-g") == 0) {
	    is_get = 1;
	} else if (strcmp(argv[i], "--put") == 0 || strcmp(argv[i], "-p") == 0) {
	    is_put = 1;
	} else if (strcmp(argv[i], "--delete") == 0 || strcmp(argv[i], "-d") == 0) {
	    is_delete = 1;
	} else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
	    printf("Usage: %s [options] <data>\n", argv[0]);
	    printf("Options:\n");
	    printf("  -u, --url    URL\n");
	    printf("  -o, --post   Perform POST request\n");
	    printf("  -g, --get    Perform GET request\n");
	    printf("  -p, --put    Perform PUT request\n");
	    printf("  -d, --delete Perform DELETE request\n");
	    printf("  -h, --help   Show help\n");
	    return 0;
	} else {
	    // Its the data
	    data = argv[i];
	}
    }
    // Check if URL is provided
    if (url == NULL) {
        printf("URL is required.\n");
        return 1;
    }
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        // Set the URL
	printf("Setting the URL to %s\n", url);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        // Set request type
	if (is_get) {
	    printf("Sending GET\n");
	    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	} else if (is_post) {
	    printf("Sending POST %s\n", data);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
	    if (data != NULL) {
	        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	    } else {
	    	printf("Data not valid\n");
		return 1;
	    }
        } else if (is_put) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
	    printf("Sending PUT %s\n", data);
            if (data != NULL) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	    } else {
	       printf("Data not valid\n");
	       return 1;
	    }
	} else if (is_delete){
	    printf("Sending DELETE %s\n", data);
	    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	    if (data != NULL){
	        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	    } else {
	       printf("Data not valid\n");
	       return 1;
	    }
	} else {
            printf("No valid request type specified\n");
	    return 1;
        }
	//Send the request
	res = curl_easy_perform(curl);
        if (res != CURLE_OK){
            printf("Error in performing request\n");
            return 1;
        } 
        // cleanup
	curl_easy_cleanup(curl);
    }

    return 0;
}


















