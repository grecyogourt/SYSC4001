#include "common.h"

int main()
{
    if (access(ACTU_FIFO, F_OK) == -1)//creat named pipe ACTU_FIFO
    {
        if ((mkfifo(ACTU_FIFO, 0666) < 0) && (errno != EEXIST))
        {
            perror("mkfifo ACTU_FIFO error!");
            return -1;
        }
    }
    int actuFd = open(ACTU_FIFO, O_RDONLY);//open named pipe ACTU_FIFO
    if (actuFd == -1)
    {
        perror("open ACTU_FIFO error!");
        return -1;
    }

    char actuBuff[MAX_BUFFER_SIZE];
    while (1)//keep read named pipe ACTU_FIFO
    {
        bzero(actuBuff, sizeof(actuBuff));
        if (read(actuFd, actuBuff, MAX_BUFFER_SIZE) > 0)//receive new order of actuator
        {
            ActuMessage newActuMessage;
            memcpy(&newActuMessage, actuBuff, sizeof(newActuMessage));
            if (newActuMessage.act == 0)//print new order of actuator
            {
                printf("Close Actuator!\n");
            }
            else
            {
                printf("Open Actuator!\n");
            }
        }
    }
    close(actuFd);//close and delete named pipe ACTU_FIFO
    unlink(ACTU_FIFO);
    return 0;
}
