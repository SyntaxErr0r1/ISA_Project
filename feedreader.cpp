#include "feedreader.hpp"

using namespace std;

#define DOWNLOAD_DIR "temp/"

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

/**
 * @brief Downloads and then processes the provided XML resource
 * 
 * @param url_add string of the url
 * @param certfile 
 * @param certaddr 
 * @param showTime 
 * @param showAuthor 
 * @param showUrls 
 */
void read_from_url(const char* url_add, std::string certfile, std::string certaddr, bool showTime, bool showAuthor, bool showUrls){
    struct url url_location = parse_url(url_add);

    // string filename = DOWNLOAD_DIR + url_location.host + ".xml";
    string filename ="temp/temp.xml";

    if(url_location.is_https)
        download_https_feed(url_location, filename, certfile, certaddr);
    else
        download_http_feed(url_location, filename);

    parse_news_feed_file(filename, showTime, showAuthor, showUrls);
}

int main(int argc, char const *argv[])
{
    /*
        Parsing command line arguments
    */

    const char *download_location = "./temp";
    bool showTime = false;
    bool showAuthor = false;
    bool showUrls = false;

    bool isSingleUrl = false;
    string location = "";

    string certfile = "";
    string certaddr = "";

    FILE *fp = NULL;

    if(argc < 2 ){
        fprintf(stderr, "Error: No URL or file specified.\n");
        print_usage();
    }
    
    if(argv[1][0] == '-'){
        if(argv[1][1] != 'f'){
            fprintf(stderr,"char: %c\n",argv[1][1]);
            print_usage();
        }

        // try to open and read from the file
        if(argc < 3){
            debug_print("No file specified\n");
            print_usage();
        }
        fp = fopen(argv[2], "r");
        if(fp == NULL){
            fprintf(stderr, "Error: Could not open file %s\n", argv[2]);
            exit(1);
        }
        fclose(fp);
        
        location = argv[2];

    }
    else{
        isSingleUrl = true;
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
        Starting to parse the feed(s)
    */
    if(isSingleUrl){
        read_from_url(location.c_str(), certfile, certaddr, showTime, showAuthor, showUrls);
    }else{
        fstream newfile;

        newfile.open(location.c_str(),ios::in); //open a file to perform read operation using file object
        if (newfile.is_open()){   //checking whether the file is open
            string tp;
            while(getline(newfile, tp)){ //read data from file object and put it into string.

                if (!tp.empty() && tp[tp.size() - 1] == '\r')
                    tp.erase(tp.size() - 1);

                read_from_url(tp.c_str(), certfile, certaddr, showTime, showAuthor, showUrls);
            
            }
            newfile.close(); //close the file object.
        }
    }
    
    return 0;
}