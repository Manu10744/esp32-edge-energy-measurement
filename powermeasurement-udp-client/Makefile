# Docker 
DOCKER_IMAGE_NAME = phyz1x/powermeasurement-udp-client
DOCKERFILE_DIR = ./docker

PROTOBUF_DIR = ../protobuffers

# Compilation
SRCS = udp_client.c $(PROTOBUF_DIR)/powermeasurements/powermeasurement.pb-c.c
LIBS = -lprotobuf-c -lpthread
COMPILED_FILENAME = udp_client

build:
	gcc -g -I/usr/include $(SRCS) -o $(COMPILED_FILENAME) -L/usr/lib $(LIBS)

docker: 
	cp $(COMPILED_FILENAME) $(DOCKERFILE_DIR)
	docker build -t $(DOCKER_IMAGE_NAME) $(DOCKERFILE_DIR)
	rm $(DOCKERFILE_DIR)/$(COMPILED_FILENAME)
	docker push $(DOCKER_IMAGE_NAME)