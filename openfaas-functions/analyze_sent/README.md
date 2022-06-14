# Build the function
First, the foundation is generated.
```bash
faas-cli build -f <path/to/fn.yml>
```

Then in the Dockerfile, some dependencies for NLTK have to be installed:
```Dockerfile
[...]
RUN pip install -r requirements.txt --target=/home/app/python && \
    python3 -m nltk.downloader punkt && \
    python3 -m nltk.downloader averaged_perceptron_tagger
[...]
```

<br>

# Invoke the function
`HTTP POST http://<OPENFAAS_GATEWAY>:<GATEWAY_PORT>/function/analyze-sentence`

for instance via `curl`:
```bash
curl -X POST http://127.0.0.1:31112/function/analyze-sentence -H 'Content-Type: application/json' -d '{"sentence": "Hello, this is just a test to see if it works!"}'
```