# TruckHackingOS Agent Validation Plan

This document provides a systematic validation plan for an AI agent to verify the integrity, contents, and runtime capabilities of a built **TruckHackingOS** image. It relies on the Yocto build system and QEMU user-mode emulation to perform headless testing.

## 1. Context & Prerequisites

### 1.1 Working Directory
**All commands in this plan MUST be executed from the root of the Yocto workspace.** 
This is the directory containing `oe-init-build-env`, the `meta-*/` layers, and the `build/` directory. Do not run these commands from within `meta-tcat` or `build` unless explicitly instructed.

Expected Directory Structure:
```text
Yocto/                 <-- WORKING DIRECTORY (Run commands here)
├── oe-init-build-env
├── build/
├── meta-tcat/
├── .gemini/
│   └── skills/        <-- Agent skills repository
└── ...
```

### 1.2 Obtaining the Required Skills
This validation plan relies on specific agent skills (like `qemu-user-mode`). If the `.gemini/skills` directory is missing or incomplete, obtain it from the official repository.

**Action:** Clone skills repository if not present
```bash
if [ ! -d ".gemini/skills" ]; then
    git clone https://github.com/BenGardiner/bitbake-yocto-agent-skills .gemini/skills
else
    echo "Skills directory already exists."
fi
```

## 2. Phase 1: Build System Validation

Before interacting with the root filesystem, verify that the bitbake environment is correctly configured. 
*Note: You must source the build environment for these commands to work.*

**Action:** Verify the Distro Name
```bash
source oe-init-build-env build && bitbake-getvar -q --value DISTRO
# Expected Output: TruckHackingOS
```

**Action:** Verify the requested Python version
```bash
source oe-init-build-env build && bitbake-getvar -q --value PREFERRED_VERSION_python3
# Expected Output: 3.11%
```

## 3. Phase 2: Rootfs Preparation & Static Validation

To perform runtime and package checks, the latest built image must be extracted to the host. The image (`core-image`) must have already been successfully built.

### 3.1 Extract the Rootfs
Use the `qemu-user-mode` skill to extract the latest `core-image-tcat.rootfs.tar.xz` into a local directory.
```bash
./.gemini/skills/qemu-user-mode/scripts/extract_rootfs.sh
```
*Expected Result:* A `rootfs-extract/` directory is created in the workspace root.

### 3.2 Verify Critical Packages (Static Check)
Verify that specific sub-packages and tools are present in the extracted filesystem without running them.

**Action:** Check for `can-utils-isotp` tools
```bash
ls -l rootfs-extract/usr/bin/isotpsend rootfs-extract/usr/bin/isotprecv
# Expected Output: Both binaries should exist and be executable (-rwxr-xr-x).
```

**Action:** Check for `pretty-j1939`
```bash
ls -l rootfs-extract/usr/bin/pretty_j1939
# Expected Output: The python wrapper script should exist.
```

**Action:** Check for `truckdevil`
```bash
ls -l rootfs-extract/opt/tcat/programs/truckdevil/truckdevil.py
# Expected Output: The main python script should exist.
```

**Action:** Check for `automotive-scapy-playground`
```bash
ls -l rootfs-extract/usr/share/automotive_scapy_playground/easy_log.ipynb
# Expected Output: The playground notebooks should exist.
```

## 4. Phase 3: Runtime Validation (QEMU User-Mode)

Use the `run_in_qemu_user.sh` script to execute target ARM binaries natively on the host. 
**Note:** Yocto Python scripts often have shebangs (`#!/usr/bin/env python3`) that QEMU user-mode struggles to resolve automatically. Always invoke the target Python interpreter explicitly and pass the script as an argument.

### 4.1 Python and Jupyter Validation

**Action:** Verify Python 3.11 Runtime
```bash
./.gemini/skills/qemu-user-mode/scripts/run_in_qemu_user.sh /usr/bin/python3 --version
# Expected Output: Python 3.11.5
```

