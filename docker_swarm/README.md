### This is work in progress more of an R&D effort to replicate bidders on the cloud - needs performance tuneup  

<p style="color:grey;font-size:150%;font-family:verdana;font-style:italic">
 In our examples we created docker swarm container replication we run examples on the single host single manager node 
 In production environment one can scatter across different hosts see  `docker swarm join-token` command 
 In this case replication can be replaced by ( see --global flag ) and each node will run single container , in this case
 docker loadbalancing occurs between physical hosts running single container rather then multiple containers running on single host 
</p>

#### Installing docker swarm run scatter.sh 

```bash
bash -x scatter.sh
```

#### This command will show you all services deployed to swarm network 

```bash
docker service ls
```
 
If you see REPLICAS 0/5 instead of 5/5 give it some time , if it's still 0/5 change  --loglevel=DEBUG in swarm-persist-with-traefik.yaml
and rerun scatter.sh again. See 'Checking the logs' for more details 

```
ID                  NAME                         MODE                REPLICAS            IMAGE                               PORTS
c572yhpfkobp        vanilla_swarm_bidder         replicated          5/5                 localhost:5000/vanilla-dev:latest   *:0->9081/tcp
m94d32qjo4ya        registry                     replicated          1/1                 registry:2                          *:5000->5000/tcp
nyigeppeimn8        vanilla_swarm_loadbalancer   replicated          1/1                 traefik:latest                      *:80->80/tcp,*:9090->8080/tcp
macroots-MacBook-Air:docker_swarm vvenedik$
```

```bash
docker service ps vanilla_swarm_bidder
```

#### This command will show you how many bidders replicated

```
ID                  NAME                     IMAGE                               NODE                DESIRED STATE       CURRENT STATE           ERROR               PORTS
2iqnwf4pl9l5        vanilla_swarm_bidder.1   localhost:5000/vanilla-dev:latest   default             Running             Running 2 minutes ago                       
lkwgc3hb303i        vanilla_swarm_bidder.2   localhost:5000/vanilla-dev:latest   default             Running             Running 2 minutes ago                       
aggdy5ezj92x        vanilla_swarm_bidder.3   localhost:5000/vanilla-dev:latest   default             Running             Running 2 minutes ago                       
jkuh8gthxaqh        vanilla_swarm_bidder.4   localhost:5000/vanilla-dev:latest   default             Running             Running 2 minutes ago                       
xjyvbavobvfq        vanilla_swarm_bidder.5   localhost:5000/vanilla-dev:latest   default             Running             Running 2 minutes ago                       
```

#### Checking service logs 
```bash
docker service logs vanilla_swarm_bidder --raw
docker service logs vanilla_swarm_loadbalancer
```
Start vanilla bidder + traefik proxy, use sudo on linux
```bash
docker stack deploy -c  swarm-persist-with-traefik.yaml  vanilla_swarm
```

Connect to traefik and make sure it shows backend and frontend mapped 

![traefik](https://github.com/venediktov/vanilla-rtb/wiki/images/SwarmTraefikDocker.png)
 

#### To compare performane of single docker image with swarm and with standalone stack running on physical host replace $(docker-machine ip) with localhost on Linux  
```bash
#running agains swarm/traefik utilizing sticky option backend processes use persistent connection 
ab -k -p ../examples/bidder/BID_REQUEST_BANNER.json -T application/json -n 10000 -c 10 http://$(docker-machine ip)/bid/123
#running against bidder in a single container backend process uses persistent connection
ab -k -p ../examples/bidder/BID_REQUEST_BANNER.json -T application/json -n 10000 -c 10 http://$(docker-machine ip):9081/bid/123
#running against loally built exe ( persistent connection)
ab -k -p ../examples/bidder/BID_REQUEST_BANNER.json -T application/json -n 10000 -c 10 http://localhost:9081/bid/123

```

#### To get HTTP keep-alive mode we have researched running docker swarm with HAProxy http-keep-alive option 

#### From common use case seen in many deployments

![haproxy-original](https://github.com/venediktov/vanilla-rtb/wiki/images/IngressDockerMeshRouting.png)

#### We have remodeled containers to utilize persistent connection based on http-keep-alive

![haproxy-vanilla-rtb](https://github.com/venediktov/vanilla-rtb/wiki/images/IngressDockerHAProxyRouting.png)
 
Start vanilla bidder + HAProxy proxy, use sudo on linux
```bash
docker stack deploy -c  swarm-persist-with-haproxy.yaml  vanilla_swarm

```

The performance is still an issue with any proxy we have tried, we'll add our own light weight proxy with zero memory allocation , zero copy just binding fron-end and back-end BSD sockets.
Another way to achieve best performance is to abondon TCP/IP for load balancing and utilize shared memory as explained [here](https://torusware.com/blog/2015/04/optimizing-communications-between-html/)

 
