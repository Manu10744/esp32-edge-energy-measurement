# Docker
DOCKER_IMAGE_NAME = phyz1x/prometheus-power-exporter
DOCKERFILE_DIR = ./docker

PROTOBUF_DIR = ../protobuffers

# Compilation
SRCS = exporter.c power_metrics.c $(PROTOBUF_DIR)/powermeasurements/powermeasurement.pb-c.c
LIBS = -lprom -lpromhttp -lmicrohttpd -lpthread -lprotobuf-c
COMPILED_FILENAME = exporter

build:
	gcc -Wall $(SRCS) -o $(COMPILED_FILENAME) $(LIBS)

docker: 
	cp $(COMPILED_FILENAME) $(DOCKERFILE_DIR)
	docker build -t $(DOCKER_IMAGE_NAME) $(DOCKERFILE_DIR)
	rm $(DOCKERFILE_DIR)/$(COMPILED_FILENAME)
	docker push $(DOCKER_IMAGE_NAME)