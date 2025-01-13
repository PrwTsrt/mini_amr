import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/prawicht/workspaces/mini_amr/amrROS2_ws/install/robot_nav_tool'
