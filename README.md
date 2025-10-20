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
> Copy the files that you have with e.g. `cp ../../../{*.json,*.pdf.txt} meta-tcat/recipes-core/base-files/files/`
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
