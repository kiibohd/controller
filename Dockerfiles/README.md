# Kiibohd Controller - Dockerfiles

Instead of installing all of the dependencies manually, it's recommended to use a Dockerfile to install all the necessary dependencies automatically.


## Setting up Docker

You'll need to setup Docker for your system.
This is usually pretty straight-forward if you haven't already installed it.


### Linux

Usually this just entails installing the docker package and starting the docker service.
You'll also want to enable non-root access to the docker daemon.

* [Arch Linux](https://wiki.archlinux.org/index.php/Docker)
* [Ubuntu 16.04](https://www.digitalocean.com/community/tutorials/how-to-install-and-use-docker-on-ubuntu-16-04)

After you're finished, make sure you're able to run `docker info` in your terminal window.


### macOS

* [Installation](https://docs.docker.com/docker-for-mac/install/)
* [Getting Started](https://docs.docker.com/docker-for-mac/)

After you're finished, make sure you're able to run `docker info` in your terminal window.


### Windows

* [Installation](https://docs.docker.com/docker-for-windows/install/)
* [Getting Started](https://docs.docker.com/docker-for-windows/)

After you're finished, make sure you're able to run `docker info` in your terminal window.


## Using a Dockerfile

Using a terminal where you can successfully call `docker info` run the following commands.
The Ubuntu Environment is the default supported environment.
However, most developers are using Arch Linux for their testing.


### [Ubuntu Environment](Dockerfile)

To generate the environment, only need to do this once, or whenever you update the git repo.

```bash
cd controller/Dockerfiles
docker build -t controller .
cd ..
```

To enter the build environment.
```bash
docker run -it --rm -v "$(pwd):/controller" controller
```

To exit the docker environment.
```bash
exit
```


### [Arch Linux Environment](Dockerfile.archlinux)

To generate the environment, only need to do this once, or whenever you update the git repo.

```bash
cd controller/Dockerfiles
docker build -f Dockerfile.archlinux -t controller.archlinux .
cd ..
```

To enter the build environment.
```bash
docker run -it --rm -v "$(pwd):/controller" controller.archlinux
```

To exit the docker environment.
```bash
exit
```


## Building Firmware

Once in the docker environment, you'll already be inside the [Keyboards](../Keyboards) folder.
So you can just build firmware for a keyboard.
```bash
./k-type.bash
```

The contents will be located in `linux-gnu.K-Type.gcc.ninja`.


## Files

* [all.bash](all.bash) - Test script used by Travis-CI to test all Dockerfiles.
* [archlinux.bash](archlinux.bash) - Test script to test ArchLinux Dockerfile.
* [common.bash](common.bash) - Library commands used by [all.bash](all.bash).
* [docker.bash](docker.bash) - Convenince library used by [all.bash](all.bash).
* [Dockerfile](Dockerfile) - Ubuntu Dockerfile.
* [Dockerfile.archlinux](Dockerfile.archlinux) - ArchLinux Dockerfile.
* [ubuntu.bash](ubuntu.bash) - Test script to test Ubuntu Dockerfile.

