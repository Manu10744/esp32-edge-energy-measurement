#!/usr/bin/env bash
source ~/.bashrc
ansible-playbook master.yml -vvvv
ansible-playbook minion.yml -vvvv
ansible-playbook master-after-join.yml -vvvv
#ansible-playbook nodes-reset-notebook.yml -vvvv
#ansible-playbook master-install-kube-notebook.yml

#ansible-playbook master-create-notebook.yml -vvvv
#ansible-playbook master-join-notebook.yml -vvvv