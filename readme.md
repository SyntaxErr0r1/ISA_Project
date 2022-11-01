todo:
    maybe check if the entry title has already beed printed before printing other info
    maybe use one compiler

Quirky workaround:
- PROBLEM: Couldn't compile on eva:
  - fix: rework makefile to include libxml additional linking
- PROBLEM: Cannot run on eva without a segfault (even with just main containing return at the start):
  - fix: compiling using clang++ works just fine
- PROBLEM: clang++ does not link fstream on merlin (wtf)
  - fix: idk, eva works with clang++ and merlin with g++
- IDEA: let's detect freebsd and then use clang if on freebsd
- PROBLEM: freebsd make does not support if
  - fix: use one makefile to run another makefile using gmake (which supports if & is on eva)
- Finally: The program can be compiled on Debian, merlin & eva


Assignment:
    https://www.vut.cz/studis/student.phtml?script_name=zadani_detail&apid=231021&zid=50242


Using HTTP/1.0 (because of weird strings before and after XML content when using 1.1)

## Testing:

RSS :
- https://en.wikipedia.org:443/w/api.php?hidebots=1&days=7&limit=3&hidewikidata=1&action=feedrecentchanges&feedformat=rss

HTTPS:
- https://what-if.xkcd.com/feed.atom
  
HTTP: 
- http://www.theregister.com/headlines.atom