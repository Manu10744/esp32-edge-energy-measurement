PROTOBUF_DIR = ../../protobuffers

# Compilation
SRCS = mock-server.c $(PROTOBUF_DIR)/powermeasurements/powermeasurement.pb-c.c
LIBS = -lprotobuf-c -lpthread
COMPILED_FILENAME = mock_server

build:
	gcc -Wall -g -I/usr/include $(SRCS) -o $(COMPILED_FILENAME) -L/usr/lib $(LIBS)

clean:
	@if test -f "$(COMPILED_FILENAME)"; then\
		@echo "Removing file $(COMPILED_FILENAME)";\
    	rm $(COMPILED_FILENAME);\
	else\
		@echo "File $(COMPILED_FILENAME) cannot be removed, as it does not exist, yet.";\
	fi