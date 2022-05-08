#!/bin/bash

# This will remove the docker containers created by 'full_arch_setup.sh'.

echo "Removing the Docker Containers from multi-arch test setup ..."
echo
docker container stop mock-powermeasurement-server && docker container rm mock-powermeasurement-server

echo 

docker container stop armv7-client && docker container rm armv7-client
docker container stop armv7-exporter && docker container rm armv7-exporter

echo

docker container stop arm64v8-client && docker container rm arm64v8-client
docker container stop arm64v8-exporter && docker container rm arm64v8-exporter

echo

docker container stop amd64-client && docker container rm amd64-client
docker container stop amd64-exporter && docker container rm amd64-exporter
