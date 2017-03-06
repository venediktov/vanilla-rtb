#/bin/bash 

ab -k -p BID_REQUEST_BANNER.json -T application/json -n 1000 -c 100 http://localhost:9090/bid/
