#!/bin/bash

output=`curl -X POST -H "Content-Type: application/json" --data @REQUEST__SHORT_NOROOT_STRING.json http://localhost:8081/openrtb_handler/auction/123 2> /dev/null`

echo $output

output=`curl -X POST -H "Content-Type: application/json" --data @REQUEST__SHORT_NOROOT_STRING.json http://localhost:8081/openrtb_handler/v2.4/auction/123 2>/dev/null`

echo $output

