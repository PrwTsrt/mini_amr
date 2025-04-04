#!/bin/bash

# ฟังก์ชัน: หยุด process เดิมถ้ามี
kill_old_autodock() {
  echo "🔍 Checking for existing auto_dock_launch.py..."

  PID=$(pgrep -f "auto_dock_launch.py")

  if [ -n "$PID" ]; then
    echo "⚠️  Found existing auto_dock_launch.py process (PID: $PID), killing it..."
    kill "$PID"
    sleep 1
  else
    echo "✅ No existing auto_dock_launch.py process found."
  fi
}

# เริ่มทำงาน
echo "🚀 Autodock Launcher Script Starting..."
kill_old_autodock

# เปิด terminal เพื่อ launch ระบบ autodock
gnome-terminal -- bash -c "source ~/.bashrc; ros2 launch action_autodock auto_dock_launch.py; exec bash"

# รอ 2 วินาที
sleep 2

# เปิด terminal เพื่อส่ง action goal
gnome-terminal -- bash -c "source ~/.bashrc; ros2 action send_goal --feedback autodock custom_interface/action/Autodock '{is_dock: False}'; exec bash"

echo "✅ Done!"