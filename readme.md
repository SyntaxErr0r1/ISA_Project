# Feed Reader
Author: Juraj Dedič, xdedic07

Project for ISA at FIT VUT

## Description

This program reads the provided RSS 2.0 or Atom feed. The input can be provided as a URL or as a set of urls in a _feedfile_. It outputs the name of the source and the following information about an article:
 - Time of the article (-T)
 - Author information (-a)
 - URL of the article (-u)

Usage:
```
 Usage: feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T] [-a] [-u]
```


Assignment:
    https://www.vut.cz/studis/student.phtml?script_name=zadani_detail&apid=231021&zid=50242

todo:
    maybe check if the entry title has already beed printed before printing other info

Using HTTP/1.0 (because of weird strings before and after XML content when using 1.1)

## Testing:
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