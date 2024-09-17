#!/bin/bash

# Loop through ports 8081 to 8089
for (( port=8001; port<=8008; port++ )); do
	sudo fuser -k ${port}/tcp
	echo "Ended Python HTTP server on port ${port}."
done

# Notify the user that servers are started
