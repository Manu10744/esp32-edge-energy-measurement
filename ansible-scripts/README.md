# Ansible Scripts for OpenFaaS Cluster Automation

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

#

### Ping all configured hosts
Checks availability.

```bash
ansible all -m ping
```

#

### Executing playbooks
```bash
# Without become
ansible-playbook <playbook_name>.yml

# With using become (this will ask for become user password)
ansible-playbook --ask-become-pass <playbook_name>.yml

# Execute playbook at a certain task (usually with the failed one from previous run)
ansible-playbook <playbook_name>.yaml --start-at-task="<task_name>"
```
