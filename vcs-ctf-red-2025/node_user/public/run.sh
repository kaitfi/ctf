#!/bin/sh

docker build . -t node
docker run -it -p 1337:1337 node