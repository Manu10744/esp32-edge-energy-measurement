# Docker
DOCKER_IMAGE_NAME = phyz1x/mock-powermeasurement-server
DOCKERFILE_DIR = ./docker

PROTOBUF_DIR = ../../protobuffers

# Compilation
SRCS = mock-server.c $(PROTOBUF_DIR)/powermeasurements/powermeasurement.pb-c.c
LIBS = -lprotobuf-c -lpthread
COMPILED_FILENAME = mock_server

build:
	gcc -Wall -g -I/usr/include $(SRCS) -o $(COMPILED_FILENAME) -L/usr/lib $(LIBS)

clean:
	@if test -f "$(COMPILED_FILENAME)"; then\
		echo "Removing file $(COMPILED_FILENAME)";\
    	rm $(COMPILED_FILENAME);\
	else\
		echo "Cannot remove $(COMPILED_FILENAME): File doesn't exist.";\
	fi

dockerimage:
	make build
	cp mock_server ./docker
	docker build -t $(DOCKER_IMAGE_NAME) -f $(DOCKERFILE_DIR)/Dockerfile .
	rm ./docker/mock_server