# Docker
DOCKER_IMAGE_NAME = phyz1x/prometheus-power-exporter
DOCKERFILE_DIR = ./docker
ARCHITECTURES = linux/arm/v7,linux/arm64/v8,linux/amd64

dockerimage: 
	@echo "Creating Docker Image '$(DOCKER_IMAGE_NAME)' for the following architectures: $(ARCHITECTURES)..."
	docker buildx build --platform $(ARCHITECTURES) --tag $(DOCKER_IMAGE_NAME):1.0.0 -f $(DOCKERFILE_DIR)/Dockerfile --push .