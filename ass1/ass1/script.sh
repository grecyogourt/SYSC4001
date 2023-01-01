#! /bin/bash
gcc controller.c -o controller
gcc sensor.c -o sensor
gcc cloud.c -o cloud
gcc actu.c -o actu
xterm -e ./controller -a controller &
xterm -e ./actu -a actu &
xterm -e ./cloud -a cloud &
xterm -e ./sensor -a sensor1 &
sleep 1
xterm -e ./sensor -a sensor2 &
