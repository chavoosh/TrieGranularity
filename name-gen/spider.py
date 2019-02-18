#----------------------------------------------------------------------------------
# ArMi: A Simple URL Collector on top of Scrapy
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# (c) 2018 - 2019 University of Arizona & University of Michigan
#----------------------------------------------------------------------------------

from scrapy.linkextractors import LinkExtractor
from scrapy.spider import BaseSpider
from scrapy import Request

#-- create an output file to store URLs collected by ArMi --
open('urls.out', 'w')

#-- define spider: spider is a class that Scrapy uses to scrape information from a website (or a group of websites) --
class MySpider(BaseSpider):
    #-- give a name to the Spider --
    name = 'URL_collector'

    #-- input a list of websites from which Spider will begin to crawl --
    with open("starting_urls.txt", "rt") as f:
        start_urls = [url.strip() for url in f.readlines()]

    #-- define parsing method: this method usually parses the response (where the response holds the page content), extracting the scraped data as dicts and also finding new URLs to follow and creating new requests from them --
    def parse(self, response):
	#-- extract the links from a website --
        le = LinkExtractor()
	#-- check the list not be empty -- 
        if le is not None:        
	    for link in le.extract_links(response):
		#-- build a full absolute URL (since the links can be relative) --
	        link.url = response.urljoin(link.url) 
		#-- print urls in output file --
		with open('urls.out', 'a') as f:
 		    f.write(link.url+'\n')
	        #-- recursively follow links --
                yield Request(link.url, callback=self.parse)
