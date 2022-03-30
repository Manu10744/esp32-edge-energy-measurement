# k8s setup for tests

k8s provided by Docker Desktop, v1.22.5

## Structure
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

- Deploy OpenWhisk

Done with `helm`. More info: https://github.com/apache/openwhisk-deploy-kube
```bash
cd openwhisk
helm repo add openwhisk https://openwhisk.apache.org/charts
helm repo update
helm install owdev openwhisk/openwhisk -n openwhisk --create-namespace -f mycluster.yaml
```
