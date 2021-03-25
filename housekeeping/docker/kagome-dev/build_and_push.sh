#!/bin/bash -ex

cd "$(dirname "$0")"

VERSION=1
TAG=harrm/kagome-dev:$VERSION

docker build -t ${TAG}-minideb -f minideb-full.Dockerfile .
docker push ${TAG}-minideb
