# k8s Setup for Tests

Deployment files are based on k8s provided by Docker Desktop, v1.22.5<br>
(Can be used with some slight modifications on `minikube` as well.)

## Deployment Steps

- Deploy NGINX Ingress Controller:
```bash
cd k8s-dashboard
kubectl apply -f ingress-nginx.yaml
```

- Deploy Kubernetes Dashboard
```bash
cd k8s-dashboard
kubectl apply -f dashboard-deployment.yaml
kubectl apply -f dashboard-ingress.yaml
```
Dashboard Authentication is disabled, dashboard-default `ServiceAccount` has role `cluster-admin`

- Deploy Grafana
```bash
cd grafana
kubectl apply -f grafana-datasource.yaml
kubectl apply -f grafana-deployment.yaml
kubectl apply -f grafana-service.yaml
kubectl apply -f grafana-ingress.yaml
```
Default credentials: `username`: admin | `password`: admin

- Deploy Prometheus
```bash
cd prometheus
kubectl apply -f prometheus-deployment.yaml
kubectl apply -f prometheus-service.yaml
kubectl apply -f prometheus-ingress.yaml
```

- Deploy OpenFaaS
<br>
(=> If not done already, install `faas-cli`)<br>

Deployment is done with `helm`. More info: https://github.com/openfaas/faas-netes/blob/master/chart/openfaas/README.md

```bash
# Creates the OpenFaaS namespaces
kubectl apply -f https://raw.githubusercontent.com/openfaas/faas-netes/master/namespaces.yml

# Add OpenFaaS helm repo
helm repo add openfaas https://openfaas.github.io/faas-netes/

helm repo update \
 && helm upgrade openfaas --install openfaas/openfaas \
    --namespace openfaas  \
    --set functionNamespace=openfaas-fn \
    --set generateBasicAuth=true

PASSWORD=$(kubectl -n openfaas get secret basic-auth -o jsonpath="{.data.basic-auth-password}" | base64 --decode) && \
echo "OpenFaaS admin password: $PASSWORD"
```

OpenFaas Gateway can be reached in the browser (under `127.0.0.1`) via the mapped port of the `gateway-external` service:<br>
`kubectl get svc -n openfaas gateway-external -o wide`

Login with 'admin' as `user` and $PASSWORD as `password`.<br>
Login via `faas-cli`: `echo -n $PASSWORD | faas-cli login -g $OPENFAAS_URL -u admin --password-stdin`

<br>

### Power Measurement / Monitoring Stack
- Deploy the mocked power measurement server

```bash
kubectl apply -f $MOCK_SERVER_DIR/deploy
```

- Deploy the power monitoring config for the cluster

Set the IP of the mocked measurement server inside the config first.<br>
The config file will be mounted as a volume to the `powermeasurement-udp-client` that resides in the same pod as the power-exporter.<br>
(This file needs modifications if `minikube` is used)

```bash
kubectl apply -f test-powermonitoring-configmap.yaml
```

- Deploy the Prometheus Power Exporter
```bash
kubectl apply -f $EXPORTER_DIR/deploy
```

