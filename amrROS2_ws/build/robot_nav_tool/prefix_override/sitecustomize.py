import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/smr/workspaces/amrROS2_ws/install/robot_nav_tool'
