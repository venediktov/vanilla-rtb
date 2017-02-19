#!/bin/bash

output=`curl -X POST -H "Content-Type: application/json" --data @BID_REQUEST_BANNER.json http://localhost:9081/bid/123 2> /dev/null`
#output=`curl -X POST -H "Content-Type: application/json" --data @REQUEST__SHORT_NOROOT_STRING.json http://localhost:9081/bid/123 2> /dev/null`

echo $output

