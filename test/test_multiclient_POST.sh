#!/bin/bash

URL="http://localhost:6969/echo"
NUM_CLIENTS=50
POST_DATA="sigma"
HEADER="sigmabot"

# Function to perform a POST request
perform_post_request() {
    curl -s -o /dev/null -w "%{http_code}" -X POST -H "$HEADER" -d "$POST_DATA" $URL
}

# Run multiple clients in parallel
for i in $(seq 1 $NUM_CLIENTS); do
    perform_post_request &
done

# Wait for all background processes to finish
wait

echo "Completed $NUM_CLIENTS POST requests."