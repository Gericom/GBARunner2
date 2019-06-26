docker -v
if (!$?) {
    Write-Error "Docker not installed.";
    Write-Output "Install Docker at https://www.docker.com/";
}

#check for GBARunner2 image
docker image inspect gbarunner2 >$null 2>&1 

if (!$?) {
    # build the image if it doesn't exist.
    docker build -t gbarunner2 --label gbarunner2 ./docker
}

docker run --rm -t -i -v "$pwd\:/data" gbarunner2 make @args