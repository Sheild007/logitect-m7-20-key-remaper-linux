#!/bin/bash
# M720 Button Remapper Startup Script

# Wait for desktop to be ready
sleep 10

# Start ydotool daemon if not running
if ! pgrep ydotoold > /dev/null; then
    sudo ydotoold &
    sleep 2
fi

# Start the button remapper
cd /home/usmandauna/Documents/extras/logitect-m7-20-key-remaper-linux
sudo python3 m720_button_remapper_safe.py &

echo "M720 button remapper started"
