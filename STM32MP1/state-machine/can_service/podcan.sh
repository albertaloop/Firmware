#!/bin/bash

CANDUMP_LOG_DIR='/home/root/can_service'
LOG_DIR='/home/root/can_service'
APP_LOG_DIR='/home/root'
CANDUMP_LOG_PREFIX='candump'
LOG_PREFIX='podcan'
APP_LOG_PREFIX='main'
CANDUMP_LOGFILE=""
LOGFILE=""
LOGFILE_MAX_SIZE=$((1 * 1024 * 1024))  # 1 MB
MAX_LOGFILES=1
ETH_INTERFACE="end0"  # Replace with your Ethernet interface name
APP_PATH="/home/root/main"

mkdir -p "$CANDUMP_LOG_DIR"
mkdir -p "$LOG_DIR"

setup_can_interface() {
    echo "Setting up CAN interface" >> "$LOGFILE"
    ip link set can0 down
    ip link set can0 type can bitrate 250000 fd off
    ip link set can0 up
}

bring_can_interface_up() {
    echo "Bringing can0 back up." >> "$LOGFILE"
    ip link set can0 down
    ip link set can0 up
}

is_can0_up() {
    ip link show can0 | grep -q "state UP"
    return $?
}

start_candump() {
    echo "Restarting candump." >> $LOGFILE
    TIMESTAMP=$(date +"%Y%m%d%H%M%S")
    CANDUMP_LOGFILE="$CANDUMP_LOG_DIR/${CANDUMP_LOG_PREFIX}_${TIMESTAMP}.log"
    candump can0 > "$CANDUMP_LOGFILE" &
    CANDUMP_PID=$!
    rotate_log
}

monitor_candump() {
    grep -q "bus-off" "$CANDUMP_LOGFILE"
    return $?
}

rotate_log() {
    CANDUMP_LOGFILES=($(ls -t "$CANDUMP_LOG_DIR/${CANDUMP_LOG_PREFIX}_*.log"))
    if [ "${#CANDUMP_LOGFILES[@]}" -gt "$MAX_LOGFILES" ]; then
        for ((i = MAX_LOGFILES; i < ${#CANDUMP_LOGFILES[@]}; i++)); do
            rm -f "${CANDUMP_LOGFILES[i]}"
        done
    fi
    LOGFILES=($(ls -t "$LOG_DIR/${LOG_PREFIX}_*.log"))
    if [ "${#LOGFILES[@]}" -gt "$MAX_LOGFILES" ]; then
        for ((i = MAX_LOGFILES; i < ${#LOGFILES[@]}; i++)); do
            rm -f "${LOGFILES[i]}"
        done
    fi
    APP_LOGFILES=($(ls -t "$APP_LOG_DIR/${APP_LOG_PREFIX}_*.log"))
    if [ "${#APP_LOGFILES[@]}" -gt "$MAX_LOGFILES" ]; then
        for ((i = MAX_LOGFILES; i < ${#APP_LOGFILES[@]}; i++)); do
            rm -f "${APP_LOGFILES[i]}"
        done
    fi
}

check_logfile_size() {
    if [ -e "$CANDUMP_LOGFILE" ] && [ -e $"$LOGFILE" ]; then
        LOGFILE_SIZE=$(stat -c%s "$CANDUMP_LOGFILE")
        if [ "$LOGFILE_SIZE" -gt "$LOGFILE_MAX_SIZE" ]; then
            echo "candump logfile too large. Creating new candump log file" >> "$LOGFILE"
            kill $CANDUMP_PID
            start_candump
        fi
        LOGFILE_SIZE=$(stat -c%s "$LOGFILE")
        if [ "$LOGFILE_SIZE" -gt "$LOGFILE_MAX_SIZE" ]; then
            echo "Logfile too large. Log file" >> "$LOGFILE"
            TIMESTAMP=$(date +"%Y%m%d%H%M%S")
            LOGFILE="$LOG_DIR/${LOG_PREFIX}_${TIMESTAMP}.log"
        fi
        LOGFILE_SIZE=$(stat -c%s "$APP_LOGFILE")
        if [ "$LOGFILE_SIZE" -gt "$LOGFILE_MAX_SIZE" ]; then
            echo "Logfile too large. Log file" >> "$LOGFILE"
            TIMESTAMP=$(date +"%Y%m%d%H%M%S")
            APP_LOGFILE="$APP_LOG_DIR/${APP_LOG_PREFIX}_${TIMESTAMP}.log"
        fi
    fi
}

is_ethernet_up() {
    ip addr show "$ETH_INTERFACE" | grep -q "inet "
    return $?
}


TIMESTAMP=$(date +"%Y%m%d%H%M%S")
LOGFILE="$LOG_DIR/${LOG_PREFIX}_${TIMESTAMP}.log"
APP_LOGFILE="$APP_LOG_DIR/${APP_LOG_PREFIX}_${TIMESTAMP}.log"

setup_can_interface
start_candump


# Wait until Ethernet IP is assigned before starting the application
echo "Waiting for IP address on $ETH_INTERFACE " >>  "$LOGFILE"
while true; do
    if is_ethernet_up; then
        break
    fi
    sleep 1
done
echo "Found IP address on $ETH_INTERFACE " >>  "$LOGFILE"

echo "Starting application" >>  "$LOGFILE"

$APP_PATH > "$APP_LOGFILE" &
APP_PID=$!

echo "Application with pid $! started" >>  "$LOGFILE"


while true; do

    # Check CAN interface and rotate logs as necessary
    if ! is_can0_up || monitor_candump; then
        kill $CANDUMP_PID
        bring_can_interface_up
        start_candump

    fi
    check_logfile_size

    # Restart the application only if the Ethernet interface goes down
    if ! is_ethernet_up; then
        echo "Ethernet is down. Waiting for Ethernet IP to be assigned." >> "$LOGFILE"
        kill $APP_PID
        while true; do
            if is_ethernet_up; then
                break
            fi
            sleep 1
        done
        $APP_PATH > "$APP_LOGFILE" &
        APP_PID=$!
    fi

    sleep 5
done