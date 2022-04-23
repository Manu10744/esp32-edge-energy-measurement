# [Stage 1]: Compile program within Container
FROM ubuntu:20.04 AS builder
WORKDIR /builder

# Install dependencies needed for compiling
RUN apt-get update && \
    apt-get install -y make build-essential libprotobuf-c-dev && \
    ldconfig

# Copy program files and protobuffers
COPY * ./app/
COPY ./docker/protobuffers ./protobuffers

RUN cd ./app && make build


# [Stage 2]: Create Docker Image from compiled file in Stage 1
FROM ubuntu:20.04
WORKDIR /opt/powermeasurement-udp-client

RUN apt-get update && \
    apt-get install -y libprotobuf-c-dev  && \
    ldconfig

RUN useradd --no-log-init --create-home --shell /bin/bash udp-client
COPY --from=builder --chown=udp-client:udp-client /builder/app/udp_client .

USER udp-client 
CMD ./udp_client 