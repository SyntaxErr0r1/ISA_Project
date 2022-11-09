#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h> 

#include <sys/types.h>
#include <sys/stat.h>

#include <filesystem>

#include "downloader.hpp"
#include "parser.hpp"

void print_usage();

void debug_print(const char *msg);

void error_print(const char *msg);

void read_from_url(const char* url_add, std::string certfile, std::string certaddr, bool showTime, bool showAuthor, bool showUrls);

int main(int argc, char *argv[]);