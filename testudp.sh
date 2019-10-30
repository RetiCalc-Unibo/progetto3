#!/bin/bash
for i in {1..300}
do
	./client_udp_testing localhost 2000 tema.txt > /dev/null 2>&1 &
done
