#!/usr/bin/env python2
#encoding: UTF-8

# 
# File:   main.py
# Author: arseny.bushev@gmail.com
#
# Created on 13  июня 2017 г., 19:25
#
 
import json
import sys
import urllib
import urllib2
from httplib import BadStatusLine
from socket import timeout
import requests
import threading
import sys

def request(url, file_name):
    headers = {
        "User-Agent": "application/json"
    }
    data = open(file_name, "r")
    if not data:
        print "Failed to open data file"
        return False

    response_data = ""
    req = urllib2.Request(url, data.read(), headers)
    data.close()
    try:
        response = urllib2.urlopen(req, timeout=1)
        response_data = response.read()
    except urllib2.URLError as e:
        print "Failed to open url %s (%s)" % (url, e.reason)
        return False
    except BadStatusLine as e:
        print "Bad status (%s)" % (e)
        return False
    except timeout as e:
        print "Timeout (%s)" % (e)
        return False
    except:
        print "Failed to open url %s (unknown error)" % (url)
        return False
    return len(response_data)

def run_request(count, thread_name):
    #files = ["empty.json", "data.json"]
    session = requests.Session()
    data_file = open('data.json', 'rb')
    json_data = data_file.read()
    idx = 0
    for x in xrange (0, count):
        idx += 1
        #if idx >= len(files):
        #    idx = 0
        try:
            r = session.post("http://localhost:9081/bid/123", data=json_data, timeout=0.1)
        except requests.exceptions.Timeout:
            print('Oops. Timeout occured')
    data_file.close()
        
            
concurrency = 5
threads = []
for c in xrange(0, concurrency):
    threads.append(threading.Thread(target=run_request, args=(100000, "t%d" % c)))
for t in threads:
    t.start()
for t in threads:
    t.join()
