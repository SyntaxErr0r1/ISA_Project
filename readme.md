todo:
    complicated url
    maybe check if the entry title has already beed printed before printing other info
    fix totally random characters at line 26 of http://www.theregister.com/headlines.atom:
        - when reading from feedfile
        - happens only with http
        - (in http download) putting request into a separate std::string causes weird behavior
        - happens also when downloading from the wiki link (https)

Using HTTP/1.0 (because of weird strings before and after XML content when using 1.1)

## Testing:

RSS :
- https://en.wikipedia.org:443/w/api.php?hidebots=1&days=7&limit=3&hidewikidata=1&action=feedrecentchanges&feedformat=rss

HTTPS:
- https://what-if.xkcd.com/feed.atom
  
HTTP: 
- http://www.theregister.com/headlines.atom