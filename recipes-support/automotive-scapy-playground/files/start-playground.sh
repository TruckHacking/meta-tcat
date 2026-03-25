#!/bin/sh
# Copy jupyter notebooks and pcapng files from data directory to current directory
cp -f /usr/share/automotive_scapy_playground/*.ipynb .
cp -f /usr/share/automotive_scapy_playground/*.pcapng .

# Launch jupyter notebook skipping the web browser
export PYDEVD_DISABLE_FILE_VALIDATION=1
jupyter notebook --no-browser --ip=0.0.0.0 --port=8888 --allow-root
