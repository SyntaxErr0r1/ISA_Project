#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <stdbool.h>
#include <string>

#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace std;

enum feed_type { 
    FEED_TYPE_UNKNOWN = 0,
    FEED_TYPE_RSS = 1,
    FEED_TYPE_ATOM = 2
};


/**
 * @brief converts a node name from atom to rss (kind of a dictionary)
 * 
 * @param atom_name the name of the node in atom
 * @return const char* the name of the node in rss
 */
const char* atom_to_rss(const char *atom_name);


/**
 * @brief compares the name of the given node to the provided name
 * 
 * @param node the node to compare
 * @param name what the name should be
 * @return true if the name is the same
 */
bool check_node_name(feed_type type, xmlNode *node, const char *name);

/**
 * @brief Prints the author node
 * 
 * @param node the author node to print
 */
void print_author_node(feed_type type, xmlNode *node);

/**
 * @brief Prints the content of an entry or item node (one article) 
 * 
 * @param item the node to print
 * @param showTime
 * @param showAuthor 
 * @param showUrls 
 */
void parse_item(feed_type type, xmlNode *item, bool showTime,bool showAuthor,bool showUrls);

/**
 * @brief Prints the content of a channel or feed node
 * 
 * @param type if the feed is rss or atom
 * @param node the node to print
 * @param showTime
 * @param showAuthor 
 * @param showUrls 
 */
void parse_feed(feed_type type, xmlNode *node, bool showTime,bool showAuthor,bool showUrls);


/**
 * @brief Prints the content of a rss or atom feed from a file
 * @note the file containing the actual feed, not the feedfile in assignment (which is a list of urls)
 * 
 * @param location 
 * @param showTime 
 * @param showAuthor 
 * @param showUrls 
 */
void parse_news_feed_file(std::string location, bool showTime,bool showAuthor,bool showUrls);

/**
 * @brief determines if the feed is rss or atom
 * 
 * @param root the root node of the feed
 * @return feed_type the type of the feed
 */
feed_type get_feed_type(xmlNode *root);

/**
 * @brief Get the channel node object (used in rss)
 * 
 * @param type type of the feed
 * @param root 
 * @return the channel node 
 */
xmlNode* get_channel_node(feed_type type, xmlNode *root);

/**
 * @brief Returns the node content and frees the node
 * 
 */
string get_node_content_string(xmlNode *node);