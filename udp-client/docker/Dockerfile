FROM ubuntu:20.04

WORKDIR /opt/powermeasurement-udp-client

RUN apt-get update && \
    apt-get install -y libprotobuf-c-dev  && \
    ldconfig

RUN useradd --no-log-init --create-home --shell /bin/bash udp-client
USER udp-client 

COPY udp_client .
CMD ./udp_client