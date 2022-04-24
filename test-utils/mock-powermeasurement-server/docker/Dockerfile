FROM ubuntu:20.04
WORKDIR /opt/mock-powermeasurement-server

RUN apt-get update && \
    apt-get install -y libprotobuf-c-dev  && \
    ldconfig

RUN useradd --no-log-init --create-home --shell /bin/bash mock-server
COPY --chown=mock-server:mock-server mock_server .

USER mock-server 

# Force line-buffered output in order to see docker logs immediately. 
# (Otherwise fully-buffered would be used by default as there is no tty)
CMD stdbuf -oL ./mock_server