**Action:** Verify Jupyter 7 Core Packages
```bash
./.gemini/skills/qemu-user-mode/scripts/run_in_qemu_user.sh /usr/bin/python3 -m jupyter --version
# Expected Output: A list of Jupyter packages, with `notebook` showing version 7.x (e.g., 7.1.3).
```

### 4.2 Tool Validation

**Action:** Test `pretty-j1939`
```bash
./.gemini/skills/qemu-user-mode/scripts/run_in_qemu_user.sh /usr/bin/python3 /usr/bin/pretty_j1939 --help
# Expected Output: The help menu for pretty_j1939 should print successfully.
```

### 4.3 Truckdevil Validation

To validate that `truckdevil` can successfully resolve its complex dependencies (including `readline`, the JSON resource database, and `python-can` handlers) and launch its modules without crashing, we test the core modules headlessly using the `-c` argument and pipe `quit` into the interactive prompts to gracefully terminate them.

**Action:** Test `ecu_discovery` Module
```bash
echo "quit" | ./.gemini/skills/qemu-user-mode/scripts/run_in_qemu_user.sh /usr/bin/python3 /usr/bin/truckdevil -c "add_device virtual can0 500000; run_module ecu_discovery"
# Expected Output: Successful load of the "ECU Discovery tool" prompt followed by "Exiting TruckDevil" (Exit code 1 is expected due to sys.exit on quit).
```

**Action:** Test `send_messages` Module
```bash
echo "quit" | ./.gemini/skills/qemu-user-mode/scripts/run_in_qemu_user.sh /usr/bin/python3 /usr/bin/truckdevil -c "add_device virtual can0 500000; run_module send_messages"
# Expected Output: Successful load of the "Send Messages tool" prompt followed by "Exiting TruckDevil".
```

**Action:** Test `read_messages` Module (for print_messages capabilities)
```bash
echo "quit" | ./.gemini/skills/qemu-user-mode/scripts/run_in_qemu_user.sh /usr/bin/python3 /usr/bin/truckdevil -c "add_device virtual can0 500000; run_module read_messages"
# Expected Output: Successful load of the "Read Messages tool" prompt followed by "Exiting TruckDevil".
```

## 5. Phase 4: Validate nmfta Account & Capabilities (Post-Flash Provisioning)

*Note: Because the `nmfta` user and `tcat-ops` group are dynamically provisioned during the eMMC flashing sequence (via `emmc-flasher`), their presence cannot be fully evaluated simply by querying `/etc/passwd` on the raw SD card rootfs. Instead, validate the configuration artifacts and the provisioning logic embedded in the image.*

**Action:** Verify `nmfta` Home Directory and Seeded Configurations
```bash
ls -a rootfs-extract/home/nmfta/
ls -l rootfs-extract/home/nmfta/.jupyter/jupyter_notebook_config.py
# Expected Output: The home directory exists and contains seeded configuration files like .bashrc, .bash_profile, .nanorc, and .jupyter configurations.
```

**Action:** Verify `emmc-flasher` Provisioning Logic
```bash
grep "useradd.*nmfta" rootfs-extract/usr/bin/emmc-flasher
grep "usermod.*nmfta" rootfs-extract/usr/bin/emmc-flasher
# Expected Output: `useradd -m -d /home/nmfta -s /bin/bash nmfta` and `usermod -aG sudo,tcat-ops nmfta` proving the user is created and assigned to the correct capability groups during install.
```

**Action:** Verify `tcat-ops` Sudo Capabilities
```bash
cat rootfs-extract/etc/sudoers.d/tcat-ops
# Expected Output: A sudoers drop-in granting the `%tcat-ops` group NOPASSWD access to networking commands (ip, ifconfig), reboot/shutdown, and systemd-networkd restarts.
```

## 6. Conclusion

If all checks pass, the TruckHackingOS image is confirmed to contain the correct Python 3.11 environment, the updated J1939/ISOTP tooling, the Jupyter 7 data-science stack, the interactive Truckdevil framework, and the `nmfta` account capabilities correctly provisioned for post-install.
