FROM ubuntu:20.04

WORKDIR prom-exporter

# Copy shared libs
COPY libprom-dev-0.1.3-Linux.deb .
COPY libpromhttp-dev-0.1.3-Linux.deb .

# Copy exporter executable
COPY example .

# Install shared libs
RUN apt update && apt-get -y install libmicrohttpd-dev && \
    dpkg -i libprom-dev-0.1.3-Linux.deb && \
    dpkg -i libpromhttp-dev-0.1.3-Linux.deb

CMD ./example