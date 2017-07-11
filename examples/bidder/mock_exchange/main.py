#!/usr/bin/env python2
#encoding: UTF-8

# 
# File:   main.py
# Author: arseny.bushev@gmail.com
#
# Created on 13  июня 2017 г., 19:25
#
 
import requests
import threading
import signal
from time import sleep
import argparse
import sys

class Exchange:
    def __init__(self, limit, url, files, timeout, concurrency):
        self.exchange = True 
        self.limit = limit/concurrency
        self.threads = []
        self.concurrency = concurrency
        self.lock = threading.Lock()
        self.threads_finished = 0
        self.exchange_data  = []
        self.url = url
        self.timeout = timeout
        for f in files:
            with open(f, 'rb') as data_file:
                self.exchange_data.append(data_file.read())  
                
    def run(self):
        for c in xrange(0, self.concurrency):
            self.threads.append(threading.Thread(target=self.run_thread))
        for t in self.threads:
            t.start()
        while self.exchange:
            sleep(0.01)

            with self.lock:
                if self.threads_finished == len(self.threads):
                    break
        for t in self.threads:
            t.join()

    def run_thread(self):
        session = requests.Session()
        
        idx = 0
        while self.exchange:
            if self.limit > 0 and idx >= self.limit:
                break
            try:
                r = session.post(self.url, data=self.exchange_data[idx%len(self.exchange_data)], timeout=self.timeout)
            except requests.exceptions.Timeout:
                print('Connection timeout Timeout occured')
            except requests.exceptions.ConnectionError:
                print('Connection failed')
                break
            idx += 1
        with self.lock:
            self.threads_finished += 1

    def stop(self):
        self.exchange = False

parser = argparse.ArgumentParser(description="""Mock exchange""")
parser.add_argument('--limit', help='total requests number, 0 for endless requests (default 0)', default=0)
parser.add_argument('--url', help='exchange url (default http://localhost:9081/bid/123)', default="http://localhost:9081/bid/123")
parser.add_argument('--requests', help='Stored bid requests "file1 [file2] ... [fileN]"')
parser.add_argument('--timeout', help='request timeout (default 0.1 sec)', default=0.1)
parser.add_argument('--concurrency', help='threads to execute (default 5)', default=5)
args = parser.parse_args()

exchange = Exchange(limit=int(args.limit), files=args.requests.split(" "), url=args.url, timeout=float(args.timeout), concurrency=int(args.concurrency)) 
signal.signal(signal.SIGINT, lambda s, f: exchange.stop())            
exchange.run()

