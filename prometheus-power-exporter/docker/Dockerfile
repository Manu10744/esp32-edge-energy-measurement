FROM ubuntu:20.04

WORKDIR prom-exporter

# Copy shared libs
COPY libprom-dev-0.1.3-Linux.deb .
COPY libpromhttp-dev-0.1.3-Linux.deb .
COPY libprotobuf-c1_1.2.1-2_amd64.deb .

# Copy exporter executable
COPY exporter .

# Install shared libs
RUN apt update && apt-get -y install libmicrohttpd-dev && \
    dpkg -i libprom-dev-0.1.3-Linux.deb && \
    dpkg -i libpromhttp-dev-0.1.3-Linux.deb && \
    dpkg -i libprotobuf-c1_1.2.1-2_amd64.deb

RUN useradd --no-log-init --create-home --shell /bin/bash exporter
USER exporter 

CMD ./exporter