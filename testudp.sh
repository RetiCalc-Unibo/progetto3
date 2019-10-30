#!/bin/bash
for i in {1..200}
do
	./client_udp_testing localhost 2000 rockyou.txt > /dev/null 2>&1 &
done
