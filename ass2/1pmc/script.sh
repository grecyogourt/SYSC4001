#! /bin/bash
gcc producer.c -o producer -lpthread
gcc consumer.c -o consumer -lpthread
xterm -e ./producer input.txt  &
xterm -e ./consumer output.txt 4 &
