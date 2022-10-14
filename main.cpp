#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdbool.h>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <filesystem>


#include "downloader.hpp"
#include "parser.hpp"


using namespace std;

/**
 * @brief prints the usage of the feedreader
 * 
 */
void print_usage(){
    fprintf(stderr, "Usage: feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T] [-a] [-u]\n");
    exit(1);
}

/**
 * @brief prints a message to stderr
 * 
 */
void debug_print(const char *msg){
    fprintf(stderr, "DEBUG: %s", msg);
}

/**
 * @brief prints an error message to stderr
 * 
 */
void error_print(const char *msg){
    fprintf(stderr, "Error: %s", msg);
    exit(1);
}

int main(int argc, char const *argv[])
{
    // namespace fs = std::filesystem;

    /*
        Parsing command line arguments
    */

    const char *download_location = "./temp";
    bool showTime = false;
    bool showAuthor = false;
    bool showUrls = false;

    bool isUrl = false;
    string location = "";

    string certfile = "";
    string certaddr = "";

    if(argc < 2 ){
        fprintf(stderr, "Error: No URL or file specified.\n");
        print_usage();
    }
    
    if(argv[1][0] == '-'){
        if(argv[1][1] != 'f'){
            fprintf(stderr,"char: %c\n",argv[1][1]);
            print_usage();
        }
        // read from file
        if(argc < 3){
            debug_print("No file specified\n");
            print_usage();
        }
        FILE *fp = fopen(argv[2], "r");
        if(fp == NULL){
            fprintf(stderr, "Error: Could not open file %s\n", argv[2]);
            exit(1);
        }
        fclose(fp);
        
        location = argv[2];

    }
    else{
        isUrl = true;
        location = argv[1];
    }

    for (int i = 0; i < argc; i++){
        const char* arg = argv[i];
        
        if(arg[0] == '-'){
            switch (arg[1])
            {
            case 'c':
                if(argc < i+2){
                    error_print("No certfile specified\n");
                    print_usage();
                }
                certfile = argv[i+1];
                break;
            case 'C':
                if(argc < i+2){
                    error_print("No certaddr specified\n");
                    print_usage();
                }
                certaddr = argv[i+1];
                break;
            case 'T':
                showTime = true;
                break;
            case 'a':
                showAuthor = true;
                break;
            case 'u':
                showUrls = true;
                break;
            case 'f':
                break;
            default:
                print_usage();
                break;
            }
        }
    }

    struct stat info;
    
    if( stat( download_location, &info ) != 0 ){
        int check = mkdir(download_location,0777);
        // check == 0 if directory is created
        if(check){
            printf("Unable to create directory\n");
            exit(2);
        }
    }
    else if( info.st_mode & S_IFDIR ){ 
        
    }
    else{
        printf( "%s is no directory\n", download_location );
        exit(2);
    }


    /*
        Starting to parse the feed
    */
    if(isUrl){
        fprintf(stderr, "Reading from url %s\n", argv[1]);

        struct url url_location = parse_url(location);

        string filename = download_location + string("/") + url_location.host + ".xml";

        if(url_location.is_https)
            download_https_feed(url_location, filename, certfile, certaddr);
        else
            download_http_feed(url_location, filename);

        parse_news_feed_file(filename, showTime, showAuthor, showUrls);
    }else{

    }
    
    
    

    return 0;
}