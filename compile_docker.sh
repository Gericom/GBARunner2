#!/bin/bash
docker -v
if [ $? -ne 0 ]; then
    echo "Docker not installed." >&2
    echo "Install Docker at https://www.docker.com/"
fi

#check for GBARunner2 image
docker image inspect gbarunner2 >/dev/null 2>&1 

if [ $? -ne 0 ]; then
    # build the image if it doesn't exist.
    docker build -t gbarunner2 --label gbarunner2 ./docker
fi

docker run --rm -t -i -v "$(pwd):/data" gbarunner2 make $@