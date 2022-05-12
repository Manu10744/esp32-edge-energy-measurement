# Build the function
```bash
faas-cli build -f <path/to/fn.yml>
```

<br>

# Invoke the function
`HTTP POST http://<OPENFAAS_GATEWAY>:<GATEWAY_PORT>/function/<FUNCTION_NAME>`