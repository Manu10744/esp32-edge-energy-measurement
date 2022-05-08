# Docker 
DOCKER_IMAGE_NAME = phyz1x/powermeasurement-udp-client
DOCKERFILE_DIR = ./docker
ARCHITECTURES = linux/arm/v7,linux/arm64/v8,linux/amd64

# Compilation
SRCS = udp_client.c powermeasurement.pb-c.c
LIBS = -lprotobuf-c -lpthread -lconfuse
COMPILED_FILENAME = udp_client

build:
	gcc -Wall -g -I/usr/include $(SRCS) -o $(COMPILED_FILENAME) -L/usr/lib $(LIBS)

dockerimage: 
	@echo "Creating Docker Image '$(DOCKER_IMAGE_NAME)' for the following architectures: $(ARCHITECTURES) ..."
	docker buildx build --platform $(ARCHITECTURES) --tag $(DOCKER_IMAGE_NAME):1.0.0 -f $(DOCKERFILE_DIR)/Dockerfile --push .

clean:
	@if test -f "$(COMPILED_FILENAME)"; then\
		echo "Removing file $(COMPILED_FILENAME)";\
    	rm $(COMPILED_FILENAME);\
	else\
		echo "Cannot remove $(COMPILED_FILENAME): File doesn't exist.";\
	fi
