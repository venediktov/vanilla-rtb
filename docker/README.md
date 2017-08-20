# VanillaRTB Docker Images

## Structure
VanillaRTB Docker images have two layers (optionally three) of hierarchy:

* #0.0 VanillaRTB Base - VanillaRTB unified runtime environment
* #1.1 VanillaRTB Dev (extends #0.0) - VanillaRTB development environment with preinstalled G++, CMake, Boost (selected libraries)
* #1.2 VanillaRTB Bld (extends #0.0) - VanillaRTB package builder image (!TODO!)
* #1.3 VanillaRTB Prod (extends #0.0) - VanillaRTB pre-built ready to run package
* #2.x (extends #1.3) - Specialized container preconfigured to run particular VanillaRTB component or subsystem

## Creating and Updating 
### ( instructions for vanilla-rtb stack contributors only !!!! )

Base package:

```bash
$ vanilla_env_ver=x.y.z
$ cd vanilla-rtb/docker
$ docker build --tag vanillartb/vanilla-base:${vanilla_env_ver} --file vanilla-base.Dockerfile ${PWD}
$ docker login
$ docker push vanillartb/vanilla-base:${vanilla_env_ver}
```

Development environment package : built automatically on hub.docker.com everytime we tag our git push 

```bash
$ vanilla_env_ver=x.y.z
$ cd vanilla-rtb/docker
$ docker build --tag vanillartb/vanilla-dev:${vanilla_env_ver} --file vanilla-dev.Dockerfile ${PWD}
$ docker tag vanillartb/vanilla-dev:${vanilla_env_ver} vanillartb/vanilla-dev:latest
$
$ docker login
$ docker push vanillartb/vanilla-dev:${vanilla_env_ver} vanillartb/vanilla-dev:latest
```

Production package :

```bash
$ vanilla_env_ver=a.b.c
$ vanilla_ver=x.y.z
$ cd vanilla-rtb/docker
$
$ docker container run -a STDOUT --name vanilla-build-box vanillartb/vanilla-dev:${vanilla_env_ver}
$ docker cp build-box:/root/pkg/vanilla-rtb/snapshot ./vanilla
$
$ docker build --tag vanillartb/vanilla-prod:${vanilla_ver} --file vanilla-prod.Dockerfile ${PWD}
$ /bin/rm -rf ./vanilla
$ docker tag vanillartb/vanilla-prod:${vanilla_ver} vanillartb/vanilla-prod:latest
$
$ docker login
$ docker push vanillartb/vanilla-prod:latest
```

# Running 

```bash
$ docker login
$ docker run --net=host -it --name vanilla-devbox vanillartb/vanilla-dev:latest
$ docker run --net=host -it --name vanilla-prodbox vanillartb/vanilla-prod:latest
```

#### You should get following container shell prompt when running our docker dev and prod containers 

```bash
root@default:~/pkg/vanilla-rtb/snapshot/bin#
```

#### vanilla-devbox has following directory structure besides OS and other dependencies installed 

```
/root/build - directory used to build vanilla-rtb binaries ( staging directory )
/root/pkg - directory where cmake had installed binaries from staging directory 
/root/code - directory holding source fetched from vanilla-rtb  github repository used in the build
```

# Running in prod with Redis
```bash
$ docker run --net=host --name vanilla-redis -d redis
$ docker run --net=host -it --name vanilla-prodbox vanillartb/vanilla-prod:latest
```

# Docker maintanance commands 
```bash
## see images installed 
sudo docker image ls
## remove stored image by id
sudo docker image rm vanillartb/vanilla-prod
sudo docker image rm redis
## remove container to restart with different params
sudo docker rm vanilla-prodbox
## stop running container 
sudo docker stop vanilla-prodbox
## start known container 
sudo docker start vanilla-prodbox
## attach shell to running container
sudo docker attach vanilla-prodbox
```
# Docker performance issues 
running docker with manual port forwarding command --publish e.g -p8080:9081  has 20-30% performace degradation due 
to this option utilizing  bridge driver, unless it has to be mapped using -p8080:9081 **avoid for performance issues**! 
The most performant network solution is **--net=host** ( exposes all container ports to host ) however **vanilla-rtb**  ports in **examples/config.cfg** file may not be open on the cloud/host server therefore you have to think of shipping config with docker container per installation or starting stack with custom command line arguments ( --bidder.port=8080  ) or starting docker with 
shared host/container file system ( please refer to docker volume -v flag )

```
$ sudo docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
db0004be8eff        bridge              bridge              local
42cee521962a        host                host                local
23f65108302b        none                null                local
```
#### Disclaimer : on linux most likely all of above docker commads need to preceed with sudo 

## References
[Debian Releases](https://www.debian.org/releases/)<br>
[Debian Docker Images](https://store.docker.com/images/debian/)<br>
[Dockerfile Reference](https://docs.docker.com/engine/reference/builder/)
