#!/bin/bash

output=`curl -X GET -H "Content-Type: application/json" http://localhost:10081/cache_loader/ 2> /dev/null`
#output=`curl -X GET -H "Content-Type: application/json" http://localhost:10081/cache_loader/geo_ad 2> /dev/null`

echo $output

