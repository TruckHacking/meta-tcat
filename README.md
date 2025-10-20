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

The build requires files processed from the SAE standards J1939, J1587, and J1708. The files must be generated from the SAE PDFs as described in the `pretty-j1939` and `pretty_j1587` projects. You need to copy the files (`J1939db.json`, `J1587_201301.pdf.txt`,  `J1708_201609.pdf.txt`) under the `base-files/files` location so our recipe can pull them. Otherwise delete or modify the [base files recipe](./recipes-core/base-files/). After generating them, copy the files that you have e.g.
```shell
cp ../../../{*.json,*.pdf.txt} meta-tcat/recipes-core/base-files/files/
```

Then you can build the image:
```shell
bitbake core-image
```

> IMPORTANT: DO NOT MODIFY ANYTHING while the build is running. It will take a while to complete for the first time. Maybe go get a coffee or something.

> NOTE: The build takes a very long time so if you are on a remote machine use something that will prevent the build from crashing if you are disconnected. e.g.
```shell
tmux new-session -d -s core-image 'bitbake core-image'
```
After the image is complete, you can flash it to your device from 'deploy-ti/images/tcat/core-image-tcat.rootfs.wic.xz' with your favorite flashing tool (tested with balenaEtcher). See the User Guide in https://github.com/nmfta-repo/TCAT

Please submit any issues with the TCAT here: https://github.com/nmfta-repo/TCAT/issues
