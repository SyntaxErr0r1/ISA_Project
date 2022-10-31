#include "downloader.hpp"

void download_https_feed(struct url url, string filename,  string certfile, string certaddr){
    // fprintf(stderr, "Downloading https feed\n");

    if(!url.is_https)
        fprintf(stderr, "Warning: feed is not served over HTTPS\n");

    long res = 1;

    SSL_CTX* ctx = NULL;
    BIO *web = NULL, *out = NULL;
    SSL *ssl = NULL;

    //init_openssl_library();

    const SSL_METHOD* method = TLS_method();
    if(!(NULL != method)) download_error_print("SSL/TLS failed, SSL_Method...");

    ctx = SSL_CTX_new(method);
    if(!(ctx != NULL)) download_error_print("SSL/TLS failed, SSL_CTX_new...");

    /* Cannot fail ??? */
    // SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);

    /* Cannot fail ??? */
    SSL_CTX_set_verify_depth(ctx, 4);

    /* Cannot fail ??? */
    const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(ctx, flags);


    // res = SSL_CTX_use_certificate_file(ctx, "./samples/c/cert.pem", SSL_FILETYPE_PEM);
    // if(!(res == 1)) download_error_print("SSL/TLS failed, SSL_CTX_use_certificate_file...");    
    
    //TODO check if working correctly
    if(certfile != ""){
        // SSL_CTX_load_verify_file(ctx, certfile.c_str());
        res = SSL_CTX_load_verify_locations(ctx, certfile.c_str(),NULL);
        if(!(res == 1)) download_error_print("SSL/TLS failed, SSL_CTX_load_verify_locations (certfile)...");
    }else if(certaddr != ""){
        res = SSL_CTX_load_verify_locations(ctx,NULL,certaddr.c_str());
        if(!(res == 1)) download_error_print("SSL/TLS failed, SSL_CTX_load_verify_locations (certaddr)...");
    }
    else{
        res = SSL_CTX_set_default_verify_paths(ctx);
    }


    if(!(1 == res)) download_error_print("SSL/TLS failed, SSL_CTX_load_verify_locations...");

    web = BIO_new_ssl_connect(ctx);
    if(!(web != NULL)) download_error_print("SSL/TLS failed, BIO_new_sll_connect...");

    res = BIO_set_conn_hostname(web, (url.host + ":" + url.port).c_str());
    if(!(1 == res)) download_error_print("SSL/TLS failed, BIO_set_conn_hostname...");

    BIO_get_ssl(web, &ssl);
    if(!(ssl != NULL)) download_error_print("SSL/TLS failed, BIO_get_ssl...");

    const char* const PREFERRED_CIPHERS = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";
    res = SSL_set_cipher_list(ssl, PREFERRED_CIPHERS);
    if(!(1 == res)) download_error_print("SSL/TLS failed, SSL_set_cipher_list...");

    res = SSL_set_tlsext_host_name(ssl, url.host.c_str());
    if(!(1 == res)) download_error_print("SSL/TLS failed, SSL_set_tlsext_host_name...");

    //new file pointer
    out = BIO_new_file(filename.c_str(), "w");
    if(!(NULL != out)) download_error_print("SSL/TLS failed, BIO_new_fp... (%s)");

    res = BIO_do_connect(web);
    if(!(1 == res)) download_error_print("SSL/TLS failed, BIO_do_connect...");

    res = BIO_do_handshake(web);
    if(!(1 == res)) download_error_print("SSL/TLS failed, BIO_do_handshake...");

    /* Step 1: verify a server certificate was presented during the negotiation */
    X509* cert = SSL_get_peer_certificate(ssl);
    if(cert) { X509_free(cert); } /* Free immediately */
    if(NULL == cert) download_error_print("SSL/TLS failed, SLL_get_peer_certificate...");

    /* Step 2: verify the result of chain verification */
    /* Verification performed according to RFC 4158    */
    res = SSL_get_verify_result(ssl);
    if(!(X509_V_OK == res)) download_error_print("SSL/TLS failed, SSL_get_verify_result...");

    /* Step 3: hostname verification */
    /* An exercise left to the reader */

    // fprintf(stderr,"Requesting resource: %s\n",url.resource.c_str());

    string request = ("GET /" + url.resource + " HTTP/1.0\r\n"
              "Host: " + url.host + "\r\n"
              "Connection: close\r\n\r\n");

    // fprintf(stderr, "Request: %s", request.c_str());

    BIO_puts(web, request.c_str());
    // BIO_puts(out, "\n");

    bool in_header = true;
    int len = 0;
    do
    {
        char buff[2048] = {};
        len = BIO_read(web, buff, sizeof(buff));
        
        if(len > 0){
            if(in_header){
                string header(buff);
                size_t pos = header.find("\r\n\r\n");
                if(pos != string::npos){
                    in_header = false;
                    header = header.substr(pos + 4);

                    BIO_write(out, header.c_str(), header.length());
                }
            } else {
                BIO_write(out, buff, len);
            }
        }
    } while (len > 0 || BIO_should_retry(web));

    if(out)
        BIO_free(out);

    if(web != NULL)
        BIO_free_all(web);

    if(NULL != ctx)
        SSL_CTX_free(ctx);
}



