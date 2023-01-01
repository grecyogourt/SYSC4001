#include "common.h"

int main()
{
    if (access(CLOUD_FIFO, F_OK) == -1)//creat named pipe CLOUD_FIFO
    {
        if ((mkfifo(CLOUD_FIFO, 0777) < 0) && (errno != EEXIST))
        {
            perror("mkfifo CLOUD_FIFO error!");
            return -1;
        }
    }
    int cloudFd = open(CLOUD_FIFO, O_RDONLY);//open named pipe CLOUD_FIFO
    if (cloudFd == -1)
    {
        perror("open CLOUD_FIFO error!");
        return -1;
    }

    char cloudBuff[MAX_BUFFER_SIZE];
    while (1)//keep read named pipe CLOUD_FIFO
    {
        bzero(cloudBuff, sizeof(cloudBuff));
        if (read(cloudFd, cloudBuff, MAX_BUFFER_SIZE) > 0)//receive new state of actuator
        {
            CloudMessage newCloudMessage;
            memcpy(&newCloudMessage, cloudBuff, sizeof(newCloudMessage));
            if (newCloudMessage.act == 0)//print new state of actuator
            {
                printf("An Actuator Has Been Closed!\n");
            }
            else
            {
                printf("An Actuator Has Been Opened!\n");
            }
        }
    }
    close(cloudFd);//close and delete named pipe CLOUD_FIFO
    unlink(CLOUD_FIFO);
    return 0;
}
