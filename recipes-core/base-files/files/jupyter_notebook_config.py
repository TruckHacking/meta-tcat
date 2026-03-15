c = get_config()
c.NotebookApp.enable_mathjax = False

# Accept connections from any IP on your local network
c.NotebookApp.ip = '0.0.0.0'

# Do not try to open a browser on the BeagleBone
c.NotebookApp.open_browser = False

# Set a static port (default is 8888)
c.NotebookApp.port = 8888

# Optional: Set a password so you don't have to copy tokens every time
# from notebook.auth import passwd
# You can generate a hash by running `passwd()` in a python shell
# c.NotebookApp.password = u'sha1:your_generated_hash_here'
