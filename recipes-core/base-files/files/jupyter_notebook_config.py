c = get_config()
c.NotebookApp.enable_mathjax = False

# Accept connections from any IP on your local network
c.NotebookApp.ip = '0.0.0.0'

# Do not try to open a browser on the BeagleBone
c.NotebookApp.open_browser = False

# Set a static port (default is 8888)
c.NotebookApp.port = 8888

# Set max buffer size to 10MB
c.NotebookApp.max_buffer_size = 10485760

import subprocess

def get_all_ips():
    ips = []
    try:
        # Run ip command to list all IPv4 and IPv6 addresses
        output = subprocess.check_output(['ip', 'addr', 'show']).decode('utf-8')
        for line in output.split('\n'):
            line = line.strip()
            # Find all non-loopback inet and inet6 addresses
            if line.startswith('inet ') or line.startswith('inet6 '):
                parts = line.split()
                if len(parts) >= 2:
                    ip = parts[1].split('/')[0]
                    if ip != '127.0.0.1' and ip != '::1' and not ip.startswith('fe80:'):
                        if ':' in ip:
                            ips.append(f"[{ip}]")
                        else:
                            ips.append(ip)
    except Exception:
        pass
    # Fallback for BeagleBone USB interface
    if not ips:
        ips.append('192.168.7.2')
    return ips

ips = get_all_ips()
urls = [f"http://{ip}:{c.NotebookApp.port}/" for ip in ips]

# We use custom_display_url to suggest the numeric IPs.
# Note: Jupyter will automatically append the authentication token to the end of this string.
c.NotebookApp.custom_display_url = "NOTICE: These numeric IP addresses are the recommended way to connect:\n    " + "\n    or ".join(urls)

# Optional: Set a password so you don't have to copy tokens every time
# from notebook.auth import passwd
# You can generate a hash by running `passwd()` in a python shell
# c.NotebookApp.password = u'sha1:your_generated_hash_here'
