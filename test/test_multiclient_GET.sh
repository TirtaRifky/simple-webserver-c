#!/bin/bash

URL="http://localhost:6969/"
NUM_CLIENTS=50

# Function to perform a GET request
perform_get_request() {
    curl -s -o /dev/null -w "%{http_code}" $URL
}

# Run multiple clients in parallel
for i in $(seq 1 $NUM_CLIENTS); do
    perform_get_request &
done

# Wait for all background processes to finish
wait

echo "Completed $NUM_CLIENTS GET requests."