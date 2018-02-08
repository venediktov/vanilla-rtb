#!/bin/bash

PROGNAME=${0##*/}
PROGVERSION=0.9.0

usage()
{
  cat << EO
        Usage: $PROGNAME [options]
               $PROGNAME -o <version> -c

        Increase the .deb file's version number, noting the change in the
       

        Options:
EO
  cat <<EO | column -s\& -t

        -h|--help & show this output
        -V|--version & show version information
EO
}

n=1000
c=10
json_file=REQUEST__SHORT_NOROOT_STRING.json

SHORTOPTS="hvta:c:n:"
LONGOPTS="help,version,auction,auction-any,auction-rapid,mock-bidders,bidder,multi-bidder,ico-bidder,test"

ARGS=$(getopt -s bash --options $SHORTOPTS  \
  --longoptions $LONGOPTS --name $PROGNAME -- "$@" )

eval set -- "$ARGS"

while true; do
   case $1 in
      -h|--help)
         usage
         exit 0
         ;;
      -v|--version)
         echo "$PROGVERSION"
         ;;
      -a)
         shift
         echo "$1"
         ;;
      -n|--num_of_requests)
        shift
        n=$1
        ;;
      -c|--concurrency)
        shift
        c=$1
        ;;
      --mock-bidders)
         CMD=http://localhost:8081/openrtb_handler/mock-bidders/auction/123
         ;;
      --auction)
         CMD=http://localhost:8081/openrtb_handler/auction/123
         json_file=BID_REQUEST_BANNER.json
         ;;
      --auction-any)
         CMD=http://localhost:8081/openrtb_handler/auction-any/123
         json_file=BID_REQUEST_BANNER.json
         ;;
      --auction-rapid)
         CMD=http://localhost:8081/openrtb_handler/auction-rapid/123
         json_file=BID_REQUEST_BANNER.json
         ;;
      --bidder)
         CMD=http://localhost:9081/bid/123
         json_file=BID_REQUEST_BANNER.json
         ;;
      --ico-bidder)
         CMD=http://localhost:9082/ico_bid/123
         json_file=ICO_BID_REQUEST_BANNER.json
         ;;
      --test)
         CMD=http://localhost:9081/test/
         json_file=BID_REQUEST_BANNER.json
         ;;
      --multi-bidder)
         CMD=http://localhost:9090/bid/
         json_file=BID_REQUEST_BANNER.json
         ;;
      --)
         shift
         break
         ;;
      *)
         shift
         break
         ;;
   esac
   shift
done



echo "ab -k -p ${json_file} -T application/json -n ${n} -c ${c} ${CMD}"
ab -k -p ${json_file} -T application/json -n ${n} -c ${c} ${CMD}


