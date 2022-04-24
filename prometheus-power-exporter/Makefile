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