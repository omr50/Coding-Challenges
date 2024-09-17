#!/bin/bash

# Loop through ports 8081 to 8089
for (( port=8001; port<=8008; port++ )); do
    # Start Python HTTP server on each port
    nohup python3 -m http.server $port > /dev/null 2>&1 &
done

# Notify the user that servers are started
echo "Started Python HTTP servers on ports 8081 to 8089."
