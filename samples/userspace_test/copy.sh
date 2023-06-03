#!/bin/bash

# shell folder
current_folder=$(cd "$(dirname "$0")" || exit;pwd)

# After power on, set /etc/ssh/sshd_config PermitRootLogin yes, then reboot
# sshpass -p root scp -r -P 3522 out root@localhost:/root
sshpass -p root scp -P 3522 ${current_folder}/out/signal root@localhost:/root
