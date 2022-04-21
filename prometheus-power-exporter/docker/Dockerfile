FROM ubuntu:20.04

WORKDIR /opt/prometheus-power-exporter

# Copy shared libs
COPY libprom-dev-0.1.3-Linux.deb .
COPY libpromhttp-dev-0.1.3-Linux.deb .

# Install shared libs
RUN apt update && \
    apt-get -y install libmicrohttpd-dev libprotobuf-c-dev && \
    dpkg -i libprom-dev-0.1.3-Linux.deb && \
    dpkg -i libpromhttp-dev-0.1.3-Linux.deb && \
    ldconfig

RUN useradd --no-log-init --create-home --shell /bin/bash power-exporter
COPY --chown=power-exporter:power-exporter exporter .

USER power-exporter 
CMD ./exporter