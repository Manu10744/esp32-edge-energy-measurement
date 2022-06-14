# OpenFaas function analyze-sentence

### Build the function
Execute multiarch build in order to deploy to ARM devices
```bash
faas-cli publish -f analyze_sent.yml --platforms linux/amd64,linux/arm/v7,linux/arm64/v8 --build-arg ADDITIONAL_PACKAGE='gcc musl-dev'
```

<br>

### Invoke the function
The function expects an input of a `JSON` string containing the key `sentence` that holds the sentence to be analyzed.
<br>

The function can be invoked via HTTP:
`HTTP POST http://<OPENFAAS_GATEWAY>:<GATEWAY_PORT>/function/analyze-sentence`

for instance using `curl`:
```bash
curl -X POST http://127.0.0.1:31112/function/analyze-sentence -H 'Content-Type: application/json' -d '{"sentence": "Hello, this is just a test to see if it works!"}'
```