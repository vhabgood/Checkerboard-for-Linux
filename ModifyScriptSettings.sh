#!/bin/bash

# This script manages the 'append-only' attribute using 'chattr'.
# WARNING: Only the root user or the file owner can remove this attribute!
# This is how you enforce protection.

LOG_FILE="./system_log.txt"

# Check for required command
if ! command -v chattr &> /dev/null; then
    echo "ERROR: 'chattr' command not found. This is usually part of the 'e2fsprogs' package."
    exit 1
fi

# Check if the log file exists
if [ ! -f "$LOG_FILE" ]; then
    echo "ERROR: Log file '$LOG_FILE' not found. Run log_entry.sh first to create it."
    exit 1
fi

# --- CORE FUNCTIONS ---

function check_status() {
    # 'lsattr' lists file attributes. The 'a' means append-only.
    STATUS=$(lsattr -d "$LOG_FILE" 2>/dev/null)
    if echo "$STATUS" | grep -q "a"; then
        echo "File '$LOG_FILE' is currently PROTECTED (Append-Only: YES)."
        echo "To modify or delete the file, you must first run: $0 unlock"
    else
        echo "File '$LOG_FILE' is currently UNPROTECTED (Append-Only: NO)."
        echo "To protect it, run: $0 lock"
    fi
}

function lock_file() {
    echo "Attempting to LOCK file: $LOG_FILE (setting 'a' attribute)..."
    # chattr +a adds the append-only attribute
    sudo chattr +a "$LOG_FILE"
    if [ $? -eq 0 ]; then
        echo "SUCCESS: File is now locked. Only root or owner can append. No one can overwrite or delete."
        check_status
    else
        echo "FAILURE: Could not set 'a' attribute. You may need to run this script with 'sudo'."
    fi
}

function unlock_file() {
    echo "Attempting to UNLOCK file: $LOG_FILE (removing 'a' attribute)..."
    # chattr -a removes the append-only attribute
    sudo chattr -a "$LOG_FILE"
    if [ $? -eq 0 ]; then
        echo "SUCCESS: File is now unlocked. All permissions (read/write/delete) are restored."
        check_status
    else
        echo "FAILURE: Could not remove 'a' attribute. You must run this script with 'sudo' and be the owner or root."
    fi
}

# --- CLI COMMAND HANDLING ---

COMMAND="$1"

case "$COMMAND" in
    lock)
        lock_file
        ;;
    unlock)
        unlock_file
        ;;
    status)
        check_status
        ;;
    *)
        echo "Usage: $0 <command>"
        echo "Commands:"
        echo "  lock    - Apply the append-only attribute (+a) to $LOG_FILE."
        echo "  unlock  - Remove the append-only attribute (-a) from $LOG_FILE."
        echo "  status  - Check the current status of the 'a' attribute."
        exit 1
        ;;
esac
