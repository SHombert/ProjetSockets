#!/bin/bash
gcc server.c -o server -lpthread

echo "======================"

gcc client.c -o client