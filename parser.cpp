#include "parser.hpp"


feed_type get_feed_type(xmlNode *root){
    feed_type type = FEED_TYPE_UNKNOWN;
    if(xmlStrcmp(root->name, (const xmlChar *) "rss") == 0){
        fprintf(stderr, "Feed type: RSS\n");
        type = FEED_TYPE_RSS;

        if(root == NULL){
            fprintf(stderr, "Error: Could not find channel element in file.\n");
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
    return type;
}

xmlNode* get_channel_node(feed_type type, xmlNode *root){
    //set root to channel
    // root = root->children;
    for (root = root->children; root; root = root->next) {
        if (root->type == XML_ELEMENT_NODE) {
            if (check_node_name(type, root, "channel")) {
                break;
            }
        }
    }
    return root;
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
bool check_node_name(feed_type type, xmlNode *node, const char *name){
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
void print_author_node(feed_type type, xmlNode *node){
    xmlNode *cur_node = NULL;
    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            if (check_node_name(type, cur_node, "author")) {
                printf("Author name:  %s\n", xmlNodeGetContent(cur_node));
            }
            if (check_node_name(type, cur_node, "email")) {
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
void parse_item(feed_type type, xmlNode *item, bool showTime,bool showAuthor,bool showUrls){

    for(xmlNode *cur_node = item->children; cur_node; cur_node = cur_node->next){
            if(check_node_name(type, cur_node,"title")){
                printf("%s\n", xmlNodeGetContent(cur_node));
            }
            else if(check_node_name(type, cur_node,"link")){
                if(showUrls){
                    if(type == FEED_TYPE_ATOM){
                        printf("URL: %s\n", xmlGetProp(cur_node, (const xmlChar *)"href"));
                    }
                    else if(type == FEED_TYPE_RSS){
                        printf("URL: %s\n", xmlNodeGetContent(cur_node));
                    }
                }
            }
            else if(check_node_name(type, cur_node,"published")){
                if(showTime){
                    printf("Aktualizace: %s\n", xmlNodeGetContent(cur_node));
                }
            }
            else if(check_node_name(type, cur_node,"author")){
                if(showAuthor){
                    print_author_node(type, cur_node->children);
                }
            }
    }
    printf("\n");
}

void parse_feed(feed_type type, xmlNode *node, bool showTime,bool showAuthor,bool showUrls){
    xmlNode *cur_node = NULL;
    for(cur_node = node->children; cur_node; cur_node = cur_node->next){
        if(cur_node->type == XML_ELEMENT_NODE){
            if(check_node_name(type, cur_node, "title")){
                printf("*** %s ***\n", cur_node->children->content);
            }

            if(check_node_name(type, cur_node, "entry")){
                parse_item(type, cur_node, showTime, showAuthor, showUrls);
            }
        }
    }
}

void parse_news_feed_file(std::string location, bool showTime,bool showAuthor,bool showUrls){
    
    enum feed_type type = FEED_TYPE_UNKNOWN;

    xmlDoc *doc = NULL;
    xmlNode *root = NULL;

    LIBXML_TEST_VERSION

    fprintf(stderr, "Reading from file %s\n", location.c_str());
    if((doc = xmlReadFile(location.c_str(), NULL, 0)) == NULL){
        fprintf(stderr, "Error: Could not open file %s\n", location.c_str());
        exit(1);
    }

    root = xmlDocGetRootElement(doc);
    if(root == NULL){
        fprintf(stderr, "Error: Could not get root element from file %s\n", location.c_str());
        exit(1);
    }

    /*
        Checking if the file is a valid feed
        Determining the type of the feed
    */
    type = get_feed_type(root);

    if(type == FEED_TYPE_RSS){
        root = get_channel_node(type, root);
    }

    /*
        Parsing the feed
    */
    parse_feed(type, root, showTime, showAuthor, showUrls);

    xmlFreeDoc(doc);       // free document
    xmlCleanupParser();    // Free globals
}