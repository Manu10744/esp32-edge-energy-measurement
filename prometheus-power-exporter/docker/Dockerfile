# [Stage 1]: Compile program within Container
FROM ubuntu:20.04 AS builder
WORKDIR /builder

# Install dependencies needed for compiling
RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get -y install unzip cmake build-essential libmicrohttpd-dev libprotobuf-c-dev && \
    ldconfig

COPY ./docker/prometheus-client-c-0.1.3.zip .

## Build and install shared libraries
RUN unzip prometheus-client-c-0.1.3.zip && \
    cd prometheus-client-c-0.1.3 && \
    ./auto build && \
    ./auto package
RUN dpkg -i ./prometheus-client-c-0.1.3/prom/libprom-dev-0.1.3-Linux.deb && \
    dpkg -i ./prometheus-client-c-0.1.3/promhttp/libpromhttp-dev-0.1.3-Linux.deb

# Copy program files and protobuffers
COPY * ./app/
COPY ./docker/protobuffers ./protobuffers

RUN cd ./app && make build


# [Stage 2]: Create Docker Image from compiled file in Stage 1
FROM ubuntu:20.04
WORKDIR /opt/prometheus-power-exporter

# Copy shared libs
COPY --from=builder /builder/prometheus-client-c-0.1.3/prom/libprom-dev-0.1.3-Linux.deb .
COPY --from=builder /builder/prometheus-client-c-0.1.3/promhttp/libpromhttp-dev-0.1.3-Linux.deb .

# Install shared libs from Stage 1
RUN apt-get update && \
    apt-get -y install libmicrohttpd-dev libprotobuf-c-dev && \
    dpkg -i libprom-dev-0.1.3-Linux.deb && \
    dpkg -i libpromhttp-dev-0.1.3-Linux.deb && \
    ldconfig

RUN useradd --no-log-init --create-home --shell /bin/bash power-exporter
COPY --from=builder --chown=power-exporter:power-exporter /builder/app/exporter .

USER power-exporter 

# Force line-buffered output in order to see docker logs immediately. 
# (Otherwise fully-buffered would be used by default as there is no tty)
CMD stdbuf -oL ./exporter