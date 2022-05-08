# Protocol Buffers
This directory contains the protocol buffers that are used in the individual project components (`powermeasurement-udp-client`, `prometheus-power-exporter`, etc.) for the communication between them over the network.

Used Protobuffer implementations:
- For `C`: [protobuf-c](https://github.com/protobuf-c/protobuf-c)
    - Used by `esp32-powermeter-udp-server` and `powermeasurement-udp-client`
- For `Python`: [betterproto](https://github.com/danielgtaylor/python-betterproto)
    - Used by `prometheus-power-exporter`


<br>

#### Generate protobuf files in the target directories
This will generate the power measurement protobuffer files inside the specific target directories of `powermeasurement-udp-client`, `prometheus-power-exporter` etc.
```bash
make protobufs
```

<br>

### Troubleshooting
- `error while loading shared libraries: libprotobuf.so.1: cannot open shared object file: No such file or directory`:
<br>

Install libprotobuf-c
```bash
apt-get install -y libprotobuf-c-dev
```