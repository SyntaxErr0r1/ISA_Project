#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdbool.h>
#include <cstring>

#include <openssl/x509v3.h> //x509 implementation for compatibility
#include <openssl/bn.h> // 
#include <openssl/asn1.h>
#include <openssl/x509.h> // x509 implementation
#include <openssl/x509_vfy.h> 
#include <openssl/pem.h> // for reading certificates & keys
#include <openssl/bio.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
// #include <libxml/xmlschemas.h>
// #include <libxml/xmlschemastypes.h>
// #include <libxml/xmlIO.h>
// #include <libxml/xmlmemory.h>
// #include <libxml/xmlstring.h>
// #include <libxml/xmlreader.h>

enum feed_type { 
    FEED_TYPE_UNKNOWN = 0,
    FEED_TYPE_RSS = 1,
    FEED_TYPE_ATOM = 2
};

using namespace std;



void print_usage(){
    fprintf(stderr, "Usage: feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T] [-a] [-u]\n");
    exit(1);
}

void debug_print(const char *msg){
    fprintf(stderr, "DEBUG> %s", msg);
}

void print_author_node(xmlNode *node){
    xmlNode *cur_node = NULL;
    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            if (strcmp((char *)cur_node->name, "name") == 0){
                printf("Author name:  %s ", xmlNodeGetContent(cur_node));
            }
            if (strcmp((char *)cur_node->name, "email") == 0){
                printf("Author email: %s\n", xmlNodeGetContent(cur_node));
            }else{
                printf("\n");
            }
        }
    }
}

void parse_item(xmlNode *item,bool showTime,bool showAuthor,bool showUrls){

    for(xmlNode *cur_node = item->children; cur_node; cur_node = cur_node->next){
            if(!strcmp((char*)cur_node->name, "title")){
                printf("%s\n", xmlNodeGetContent(cur_node));
            }
            else if(!strcmp((char*)cur_node->name, "link")){
                if(showUrls){
                    xmlChar *href = xmlGetProp(cur_node, (const xmlChar *)"href");
                    printf("URL: %s\n", href);
                }
            }
            else if(!strcmp((char*)cur_node->name, "published")){
                if(showTime){
                    printf("Aktualizace: %s\n", xmlNodeGetContent(cur_node));
                }
            }
            else if(!strcmp((char*)cur_node->name, "author")){
                if(showAuthor){
                    print_author_node(cur_node->children);
                }
            }
            // else if(!strcmp((char*)cur_node->name, "description")){
            //     printf("%s\n", xmlNodeGetContent(cur_node));
            // }
    }

    if(showTime || showAuthor || showUrls){
        printf("\n");
    }
}

int main(int argc, char const *argv[])
{

    /*
        Parsing command line arguments
    */


    bool showTime = false;
    bool showAuthor = false;
    bool showUrls = false;

    bool isUrl = false;
    string location = "";

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
            case 'f':
                break;
            default:
                print_usage();
                break;
            }
        }
    }

    /*
        Parsing feed
    */

    xmlDoc *doc = NULL;
    xmlNode *root = NULL;

    LIBXML_TEST_VERSION

    enum feed_type type = FEED_TYPE_UNKNOWN;


    if(isUrl){
        fprintf(stderr, "Reading from url %s\n", argv[1]);
        doc = xmlReadFile(location.c_str(), NULL, 0);
    }
    else{
        fprintf(stderr, "Reading from file %s\n", argv[2]);
        if((doc = xmlReadFile(location.c_str(), NULL, 0)) == NULL){
            fprintf(stderr, "Error: Could not open file %s\n", argv[2]);
            exit(1);
        }

        root = xmlDocGetRootElement(doc);
        if(root == NULL){
            fprintf(stderr, "Error: Could not get root element from file %s\n", argv[2]);
            exit(1);
        }

        if(xmlStrcmp(root->name, (const xmlChar *) "rss") == 0){
            fprintf(stderr, "Feed type: RSS\n");
            type = FEED_TYPE_RSS;
        }
        else if(xmlStrcmp(root->name, (const xmlChar *) "feed") == 0){
            fprintf(stderr, "Feed type: Atom\n");
            type = FEED_TYPE_ATOM;
        }
        else{
            fprintf(stderr, "Error: Unknown feed type\n");
            exit(1);
        }


        xmlNode *cur_node = NULL;

        for(cur_node = root->children; cur_node; cur_node = cur_node->next){
            if(cur_node->type == XML_ELEMENT_NODE){

                if(strcmp("title", (const char *) cur_node->name) == 0){
                    printf("*** %s ***\n", cur_node->children->content);
                }

                if(strcmp("entry", (const char *) cur_node->name) == 0){
                    parse_item(cur_node, showTime, showAuthor, showUrls);
                }
            }
        }

    }



    return 0;
}