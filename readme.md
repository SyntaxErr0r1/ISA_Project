# Feed Reader
Author: Juraj Dedič, xdedic07

Project for ISA at FIT VUT

## Description

This program reads the provided **RSS 2.0** or **Atom feed**. The input can be provided as a URL or as a set of urls in a _feedfile_. It outputs the name of the source and the following information about an article: 
- Article name, url, name or email of the author


## Usage:
```
 Usage: feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T] [-a] [-u]
```
 - Time of the article (-T)
 - Author information (-a)
 - URL of the article (-u)
 - feedfile - file containing URLs of news feed channels
   - each feed channel is at a separate line
 - certfile - a server certificate location
 - certaddr - directory containing certificates

## Compilation
- can be compiled using `make`
- the program is compiled by GNU c++ compiler
- requires libxml2 and openssl


Assignment:
    https://www.vut.cz/studis/student.phtml?script_name=zadani_detail&apid=231021&zid=50242

TODO:
  - ignore commented lines in feedfile
  - check if the entry title has already been printed before printing other info

Using HTTP/1.0 (because of weird strings before and after XML content when using 1.1)

## Operation

Input mode is determined based on the arguments.
Either the url provided as an argument or they are extracted from the *feedfile* 

The program parses the url(s). 

And for each url it runs the download functions. 
The protocol of the resource is determined (HTTP or HTTPS).
If HTTPS is used, the program will use the relevant TLS functions and validate the provided certificates (which can be provided by arguments). 

The program then requests the resource using HTTP version `1.0`. This version was chosen because the reponse in `HTTP/1.1` is affected probably by chunked transfer encoding when using openssl.

The resource is downloaded using openssl functions (also when not using TLS).
Then the resource is loaded from the openssl BIO into std::string and the body (the XML) is separated from the response. The XML is then written into `./temp/temp.xml`

The file is then opened by the parser and the type of the feed (either RSS 2.0 or Atom) is determined. 
The parser iterates through the nodes and finds the relevant nodes and their atributes when necessary.
In cases where the node names or the attributes are different in Atom or RSS 2.0, the node names will be converted to the Atom format for comparison inside the code.

The desired output information is printed in the order it is specified in the source XML.

## Testing
Minimal openssl version tested: `OpenSSL 1.0.2k-fips`
Testing & debugging has been done in following environments:
- Debian 11 in WSL
- freeBSD - eva.fit.vutbr.cz
- CentOS - merlin.fit.vutbr.cz

Feed sources used for testing (some of them): 

RSS :
- https://en.wikipedia.org:443/w/api.php?hidebots=1&days=7&limit=3&hidewikidata=1&action=feedrecentchanges&feedformat=rss

HTTPS:
- https://what-if.xkcd.com/feed.atom
  
HTTP: 
- http://www.theregister.com/headlines.atom

## Test script
`make test` can be used for running the test script

The test script is inside the `test.js` and requires a recent version of Node.js (v12 and above)

### The test script works in the following way:
  - it serves the files in `./tests/` over HTTP at localhost
  - the ./test/ directory must contain `<test-name>.json` files (`<test-name>` should be replaced by the actual name)
  - the script then lists all test JSON files in the directory
  - and for each test file:
    - it runs the test
    - and compares the expected and real outputs 

### Test cases:
  - each test case is identified by a _test file_, named: `<test-name>.json` 
    - `<test-name>` should be replaced by the actual name
  - the test file is in JSON as in example:
  ```json
  {
    "args": "-f tests/test1.feed -T -a -u",
    "feedFile": [
        "what-if.xkcd.com.xml"
    ],
    "feedUrl": [
        "http://what-if.xkcd.com/feed.atom"
    ],
    "returnCode": 0
  }
  ```
  - the **args** param specifies the arguments which will be appended to the binary name when executing the test
  - when including a feedfile for the test case, the feedfile is going to be named `<test-name>.feed`
    - the feedfile will be generated automatically from data in the JSON test file.
    - but the feed file must be **specified in the arguments** with proper path (eg. tests/test4.feed)
  - it then contains either **feedFile** or **feedUrl** param (feedFile will be preferred if both used)
  - *feedFile* is an **array of XML files** (containing the RSS 2.0 or Atom feed) in the `./tests/` directory
  - *feedUrl* is a **single XML** file in the `./tests/` directory  
  - another parameter is returnCode, which is an integer representing the expected exit code of the feedreader
  - An output file must be also present for each test case unless the expected return code is other than 0
    - output file contains the expected `stdout` of the test case
    - must be named `<test-name>.out` (so output file for test1.json is test1.out)
    - if the expected return code is not 0, the output file is ignored