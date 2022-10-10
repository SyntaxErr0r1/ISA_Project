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

enum feed_type type = FEED_TYPE_UNKNOWN;


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
}

/**
 * @brief converts a node name from atom to rss (kind of a dictionary)
 * 
 * @param atom_name the name of the node in atom
 * @return const char* the name of the node in rss
 */
const char* atom_to_rss(const char *atom_name){
    if(!strcmp(atom_name,"feed"))
        return "channel";
    else if(!strcmp(atom_name,"published"))
        return "pubDate";
    else if(!strcmp(atom_name,"entry"))
        return "item";
    else return atom_name;
}

/**
 * @brief compares the name of the given node to the provided name
 * 
 * @param node the node to compare
 * @param name what the name should be
 * @return true if the name is the same
 */
bool check_node_name(xmlNode *node, const char *name){
    if(type == FEED_TYPE_RSS)
        return !strcmp((const char*)node->name, atom_to_rss(name));
    else if(type == FEED_TYPE_ATOM)
        return !strcmp((const char*)node->name, name);
    
    return false;

    // return (xmlStrcmp(node->name, (const xmlChar *)name) == 0);
}

/**
 * @brief Prints the author node
 * 
 * @param node the author node to print
 */
void print_author_node(xmlNode *node){
    xmlNode *cur_node = NULL;
    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            if (check_node_name(cur_node, "author")) {
                printf("Author name:  %s\n", xmlNodeGetContent(cur_node));
            }
            if (check_node_name(cur_node, "email")) {
                printf("Author email: %s\n", xmlNodeGetContent(cur_node));
            }else{
                printf("\n");
            }
        }
    }
}

/**
 * @brief Prints the content of an entry or item node (one article) 
 * 
 * @param item the node to print
 * @param showTime
 * @param showAuthor 
 * @param showUrls 
 */
void parse_item(xmlNode *item,bool showTime,bool showAuthor,bool showUrls){

    for(xmlNode *cur_node = item->children; cur_node; cur_node = cur_node->next){
            if(check_node_name(cur_node,"title")){
                printf("%s\n", xmlNodeGetContent(cur_node));
            }
            else if(check_node_name(cur_node,"link")){
                if(showUrls){
                    if(type == FEED_TYPE_ATOM){
                        printf("URL: %s\n", xmlGetProp(cur_node, (const xmlChar *)"href"));
                    }
                    else if(type == FEED_TYPE_RSS){
                        printf("URL: %s\n", xmlNodeGetContent(cur_node));
                    }
                }
            }
            else if(check_node_name(cur_node,"published")){
                if(showTime){
                    printf("Aktualizace: %s\n", xmlNodeGetContent(cur_node));
                }
            }
            else if(check_node_name(cur_node,"author")){
                if(showAuthor){
                    print_author_node(cur_node->children);
                }
            }
    }
    printf("\n");
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
        Starting to parse the feed
    */

    xmlDoc *doc = NULL;
    xmlNode *root = NULL;

    LIBXML_TEST_VERSION


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

        /*
            Checking if the file is a valid feed
            Determining the type of the feed
        */
        if(xmlStrcmp(root->name, (const xmlChar *) "rss") == 0){
            fprintf(stderr, "Feed type: RSS\n");
            type = FEED_TYPE_RSS;

            //set root to channel
            // root = root->children;
            for (root = root->children; root; root = root->next) {
                if (root->type == XML_ELEMENT_NODE) {
                    if (check_node_name(root, "channel")) {
                        break;
                    }
                }
            }

            if(root == NULL){
                fprintf(stderr, "Error: Could not find channel element in file %s\n", argv[2]);
                exit(1);
            }

            // fprintf(stderr,"root->name: %s\n", root->name);
        }
        else if(xmlStrcmp(root->name, (const xmlChar *) "feed") == 0){
            fprintf(stderr, "Feed type: Atom\n");
            type = FEED_TYPE_ATOM;
        }
        else{
            fprintf(stderr, "Error: Unknown feed type\n");
            exit(1);
        }

        /*
            Parsing the feed
        */
        xmlNode *cur_node = NULL;

        for(cur_node = root->children; cur_node; cur_node = cur_node->next){
            if(cur_node->type == XML_ELEMENT_NODE){
                if(check_node_name(cur_node, "title")){
                    printf("*** %s ***\n", cur_node->children->content);
                }

                if(check_node_name(cur_node, "entry")){
                    parse_item(cur_node, showTime, showAuthor, showUrls);
                }
            }
        }

    }



    return 0;
}