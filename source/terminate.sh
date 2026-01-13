#! /bin/bash

sudo make -C source clean-all

sudo systemctl stop instance-1-display.service
sudo systemctl stop instance-2-display.service

sudo systemctl stop seat-display-transient-instance-1.service
sudo systemctl stop seat-display-transient-instance-2.service