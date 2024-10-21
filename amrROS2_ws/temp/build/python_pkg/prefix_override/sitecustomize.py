import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/smr/workspaces/amrROS2_ws/install/python_pkg'
