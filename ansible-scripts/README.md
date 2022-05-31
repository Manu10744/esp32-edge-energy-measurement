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

### Step 3: Add user to sudoers
Usually, many automation tasks done with Ansible require root privileges. Ansible can do that by switching to a privileged user after connection via SSH, which can be done by using the `become` directive.

```bash
sudo usermod -aG sudo <username>
```

<br>

## Usage
1. Execute the playbook `master.yml` which sets up docker and k3s on the master node(s). At the end of the process, there will be a generated `join-command` in the folder `from_remote` that can be used on other nodes to join the cluster.
2. Execute the playbook `minion.yml` to setup docker and k3s agents on the configured worker nodes. At the end of the process, each node will join the cluster by using the generated `join-command` in Step 1.
3. Execute the playbook `master-after-join.yml` in order to deploy OpenFaaS and the monitoring stack to the cluster.


#

### Mini-Cheatsheet
#### Executing playbooks
```bash
# Execute playbook
ansible-playbook <playbook_name>.yml

# Ask for become-password used for the tasks running in become-mode
ansible-playbook --ask-become-pass <playbook_name>.yml

# Execute playbook at a certain task (usually with the failed one from previous run)
ansible-playbook <playbook_name>.yaml --start-at-task="<task_name>"

# Limit playbook to one specific host
ansible-playbook --limit=<host_name> <playbook_name>.yml

# Use another hosts file
ansible-playbook -i <alternative_hosts_file>.yml <playbook_name>.yml
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

<br>

## Troubleshooting
- Raspberry Pi fails to join k3s cluster with error message `Failed to find memory cgroup`:<br>
Add the following to `/boot/cmdline.txt`:
```bash
cgroup_memory=1 cgroup_enable=memory
```

<br>

- Docker installation (`docker-ce docker-ce-cli containerd.io`) fails on Google Coral Dev Board:<br>
Could make docker run using the following code (found in https://github.com/f0cal/google-coral/issues/32)

```bash
sudo apt-get install apt-transport-https ca-certificates curl gnupg2 software-properties-common
curl -fsSL https://download.docker.com/linux/debian/gpg | sudo apt-key add -
echo "deb [arch=arm64] https://download.docker.com/linux/debian buster stable" | sudo tee /etc/apt/sources.list.d/docker.list
sudo apt update
sudo apt install docker-ce docker-ce-cli containerd.io
sudo groupadd docker
sudo usermod -aG docker $USER
sudo reboot now
```

<br>

- Can't install `docker` using the usual approach on PYNQ-Z1<br>
Either use `bionic` for the docker apt repository or install docker as described in https://discuss.pynq.io/t/docker-xilinx-platforms-pynq/1962

<br>

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

Could not make docker run with the official Ubuntu 18.04 image provided by `wiki.odroid.com`. Installing an `Armbian` Image for ODROID-XU4 (e.g. `Armbian_20.08.1_Odroidxu4_bionic_legacy_4.14.195`) solved it.

<br> 

- `sudo apt upgrade` fails on ODROID-XU4 after executing `sudo apt update` due to certificate verification errors.<br>
Can be fixed by executing ´sudo apt install ca-certificates` beforehand.

<br>

- ODROID-XU4 cannot join the k3s cluster because kernel option `CGROUP_PIDS` is not enabled and k3s depends on it (See also https://forum.odroid.com/viewtopic.php?t=37533).<br>
Fixed by upgrading the `Armbian` image to `Armbian_21.08.3_Odroidxu4_bullseye_current_5.4.151`.

<br>

- k3s agent on ODROID-XU4 fails with error message `Kernel-module mismatch- iptables/1.8.2 Failed to initialize nft: Protocol not supported`.<br>
Occured after updating the `Armbian` image. This problem seems to be caused due to kernel update, can be fixed by rebooting: `sudo reboot`.

<br>

- Task `Add repository for docker` in ansible-role `docker` fails with error message `Failed to update apt cache: E:The repository 'https://download.docker.com/linux/ubuntu <...> Release' does not have a Release file.`<br>
Observed on:
    - Raspberry Pi
    - Coral Dev Board
    - PYNQ-Z1
    - ODROID-XU4
    <br>
    <br>
Solution: Use bionic instead of `{{ ansible_lsb.codename }}` in the command.
Solves the problem for: PYNQ-Z1, Raspberry Pi, ODROID-XU4. For a solution for the Coral Dev Board see above.
