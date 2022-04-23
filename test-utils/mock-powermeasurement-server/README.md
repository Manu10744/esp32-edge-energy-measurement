# Mocked Power Measurement Server

A mocked power measurement UDP server that can be used for tests.<br>
The server is listening for requests on Port **10000** and will ignore everything that was sent by the client. Once the server received a message
it will begin to continuously send mocked power measurements.

### Build
The build requires the following dependencies to be installed:
- `protobuf-c` (requires `protobuf`)

```bash
make build
```

### Usage
The required arguments are IP and port of the target UDP server as well as the channel (e.g. `2`) to fetch power measurements from.
```bash
./mock_server
```
