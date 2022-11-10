#include "feedreader.hpp"

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
void read_from_url(const char* url_add, std::string certfile, std::string certaddr, struct parse_config config){
    struct url url_location = parse_url(url_add);

    // string filename = DOWNLOAD_DIR + url_location.host + ".xml";
    string filename = DOWNLOAD_DIR;
    filename.append("temp.xml");

    fprintf(stderr, "Downloading %s to %s\n", url_add, filename.c_str());

    if(url_location.is_https)
        download_https_feed(url_location, filename, certfile, certaddr);
    else
        download_http_feed(url_location, filename);

    parse_news_feed_file(filename, config);
}

int main(int argc, char *argv[])
{
    /*
        Parsing command line arguments
    */

    struct parse_config config = {
        .show_time = false,
        .show_author = false,
        .show_urls = false
    };

    bool is_single_url = false;
    string location = "";

    string certfile = "";
    string certaddr = "";

    if(argc < 2 ){
        fprintf(stderr, "Error: No URL or file specified.\n");
        print_usage();
    }
    
    int c;
    while ((c = getopt(argc, argv, ":auTf:c:C:h")) != -1)
    {
        switch(c)
        {
            case 'T':
                config.show_time = true;
                break;
            case 'a':
                config.show_author = true;
                break;
            case 'u':
                config.show_urls = true;
                break;
            case 'f':
                location = optarg;
                break;
            case 'c':
                certfile = optarg;
                break;
            case 'C':
                certaddr = optarg;
                break;
            case 'h':
                print_usage();
                break;
            case '?': 
                
                break; 
            default :
                print_usage();
                break;
        }
    }

    for(; optind < argc; optind++){     
        location = argv[optind];
        is_single_url = true;
        fprintf(stderr, "Assuming %s is a URL\n", location.c_str());
    }

    /* 
        Preparing the download directory 
    */
    struct stat info;
    
    if( stat( DOWNLOAD_DIR, &info ) != 0 ){
        int check = mkdir(DOWNLOAD_DIR,0777);
        // check == 0 if directory is created
        if(check){
            printf("Unable to create directory\n");
            exit(2);
        }
    }
    else if( info.st_mode & S_IFDIR ){ 
        //dir already exists
    }
    else{
        printf( "%s is no directory\n", DOWNLOAD_DIR );
        exit(2);
    }


    /*
        Starting to parse the feed(s)
    */
    if(is_single_url){
        read_from_url(location.c_str(), certfile, certaddr, config);
    }else{
        fstream newfile;

        newfile.open(location.c_str(),ios::in);
        if (newfile.is_open()){  
            string tp;
            while(getline(newfile, tp)){

                if (!tp.empty() && tp[tp.size() - 1] == '\r')
                    tp.erase(tp.size() - 1);

                if(tp.empty() || tp.find_first_not_of(' ') == string::npos)
                    continue;

                read_from_url(tp.c_str(), certfile, certaddr, config);
            
            }
            newfile.close();
        }else{
            fprintf(stderr, "Error: Could not open file %s\n", location.c_str());
            exit(1);
        }
    }
    
    return 0;
}