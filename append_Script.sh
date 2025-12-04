#!/bin/bash

# Configuration
LOG_FILE="./system_log.txt"

# --- CORE LOGIC ---

# 1. Check if the log file exists and create it if it doesn't
if [ ! -f "$LOG_FILE" ]; then
    echo "# --- Secure Application Log File ---" > "$LOG_FILE"
    echo "# Log protection status: Append-only attribute (a) applied using 'chattr'." >> "$LOG_FILE"
    echo "# Owner: $(whoami)" >> "$LOG_FILE"
    echo "# Last Protection Update: $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG_FILE"
    echo "" >> "$LOG_FILE"
    echo "Log file initialized."
fi

# 2. Get the log message and level from script arguments
LOG_LEVEL="$1"
LOG_MESSAGE="$2"

if [ -z "$LOG_LEVEL" ] || [ -z "$LOG_MESSAGE" ]; then
    echo "Usage: $0 <LEVEL> \"<MESSAGE>\""
    echo "Example: $0 INFO \"Server restarted.\""
    exit 1
fi

# 3. Format the timestamp
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S %Z')

# 4. Construct the log line
LOG_LINE="[${TIMESTAMP}] [${LOG_LEVEL^^}] ${LOG_MESSAGE}"

# 5. Append the log line
# If the append-only attribute ('a') is set on $LOG_FILE, this operation will succeed.
# Any attempt to overwrite, truncate, or delete the file will fail.
echo "$LOG_LINE" >> "$LOG_FILE"

if [ $? -eq 0 ]; then
    echo "Log entry appended successfully: ${LOG_LINE}"
else
    # This might happen if 'a' attribute is removed and file permissions are denied.
    echo "ERROR: Failed to write to log file. Check file permissions and 'a' attribute." >&2
    exit 2
fi

# Example usage:
# ./log_entry.sh INFO "A normal event occurred."
# ./log_entry.sh ERROR "A critical error!"

exit 0
