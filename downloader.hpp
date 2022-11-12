#include "cstring"
#include <string>
#include <fstream>

#include <openssl/pem.h> // for reading certificates & keys
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>



using namespace std;

struct url {
    string host;
    string port;
    string resource;
    bool is_https;
    bool is_valid;
};

/**
 * @brief prints an error message to stderr
 * 
 */
void download_error_print(const char *msg);

/**
 * @brief downloads a provided https feed into a file
 * 
 * @param url the HTTPS resource 
 * @param filename filename where it will be saved
 * @param certfile
 * @param certaddr 
 */
bool download_https_feed(struct url url, string filename, string certfile, string certaddr);

/**
 * @brief downloads http feed into file
 * 
 * @param url the HTTP resource
 * @param filename the name of the save file
 */
bool download_http_feed(struct url url, string filename);

/**
 * @brief returns structure containing information about the URL
 * 
 * @param url string of the url
 * @return struct url 
 */
struct url parse_url(string url);

/**
 * @brief returns the body from the received response
 * 
 * @param web BIO* handling the communication
 * @return string containing the body without the header
 */
string get_body(BIO *web);
