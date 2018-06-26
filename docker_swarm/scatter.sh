#!/usr/bin/env bash


if [ "$(uname)" == "Darwin" ]; then
   IP_ADDR="--advertise-addr $(docker-machine ip default)"
   SUDO=
   MAC=1 
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
   IP_ADDR=
   SUDO=sudo
else
   echo platform $(uname -s) not supported ... 
fi

#else
#    echo "$(expr substr $(uname -s) 1 10) system is not yet supported
#    exit 1
#fi

if ! [ -z ${MAC+x} ]; then
echo MAC OS detected
 if docker-machine status  2> /dev/null; then 
   docker-machine upgrade 
 else 
    docker-machine create --driver virtualbox default
 fi
 #to set env corectly for docker commands mac and win only 
 eval $(docker-machine env default)
fi


$SUDO docker swarm leave --force

#for multi IP networks specify ip to start swarm network
$SUDO docker swarm init ${IP_ADDR} 

#create local service to share image between nodes 
$SUDO docker service create --name registry --publish 5000:5000 registry:2

#pull vanilla image if it's not installed on VM
$SUDO docker pull vanillartb/vanilla-dev

#tag it with localhost for local registry upload
$SUDO docker tag  vanillartb/vanilla-dev localhost:5000/vanilla-dev

#upload image to local registry accessibe by all swarm nodes 
$SUDO docker push localhost:5000/vanilla-dev

#remove cached images in the docker
$SUDO docker image rm vanillartb/vanilla-dev --force
$SUDO docker image rm localhost:5000/vanilla-dev --force 

#deploy stack to the swarm manager
#$SUDO docker stack deploy -c  swarm-persist-with-traefik.yaml  vanilla_swarm
#$SUDO docker stack deploy -c  swarm-persist-with-haproxy.yaml  vanilla_swarm


 
