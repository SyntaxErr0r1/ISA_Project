#include "cstring"
#include <string>

#include <openssl/x509v3.h> //x509 implementation for compatibility
#include <openssl/bn.h> // 
#include <openssl/asn1.h>
#include <openssl/x509.h> // x509 implementation
#include <openssl/x509_vfy.h> 
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
};

/**
 * @brief prints an error message to stderr
 * 
 */
void download_error_print(const char *msg);

void download_https_feed(struct url url, string filename, string certfile, string certaddr);

void download_http_feed(struct url url, string filename);

struct url parse_url(string url);
