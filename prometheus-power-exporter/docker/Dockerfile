FROM python:3.9-slim
WORKDIR /opt/prometheus-power-exporter

RUN useradd --no-log-init --create-home --shell /bin/bash power-exporter

COPY --chown=power-exporter:power-exporter exporter.py powermeasurement.py power_metrics.py requirements.txt .

RUN apt-get update && \
    # gcc is needed for pip package multidict
    apt-get install -y gcc && \
    pip install -r requirements.txt

USER power-exporter 

# Force line-buffered output in order to see docker logs immediately. 
# (Otherwise fully-buffered would be used by default as there is no tty)
CMD stdbuf -oL python exporter.py