void download_http_feed(struct url url, string filename){
    // fprintf(stderr, "Downloading http feed \n");

    BIO *web = NULL, *out = NULL;
    long res = 1;

    web = BIO_new_connect((url.host + ":" + url.port).c_str());
    if(!(web != NULL)) download_error_print("HTTP failed, BIO_new_connect...");

    res = BIO_do_connect(web);
    if(!(1 == res)) download_error_print("HTTP failed, BIO_do_connect...");

    out = BIO_new_file(filename.c_str(), "w");
    if(!(NULL != out)) download_error_print("HTTP failed, BIO_new_fp...");

    BIO_puts(web, ("GET /" + url.resource + " HTTP/1.0\r\n"
              +"Host: " + url.host + "\r\n"
              +"Connection: close\r\n\r\n").c_str());
    // BIO_puts(out, "\n");

    // fprintf(stderr, "Requesting HTTP: %s\n", ("GET /" + url.resource + " HTTP/1.0\r\n"
    //           "Host: " + url.host + "\r\n"
    //           "Connection: close\r\n\r\n").c_str());

    bool in_header = true;
    int len = 0;

    do
    {
        char buff[2048] = {};
        len = BIO_read(web, buff, sizeof(buff));
        if(len > 0){
            if(in_header){
                string header(buff);
                size_t pos = header.find("\r\n\r\n");
                if(pos != string::npos){
                    in_header = false;
                    header = header.substr(pos + 4);

                    BIO_write(out, header.c_str(), header.length());
                }
            } else {
                BIO_write(out, buff, len);
            }
        }
    } while (len > 0 || BIO_should_retry(web));

    if(out)
        BIO_free(out);

    if(web != NULL)
        BIO_free_all(web);

}

void download_error_print (const char *msg){
    fprintf(stderr, "Download error: %s", msg);
    exit(1);
}

struct url parse_url(string url) {
    struct url parsed_url;
    string host;
    string port;
    string resource;
    bool is_https = false;
    size_t pos = url.find("://");
    
    // check if https or http
    if (pos != string::npos) {
        if (url.substr(0, pos) == "https") {
            is_https = true;
        } else if (url.substr(0, pos) == "http") {
            is_https = false;
        }else{
            download_error_print("Invalid protocol");
        }
    } else {
        is_https = false;
    }

    if (pos != string::npos) {
        url = url.substr(pos + 3);
    }
    pos = url.find("/");
    if (pos != string::npos) {
        host = url.substr(0, pos);
        resource = url.substr(pos + 1);
    } else {
        host = url;
        resource = "";
    }
    pos = host.find(":");
    if (pos != string::npos) {
        port = host.substr(pos + 1);
        host = host.substr(0, pos);
    } else {
        if (is_https) {
            port = "443";
        } else {
            port = "80";
        }
    }
    fprintf(stderr,"resource: %s\n",resource.c_str());
    parsed_url.host = host;
    parsed_url.port = port;
    parsed_url.resource = resource;
    parsed_url.is_https = is_https;

    return parsed_url;
}