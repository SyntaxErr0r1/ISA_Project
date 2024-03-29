#include "downloader.hpp"

bool download_https_feed(struct url url, string filename,  string certfile, string certaddr){

    SSL_library_init();

    if(!url.is_https)
        fprintf(stderr, "Warning: feed is not served over HTTPS\n");

    long res = 1;


    SSL_CTX* ctx = NULL;
    BIO *web = NULL;
    SSL *ssl = NULL;

    #if OPENSSL_VERSION_NUMBER < 0x10100000L
    const SSL_METHOD* method = TLSv1_2_method();
    #else
    const SSL_METHOD* method = TLS_method();
    #endif
    
    if(!(NULL != method)){
        download_error_print("TLS method not found");
        https_download_cleanup(ctx, web);
        return false; 
    }    

    ctx = SSL_CTX_new(method);
    if(!(ctx != NULL)) {
        download_error_print("SSL context not created");
        https_download_cleanup(ctx, web);
        return false;
    }

    // SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);

    SSL_CTX_set_verify_depth(ctx, 4);

    const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(ctx, flags);


    // res = SSL_CTX_use_certificate_file(ctx, "./samples/c/cert.pem", SSL_FILETYPE_PEM);
    // if(!(res == 1)) download_error_print("SSL/TLS failed, SSL_CTX_use_certificate_file...");    
    
    //TODO check if working correctly
    if(certfile != ""){
        // SSL_CTX_load_verify_file(ctx, certfile.c_str());
        res = SSL_CTX_load_verify_locations(ctx, certfile.c_str(),NULL);
        if(!(res == 1)){
            download_error_print("Could not load certificate file");
            https_download_cleanup(ctx, web);
            return false;
        }
    }else if(certaddr != ""){
        res = SSL_CTX_load_verify_locations(ctx,NULL,certaddr.c_str());
        if(!(res == 1)){
            download_error_print("Could not load certificate directory");
            https_download_cleanup(ctx, web);
            return false;
        }
    }
    else{
        res = SSL_CTX_set_default_verify_paths(ctx);
    }

    web = BIO_new_ssl_connect(ctx);
    if(!(web != NULL)){
        download_error_print("Could not create BIO");
        https_download_cleanup(ctx, web);
        return false;
    }

    res = BIO_set_conn_hostname(web, (url.host + ":" + url.port).c_str());
    if(!(1 == res)) {
        download_error_print("Could not set hostname");
        https_download_cleanup(ctx, web);
        return false;
    }

    BIO_get_ssl(web, &ssl);
    if(!(ssl != NULL)) {
        download_error_print("Could not get SSL");
        https_download_cleanup(ctx, web);
        return false;
    }

    const char* const PREFERRED_CIPHERS = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";
    res = SSL_set_cipher_list(ssl, PREFERRED_CIPHERS);
    if(!(1 == res)) {
        download_error_print("Could not set cipher list");
        https_download_cleanup(ctx, web);
        return false;
    }

    res = SSL_set_tlsext_host_name(ssl, url.host.c_str());
    if(!(1 == res)) {
        download_error_print("Could not set TLS extension host name");
        return false;
    }

    res = BIO_do_connect(web);
    if(!(1 == res)) {
        download_error_print("Could not connect to host");
        https_download_cleanup(ctx, web);
        return false;
    }

    res = BIO_do_handshake(web);
    if(!(1 == res)) {
        download_error_print("Could not perform TLS handshake");
        https_download_cleanup(ctx, web);
        return false;
    }

    // Verify a server certificate was presented during the negotiation
    X509* cert = SSL_get_peer_certificate(ssl);
    if(cert) { X509_free(cert); } /* Free immediately */
    if(NULL == cert) {
        download_error_print("Could not get a certificate from the host");
        https_download_cleanup(ctx, web);
        return false;
    }

    // Verify the result of chain verification
    res = SSL_get_verify_result(ssl);
    if(!(X509_V_OK == res)) {
        download_error_print("Could not verify host certificate");
        https_download_cleanup(ctx, web);
        return false;
    }

    string request = ("GET /" + url.resource + " HTTP/1.0\r\n"
              "Host: " + url.host + "\r\n"
              "Content-type: application/xml\r\n"
              "Connection: close\r\n\r\n");

    BIO_puts(web, request.c_str());

    string xml = get_body(web);

    std::ofstream file (filename, std::ofstream::out);

    file << xml;

    file.close();

    https_download_cleanup(ctx, web);

    return true;
}

void https_download_cleanup(SSL_CTX* ctx, BIO* web){
    if(web != NULL)
        BIO_free_all(web);
    if(ctx != NULL)
        SSL_CTX_free(ctx);
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
}





bool download_http_feed(struct url url, string filename){

    BIO *web = NULL; 
    long res = 1;

    web = BIO_new_connect((url.host + ":" + url.port).c_str());
    if(!(web != NULL)) {
        download_error_print("Could not create BIO");
        if(web != NULL)
            BIO_free_all(web);
        return false;
    }

    res = BIO_do_connect(web);
    if(!(1 == res)){
        download_error_print("Could not connect to host");
        if(web != NULL)
            BIO_free_all(web);
        return false;
    }

    BIO_puts(web, ("GET /" + url.resource + " HTTP/1.0\r\n"
              +"Host: " + url.host + "\r\n"
              +"Content-type: application/xml\r\n"
              +"Connection: close\r\n\r\n").c_str());
    
    string xml = get_body(web);

    std::ofstream file (filename, std::ofstream::out);

    file << xml;

    file.close();

    if(web != NULL)
        BIO_free_all(web);

    return true;

}

string get_body(BIO *web){
    string output = string();
    int len = 0;

    do
    {
        char buff[4096] = {};
        len = BIO_read(web, buff, sizeof(buff) - sizeof(char));
        if(len > 0){
            output.append(buff);
        }
    } while (len > 0 || BIO_should_retry(web));

    // fprintf(stderr, "Response: %s\n", output.c_str());
    //remove the header
    size_t pos = output.find("\r\n\r\n");

    if(pos != string::npos){
        output = output.substr(pos+4);
    }else{
        fprintf(stderr,"Error: No header found\n");
    }

    return output;
}

void download_error_print (const char *msg){
    fprintf(stderr, "Download error: %s\n", msg);
}

struct url parse_url(string url) {
    struct url parsed_url;
    string host;
    string port;
    string resource;
    bool is_https = false;
    bool is_valid = true;
    size_t pos = url.find("://");
    
    // check if https or http
    if (pos != string::npos) {
        if (url.substr(0, pos) == "https") {
            is_https = true;
        } else if (url.substr(0, pos) == "http") {
            is_https = false;
        }else{
            download_error_print("Invalid protocol");
            is_valid = false;
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
    parsed_url.host = host;
    parsed_url.port = port;
    parsed_url.resource = resource;
    parsed_url.is_https = is_https;
    parsed_url.is_valid = is_valid;

    return parsed_url;
}