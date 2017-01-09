#!/bin/bash

curl -X POST -H "Content-Type: application/json" --data @REQUEST__SHORT_NOROOT_STRING.json http://localhost:8081/openrtb_handler/auction/123

