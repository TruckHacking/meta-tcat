# meta-tcat layer

This README file contains information on the contents of the meta-tcat layer, corresponding to the [NMFTA Truck Cybersecurity Assessment Tool (TCAT)](https://github.com/nmfta-repo/TCAT).

## Build From Scratch:

```shell
mkdir tcat-dev
cd tcat-dev
```
Download the tcat-setup-dev-env.sh script:
```shell
wget https://github.com/TruckHacking/meta-tcat/raw/scarthgap/tcat-setup-dev-env.sh
```
Run it:
> Note: Performed as user with passwordless sudo within Linux env.
```shell
chmod +x tcat-setup-dev-env.sh
./tcat-setup-dev-env.sh
```
After it has run, you'll need to source oe-init-build-env EVERY TIME YOU OPEN A NEW TERMINAL:
```shell
cd Yocto
source oe-init-build-env
```

Then you can build the image:
> IMPORTANT: you need to have all the standards (e.g., J1939db.json) under the same location so our recipe can pull them. Otherwise delete or modify the [base files recipe](./recipes-core/base-files/)
> Copy the files that you have with e.g. `cp ../../../{*.json, *.pdf.txt} meta-tcat/recipes-core/base-files/files/`
> The files must be generated from the SAE PDFs as described in the `pretty-j1939` and `pretty_j1587` projects.


```shell
bitbake core-image
```
> DO NOT MODIFY ANYTHING while the build is running. It will take a while to complete for the first time. Maybe go get a coffee or something:
```shell
tmux new-session -d -s core-image 'bitbake core-image'
```
After the image is complete, you can flash it to your device from 'deploy-ti/images/tcat/core-image-tcat.rootfs.wic.xz' with your favorite flashing tool (tested with balenaEtcher). See the User Guide in https://github.com/nmfta-repo/TCAT

Please submit any issues with the TCAT here: https://github.com/nmfta-repo/TCAT/issues

## PRU Configuration and Dynamic Mode Switching

The TCAT platform uses the AM335x PRU (Programmable Real-Time Unit) to interface with the Intellon SSC P485 modem for J2497 and J1708 communication. Because different transceiver setups require different ultra-precise nanosecond-level timing constraints, we provide multiple PRU firmware variants. 

You can dynamically switch the PRU0 firmware and the host daemon routing using the `set-tcat-pru` command.

**Usage:**
`set-tcat-pru [MODE] [--persistent] | --status`

**Available Modes:**
*   `plc` or `plc-default`: Loads the standard J2497 firmware. This uses the hardware UART TX FIFO to pace transmissions to the Intellon modem, and routes the host daemon to listen on UDP ports 6971/6972.
*   `plc-bitbangtx`: Loads an alternate PLC firmware. Reception occurs via the UART, but transmission bypasses the UART FIFO and bitbangs the J2497 symbols directly out of GPIO 51 using legacy timing loops.
*   `j1708`: Loads J1708-specific timing firmware. The script also automatically creates a `systemd` override to force the `plc4trucksduck_host` daemon to route traffic over the legacy J1708 UDP ports (6969/6970).

**Persistence:**
By default, the script modifies the active PRU firmware using sysfs (`/sys/class/remoteproc`) and places temporary overrides in `/run/systemd/`. These changes will be lost on reboot. 
If you pass the `--persistent` flag, the script will overwrite the default `/lib/firmware/am335x-pru0-fw` binary and place the systemd override in `/etc/systemd/system/`. **Warning:** These changes survive reboots and permanently alter the default state of the board.
