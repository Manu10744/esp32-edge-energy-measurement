# Ansible Scripts for OpenFaaS Cluster Automation
This directory contains Ansible scripts that can be used to automatically setup a FaaS Kubernetes cluster of edge devices with the help of k3s and OpenFaaS.<br>
A monitoring stack powered by Prometheus & Grafana is deployed as well in order to monitor the cluster and its nodes.

<br>

**Note**: It cannot guaranteed that these scripts finish successfully on every device because there is a huge range of different edge devices that all come with different operating systems and other specifications.
The scripts were tested in the following setup:

|  Edge Device  |  Role  |
| --- | --- |
| NVIDIA Jetson Nano Developer Kit    |  cluster-master   |
| Raspberry Pi Model B v1.2   |  cluster-worker   |
| PYNQ-Z1   |  cluster-worker   |
| Google Coral Dev Board    |  cluster-worker   |
| ODROID-XU4 (2x)   |  cluster-worker   |

<br>

## Before use

Before using the ansible playbooks, some initial steps have to be done first.

### Step 1: Connect edge devices to the network
The IPs should be fixed. Once each edge device was assigned an IP, the IPs can be configured per host in the ansible `hosts.yaml` file.


### Step 2: Create user that can be used for SSH connection
Ansible needs a user that it can use to connect as via SSH. The home directory should be created for the user so there is access to an `.ssh` folder.

```bash
# Create user
sudo useradd -m <username>
# Set password for user
passwd <username>
```

## Step 3: Add user to sudoers
Usually, many automation tasks done with Ansible require root privileges. Ansible can do that by switching to a privileged user after connection via SSH, which can be done by using the `become` directive.

```bash
sudo usermod -aG sudo <username>
```

#

### Mini-Cheatsheet
#### Executing playbooks
```bash
# Execute playbook
ansible-playbook <playbook_name>.yml

# Ask for become-password if running with become-mode is enabled
ansible-playbook --ask-become-pass <playbook_name>.yml

# Execute playbook at a certain task (usually with the failed one from previous run)
ansible-playbook <playbook_name>.yaml --start-at-task="<task_name>"

# Limit playbook to one specific host
ansible-playbook --limit=<host_name> <playbook_name>.yml
```

#### Ping all configured hosts

```bash
ansible all -m ping
```


#### Add user to group `docker` to avoid sudo all the time
```bash
# Create group
sudo groupadd docker

# Add current user
sudo usermod -aG docker $USER

# If permission denied still occurs when executing docker commands:
sudo chmod 666 /var/run/docker.sock
```

#

## Troubleshooting
- Raspberry Pi fails to join k3s cluster with error message `Failed to find memory cgroup`:<br>
Add the following to `/boot/cmdline.txt`:
```bash
cgroup_memory=1 cgroup_enable=memory
```

<br>

- Docker installation fails on Google Coral Dev Board:<br>
Instructions can be found in https://github.com/f0cal/google-coral/issues/32

<br>

- Can't install `docker` using the usual approach on PYNQ-Z1<br>
Instructions can be found in https://discuss.pynq.io/t/docker-xilinx-platforms-pynq/1962

- `dockerd` cannot start on ODROID-XU4, it fails with:
```bash
docker.service - Docker Application Container Engine
   Loaded: loaded (/lib/systemd/system/docker.service; enabled; vendor preset: enabled)
   Active: failed (Result: exit-code) since Fri 2017-07-14 18:23:13 IST; 2min 4s ago
     Docs: https://docs.docker.com
  Process: 6325 ExecStart=/usr/bin/dockerd -H fd:// (code=exited, status=1/FAILURE)
 Main PID: 6325 (code=exited, status=1/FAILURE)

Jul 14 18:23:12 iconlap02 systemd[1]: Starting Docker Application Container Engine...
Jul 14 18:23:12 iconlap02 dockerd[6325]: time="2017-07-14T18:23:12.415162784+05:30" level=info msg="libcontainerd: new containerd process, pid: 6333"
Jul 14 18:23:13 iconlap02 dockerd[6325]: Error starting daemon: error initializing graphdriver: /var/lib/docker contains several valid graphdrivers: aufs, overlay; Please cleanup or explicitly choose storage driver (-s <DRIVER>)
Jul 14 18:23:13 iconlap02 systemd[1]: docker.service: Main process exited, code=exited, status=1/FAILURE
Jul 14 18:23:13 iconlap02 systemd[1]: Failed to start Docker Application Container Engine.
Jul 14 18:23:13 iconlap02 systemd[1]: docker.service: Unit entered failed state.
Jul 14 18:23:13 iconlap02 systemd[1]: docker.service: Failed with result 'exit-code'.
```

Could not make it work with the official Ubuntu 18.04 image provided by `wiki.odroid.com`. Installing an `Armbian` Image for ODROID-XU4 (`Armbian_20.08.1_Odroidxu4_bionic_legacy_4.14.195`) solved it.

<br> 

- `sudo apt upgrade` fails on ODROID-XU4 after executing `sudo apt update` when running the Armbian image due to certificate verification errors
Can be fixed by executing ´sudo apt install ca-certificates` beforehand.