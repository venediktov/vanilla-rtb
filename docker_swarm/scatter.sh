#!/usr/bin/env bash

if [ "$(uname)" == "Darwin" ]; then
   IP_ADDR="--advertise-addr $(docker-machine ip default)"
   MAC=1 
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
   SUDO=sudo
else
   echo platform $(uname -s) not supported ... 
fi

#else
#    echo "$(expr substr $(uname -s) 1 10) system is not yet supported
#    exit 1
#fi

if [ -z ${MAC+x} ]; then
 #additional commands for Mac OS 
 if docker-machine status  2> /dev/null; then 
   docker-machine upgrade 
 else 
    docker-machine create --driver virtualbox default
 fi
 #to set env corectly for docker commands mac and win only 
 eval $(docker-machine env default)
fi


docker swarm leave --force
#for multi IP networks specify ip to start swarm network
docker swarm init ${IP_ADDR} 

#create local service to share image between nodes 
docker service create --name registry --publish 5000:5000 registry:2

#pull vanilla image if it's not installed on VM
docker pull vanillartb/vanilla-dev

#tag it with localhost for local registry upload
docker tag  vanillartb/vanilla-dev localhost:5000/vanilla-dev

#upload image to local registry accessibe by all swarm nodes 
docker push localhost:5000/vanilla-dev

#remove cached images in the docker
docker image rm vanillartb/vanilla-dev --force
docker image rm localhost:5000/vanilla-dev --force 

#deploy stack to the swarm manager
#docker stack deploy -c docker-compose.yaml vanilla_swarm
docker stack deploy -c  swarm-persist-with-traefik.yaml  vanilla_swarm


 
