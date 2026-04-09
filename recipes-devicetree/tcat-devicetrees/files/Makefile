# Compiler for device trees
DTC = dtc

# dt-bindings directory
DT_BINDINGS_DIR = $(shell ls -d /usr/lib/modules/*/build/include/dt-bindings | head -n 1)
SRC_DIR = .
OUT_DIR = /boot/dtb/tcat

# Find all .dts files in the current directory
DTS_FILES := $(wildcard $(SRC_DIR)/*.dts)
DTBO_FILES := $(patsubst $(SRC_DIR)/%.dts, $(OUT_DIR)/%.dtbo, $(DTS_FILES))

# Flags for dtc (Device Tree Compiler)
DTC_FLAGS = -i $(DT_BINDINGS_DIR) -O dtb -b 0 -@ 

# Default rule: Compile all .dts files
all: $(DTBO_FILES)

# Compile each .dts into .dtbo
$(OUT_DIR)/%.dtbo: $(SRC_DIR)/%.dts
	@mkdir -p $(OUT_DIR)
	$(DTC) $(DTC_FLAGS) -o $@ $<

# Clean up compiled .dtbo files
clean:
	rm -f $(DTBO_FILES)
	@echo "Cleaning up extlinux directory"
	rm -f /boot/p1/extlinux/*.dtbo

.PHONY: all clean
