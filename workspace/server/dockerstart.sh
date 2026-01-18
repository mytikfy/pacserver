#!/bin/sh

BASEDIR=$(readlink -f $(dirname $0))

docker container rm pacserver > /dev/null 2>&1

docker run --name pacserver --net host \
		-v ./paccache:/var/cache/paccache \
		-v ./pacconfig:/etc/pacconfig \
		pacserver:latest \
		sh -c "bin/server --cache /var/cache/paccache --config /etc/pacconfig --port 8080"

