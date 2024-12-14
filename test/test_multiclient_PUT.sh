#!/bin/bash

URL="http://localhost:6969/file/bus.txt"
NUM_CLIENTS=50
DATA=("cek1 cek2 cek3" "data1 data2 data3" "msg1 msg2 msg3")

# Function to perform a PUT request with random data
perform_put_request() {
    RANDOM_DATA=${DATA[$RANDOM % ${#DATA[@]}]}
    curl -s -o /dev/null -w "%{http_code}" -X PUT -d "$RANDOM_DATA" $URL
}

# Run multiple clients in parallel
for i in $(seq 1 $NUM_CLIENTS); do
    perform_put_request &
done

# Wait for all background processes to finish
wait

echo "Completed $NUM_CLIENTS PUT requests."