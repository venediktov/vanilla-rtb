#!/bin/bash

output=`curl -X PUT -H "Content-Type: application/json" --data @campaigns.json http://localhost:11081/campaign/id/123 2> /dev/null`
echo $output
output=`curl -X GET -H "Content-Type: application/json" --data @campaigns.json http://localhost:11081/campaign/id/123 2> /dev/null`
echo $output
output=`curl -X GET -H "Content-Type: application/json" --data @campaigns.json http://localhost:11081/campaign/id/ 2> /dev/null`
echo $output


