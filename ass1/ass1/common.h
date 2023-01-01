#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h> 
#include <time.h>
#define SENSOR_FIFO "./sensorfifo"
#define ACTU_FIFO "./actufifo"
#define CLOUD_FIFO "./cloudfifo"
#define MAX_BUFFER_SIZE 1024
#define SENSOR_NUM 2
typedef struct sensor
{
    pid_t pid;
    char name[20];
    int threshold;
} Sensor;
Sensor sensors[SENSOR_NUM];

typedef struct sensorMessage
{
    int flag; // 0:new sensor  1:new sensed data
    pid_t pid;
    char name[20];
    int threshold;
    int newdata;
} SensorMessage;

typedef struct actuMessage
{
    int act; // 0 close 1 open
} ActuMessage;

typedef struct cloudMessage
{
    int act; // 0 close 1 open
} CloudMessage;

