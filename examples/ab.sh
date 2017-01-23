#/bin/bash 

ab -k -p REQUEST__SHORT_NOROOT_STRING.json -T application/json -n 100 -c 10 http://localhost:8081/openrtb_handler/auction/123
