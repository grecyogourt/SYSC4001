#include "common.h"

int main()
{
    srand((int)time(0));//create rand seed
    char sensorBuff[MAX_BUFFER_SIZE];
    if (access(SENSOR_FIFO, F_OK) == -1)//creat named pipe SENSOR_FIFO
    {
        if ((mkfifo(SENSOR_FIFO, 0777) < 0) && (errno != EEXIST))
        {
            perror("mkfifo SENSOR_FIFO error!");
            return -1;
        }
    }
    int sensorFd = open(SENSOR_FIFO, O_WRONLY);//open named pipe SENSOR_FIFO
    if (sensorFd == -1)
    {
        perror("open SENSOR_FIFO error!");
        return -1;
    }

    SensorMessage newSensorMessage;
    
    //send new sensor register data to controller
    newSensorMessage.flag = 0;
    newSensorMessage.pid = getpid();
    strcpy(newSensorMessage.name, "sensor");
    newSensorMessage.threshold = rand() % 50;
    bzero(sensorBuff, sizeof(sensorBuff));
    memcpy(sensorBuff, &newSensorMessage, sizeof(newSensorMessage));
    write(sensorFd, sensorBuff, MAX_BUFFER_SIZE);
    printf("Send Register: \n  PID: %d \n  name: %s \n  Threshold: %d \n", 
    newSensorMessage.pid, newSensorMessage.name, newSensorMessage.threshold);
    
    while (1)//keep send new sensed data to controller
    {
        sleep(1);//sleep 1 second
        //send new sensed data to controller
        memset(&newSensorMessage, 0, sizeof(newSensorMessage));
        newSensorMessage.flag = 1;
        newSensorMessage.pid = getpid();
        newSensorMessage.newdata = rand() % 50;
        bzero(sensorBuff, sizeof(sensorBuff));
        memcpy(sensorBuff, &newSensorMessage, sizeof(newSensorMessage));
        write(sensorFd, sensorBuff, MAX_BUFFER_SIZE);
        printf("PID %d Send New Sensed Data : %d \n", 
        newSensorMessage.pid, newSensorMessage.newdata);
    }
    close(sensorFd);//close named pipe SENSOR_FIFO
    return 0;
}
