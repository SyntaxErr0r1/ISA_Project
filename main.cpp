#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stdbool.h>

using namespace std;

void print_usage(){
    fprintf(stderr, "Usage: feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T] [-a] [-u]\n");
    exit(1);
}

int main(int argc, char const *argv[])
{

    bool showTime = false;
    bool showAuthor = false;
    bool showUrls = false;

    if(argc < 2 ){
        fprintf(stderr, "Usage: feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T] [-a] [-u]\n");
        exit(1);
    }
    
    if(argv[1][0] == '-'){

        if(argv[1][1] != 'f')
            print_usage();

        // read from file
        if(argc < 3){
            print_usage();
        }
        FILE *fp = fopen(argv[2], "r");
        if(fp == NULL){
            print_usage();
        }
        fprintf(stdout, "Reading from file %s\n", argv[2]);
    }
    else{
        fprintf(stdout, "Reading from url %s\n", argv[1]);
    }


    for (size_t i = 0; i < argc; i++){
        const char* arg = argv[i];
        
        if(arg[0] == '-'){
            switch (arg[1])
            {
            case 'c':
                fprintf(stdout, "certfile: %s\n", argv[i+1]);
                break;
            case 'C':
                fprintf(stdout, "certaddr: %s\n", argv[i+1]);
                break;
            case 'T':
                fprintf(stdout, "T\n");
                showTime = true;
                break;
            case 'a':
                fprintf(stdout, "a\n");
                showAuthor = true;
                break;
            case 'u':
                fprintf(stdout, "u\n");
                showUrls = true;
                break;
            default:
                print_usage();
                break;
            }
        }
    }
    
    return 0;
}
