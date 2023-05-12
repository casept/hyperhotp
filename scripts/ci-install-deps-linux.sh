#!/usr/bin/env bash
set -eo pipefail

sudo apt update
sudo apt upgrade -y

sudo apt install -y build-essential cmake pkg-config libusb-1.0-0-dev libsdl2-dev
