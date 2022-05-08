ESP32_SERVER_TARGET_DIR = ../../esp32-powermeter-udp-server/components/power_measurement/
POWERMEASUREMENT_CLIENT_TARGET_DIR = ../../powermeasurement-udp-client/
PROMETHEUS_EXPORTER_TARGET_DIR = ../../prometheus-power-exporter/

protobufs:
	@echo "Generating protobuf file(s) for the projects using C ..."
	protoc-c --c_out=$(ESP32_SERVER_TARGET_DIR) powermeasurement.proto
	protoc-c --c_out=$(POWERMEASUREMENT_CLIENT_TARGET_DIR) powermeasurement.proto

	@echo 
	@echo "Generating protobuf file(s) for the projects using Python ..."
	protoc -I . --python_betterproto_out=$(PROMETHEUS_EXPORTER_TARGET_DIR) powermeasurement.proto
