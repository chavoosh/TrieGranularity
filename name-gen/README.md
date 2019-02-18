ArMi is a simple URL Collector built on top of Scrapy, a fast high-level screen scraping and web crawling framework.

Dependencies
-----------------------------------------
Python: https://www.python.org/downloads/
Scrapy: https://github.com/scrapy/scrapy/blob/master/docs/intro/install.rst


Running the experiment
-----------------------------------------
Step 1: go to the project's directory ('MyCrawler') and run the spider
    $ scrapy runspider spider.py -s DEPTH_LIMIT=1

NOTE:
  [-s DEPTH_LIMIT] is an optinal field which can be used to limit crawling. DEPTH_LIMIT is the maximum depth that
  will be allowed to crawl for any site. It is zero by default, so no limit will be imposed.

Step 2: remove duplicate URLs
    $ awk '!seen[$0]++' urls.out > output_urls.out