#!/usr/bin/env python3
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
import random 
import sys

class Exchange:
    def __init__(self, args):
        self.concurrency = concurrency=int(args.concurrency)
        self.exchange = True 
        self.limit = int(args.limit)/concurrency
        self.threads = []     
        self.lock = threading.Lock()
        self.threads_finished = 0
        self.exchange_data  = []
        self.url = args.url
        self.timeout = float(args.timeout)
        for f in args.requests.split(" "):
            with open(f, 'r') as data_file:
                self.exchange_data.append(data_file.read())  
        self.geo = [s.split(':') for s in str(args.geo).split(' ')]
        self.size = list(map(lambda s: [int(x) for x in s.split(':')], str(args.size).split()))
        
                
    def run(self):
        for c in range(0, self.concurrency):
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
                geo = random.choice(self.geo)
                size = random.choice(self.size)
                data = str(self.exchange_data[idx%len(self.exchange_data)]) % {
                    "country": geo[0],
                    "city": geo[1],
                    "width": size[0],
                    "height": size[1]
                }
                r = session.post(self.url, data=data, timeout=self.timeout)
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
parser.add_argument('--requests', help='Stored bid requests "file1 [file2] ... [fileN] (default data.json.template)"', default="data.json.template")
parser.add_argument('--timeout', help='request timeout (default 0.1 sec)', default=0.1)
parser.add_argument('--concurrency', help='threads to execute (default 5)', default=5)
parser.add_argument('--geo', help='geo - country1:city1 [country2:city2] ...[countryN:cityN] (default Russia:Moscow)', default="Russia:Moscow")
parser.add_argument('--size', help='size - widthN:heightN [widthN:heightN] ... [widthN:heightN] (default 100:300)', default="100:300")
args = parser.parse_args()

exchange = Exchange(args) 
signal.signal(signal.SIGINT, lambda s, f: exchange.stop())            
exchange.run()

