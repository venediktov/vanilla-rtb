#!/bin/bash

output=`curl -X GET -H "Content-Type: application/json" "http://localhost:12081/win/details?price=100&campaign_id=123" 2> /dev/null`
echo $output

output=`curl -X GET -H "Content-Type: application/json" "http://localhost:12081/status.html" 2> /dev/null`

echo $output
