# Docker 
DOCKER_IMAGE_NAME = phyz1x/powermeasurement-udp-client
DOCKERFILE_DIR = ./docker

PROTOBUF_DIR = ../protobuffers

# Compilation
SRCS = udp_client.c $(PROTOBUF_DIR)/powermeasurements/powermeasurement.pb-c.c
LIBS = -lprotobuf-c -lpthread -lconfuse
COMPILED_FILENAME = udp_client

build:
	gcc -Wall -g -I/usr/include $(SRCS) -o $(COMPILED_FILENAME) -L/usr/lib $(LIBS)

dockerimage: 
	@echo "Creating Docker Image $(DOCKER_IMAGE_NAME) ..."
	@cp -R $(PROTOBUF_DIR) $(DOCKERFILE_DIR)
	docker buildx build --platform linux/arm64/v8,linux/amd64 --tag $(DOCKER_IMAGE_NAME):latest -f $(DOCKERFILE_DIR)/Dockerfile --push .
	@rm -rf $(DOCKERFILE_DIR)/protobuffers

clean:
	@if test -f "$(COMPILED_FILENAME)"; then\
		echo "Removing file $(COMPILED_FILENAME)";\
    	rm $(COMPILED_FILENAME);\
	else\
		echo "Cannot remove $(COMPILED_FILENAME): File doesn't exist.";\
	fi
