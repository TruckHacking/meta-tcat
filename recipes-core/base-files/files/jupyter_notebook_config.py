c = get_config()
c.ServerApp.ip = '0.0.0.0'

# Do not try to open a browser on the BeagleBone
c.ServerApp.open_browser = False

# Set a static port (default is 8888)
c.ServerApp.port = 8888

# Set max buffer size to 10MB
c.ServerApp.max_buffer_size = 10485760

# Performance: Disable heavy extensions
c.ServerApp.jpserver_extensions = {
    'jupyter_lsp': False
}

# Performance: Disable the LSP (Language Server Protocol) completely
c.LanguageServerApp.enabled = False

# Performance: Disable background debugger threads in IPython kernels
# This prevents ipykernel from starting the debugpy background listener
c.IPythonKernel.use_debugpy = False

import subprocess
from jupyter_server.serverapp import ServerApp

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

def custom_display_url_property(self):
    ips = get_all_ips()
    urls = []
    try:
        parts = self._get_urlparts(include_token=True)
        for ip in ips:
            netloc = f"{ip}:{self.port}"
            urls.append(parts._replace(netloc=netloc).geturl())
    except Exception:
        pass
    
    notice = "NOTICE: These numeric IP addresses are the recommended way to connect:\n        "
    return notice + "\n    or  ".join(urls)

ServerApp.display_url = property(custom_display_url_property)
