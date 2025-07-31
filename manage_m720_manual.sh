#!/bin/bash
# M720 Button Remapper Manual Management Script

SCRIPT_NAME="m720_button_remapper_safe.py"
AUTOSTART_FILE="$HOME/.config/autostart/m720-remapper.desktop"

case "$1" in
    start)
        echo "Starting M720 button remapper manually..."
        if pgrep -f "$SCRIPT_NAME" > /dev/null; then
            echo "M720 remapper is already running!"
        else
            cd /home/usmandauna/Documents/extras/logitect-m7-20-key-remaper-linux
            ./start_m720_remapper.sh
            echo "M720 remapper started"
        fi
        ;;
    stop)
        echo "Stopping M720 button remapper..."
        sudo pkill -f "$SCRIPT_NAME"
        echo "M720 remapper stopped"
        ;;
    restart)
        echo "Restarting M720 button remapper..."
        sudo pkill -f "$SCRIPT_NAME"
        sleep 2
        cd /home/usmandauna/Documents/extras/logitect-m7-20-key-remaper-linux
        ./start_m720_remapper.sh
        echo "M720 remapper restarted"
        ;;
    status)
        echo "M720 button remapper status:"
        if pgrep -f "$SCRIPT_NAME" > /dev/null; then
            echo "M720 remapper is running"
            echo "Process ID: $(pgrep -f "$SCRIPT_NAME")"
        else
            echo "M720 remapper is not running"
        fi
        ;;
    enable-autostart)
        echo "Enabling M720 remapper autostart..."
        if [ -f "$AUTOSTART_FILE" ]; then
            echo "Autostart is already enabled"
        else
            cp /home/usmandauna/Documents/extras/logitect-m7-20-key-remaper-linux/m720-remapper.desktop "$AUTOSTART_FILE"
            echo "Autostart enabled"
        fi
        ;;
    disable-autostart)
        echo "Disabling M720 remapper autostart..."
        if [ -f "$AUTOSTART_FILE" ]; then
            rm "$AUTOSTART_FILE"
            echo "Autostart disabled"
        else
            echo "Autostart was not enabled"
        fi
        ;;
    test)
        echo "Testing M720 button remapper (manual run)..."
        cd /home/usmandauna/Documents/extras/logitect-m7-20-key-remaper-linux
        sudo python3 m720_button_remapper_safe.py
        ;;
    *)
        echo "M720 Button Remapper Manual Manager"
        echo "Usage: $0 {start|stop|restart|status|enable-autostart|disable-autostart|test}"
        echo ""
        echo "Commands:"
        echo "  start            - Start the remapper manually"
        echo "  stop             - Stop the remapper"
        echo "  restart          - Restart the remapper"
        echo "  status           - Show remapper status"
        echo "  enable-autostart - Enable autostart on login"
        echo "  disable-autostart- Disable autostart on login"
        echo "  test             - Run remapper manually for testing"
        echo ""
        echo "Current status:"
        if pgrep -f "$SCRIPT_NAME" > /dev/null; then
            echo "M720 remapper is running"
        else
            echo "M720 remapper is not running"
        fi
        
        if [ -f "$AUTOSTART_FILE" ]; then
            echo "Autostart is enabled"
        else
            echo "Autostart is disabled"
        fi
        ;;
esac
