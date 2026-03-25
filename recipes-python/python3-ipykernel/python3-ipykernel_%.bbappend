# The debugpy background thread is known to consume significant CPU (e.g. 87%)
# on embedded ARM systems like the Beaglebone Black while idle.
# By removing the python3-debugpy runtime dependency, ipykernel gracefully
# falls back and disables the debugging features and their CPU-heavy threads.
RDEPENDS:${PN}:remove = "python3-debugpy"
