#include "common.h"

int cloudFd;
void signalHandle(int signo) //callback function
{
    
    char cloudBuff[MAX_BUFFER_SIZE];
    CloudMessage newCloudMessage;
    switch (signo){ //switch signal and send matched message to cloud
    case SIGUSR1:
        newCloudMessage.act = 1;
        memcpy(cloudBuff, &newCloudMessage, sizeof(newCloudMessage));
        write(cloudFd, cloudBuff, MAX_BUFFER_SIZE);
        break;
    case SIGUSR2:
        newCloudMessage.act = 0;
        memcpy(cloudBuff, &newCloudMessage, sizeof(newCloudMessage));
        write(cloudFd, cloudBuff, MAX_BUFFER_SIZE);
        break;
    default:
        break;
    }
}

int main()
{
        
    struct sigaction act; //define callback struct
    act.sa_handler = signalHandle;
    sigemptyset(&act.sa_mask);
    act.sa_flags = (SA_SIGINFO | SA_RESTART);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork error!");
        return -1;
    }
    if (pid == 0)// child program
    {
        
        if (access(SENSOR_FIFO, F_OK) == -1)//creat named pipe SENSOR_FIFO
        {
            if ((mkfifo(SENSOR_FIFO, 0777) < 0) && (errno != EEXIST)) 
            {
                perror("mkfifo SENSOR_FIFO error!");
                return -1;
            }
        }
        int sensorFd = open(SENSOR_FIFO, O_RDONLY); //open named pipe SENSOR_FIFO
        if (sensorFd == -1)
        {
            perror("open SENSOR_FIFO error!");
            return -1;
        }
        if (access(ACTU_FIFO, F_OK) == -1)//creat named pipe ACTU_FIFO
        {
            if ((mkfifo(ACTU_FIFO, 0777) < 0) && (errno != EEXIST))
            {
                perror("mkfifo ACTU_FIFO error!");
                return -1;
            }
        }
        int actuFd = open(ACTU_FIFO, O_WRONLY);//open named pipe ACTU_FIFO
        if (actuFd == -1)
        {
            perror("open ACTU_FIFO error!");
            return -1;
        }

        char sensorBuff[MAX_BUFFER_SIZE];
        char actuBuff[MAX_BUFFER_SIZE];
        int sensorCnt = 0;
        int preAct = -1;
        while (1)//keep read named pipe SENSOR_FIFO
        {
            bzero(sensorBuff, sizeof(sensorBuff));
            if (read(sensorFd, sensorBuff, MAX_BUFFER_SIZE) > 0)//read named pipe SENSOR_FIFO
            {
                SensorMessage newSensorMessage;
                memcpy(&newSensorMessage, sensorBuff, sizeof(newSensorMessage));
                if (newSensorMessage.flag == 0)//registe new sensor 
                {
                    printf("New Sensor Registered: \n  Pid: %d \n  Name: %s \n  Threshold: %d\n",
                           newSensorMessage.pid, newSensorMessage.name, newSensorMessage.threshold);
                    sensors[sensorCnt].pid = newSensorMessage.pid;
                    strcpy(sensors[sensorCnt].name, newSensorMessage.name);
                    sensors[sensorCnt].threshold = newSensorMessage.threshold;
                    sensorCnt++;
                }
                else if (newSensorMessage.flag == 1)//receive new sensed data from registered sensor 
                {
                    for (int i = 0; i < SENSOR_NUM; i++)//find that which sensor send this data
                    {
                        if (newSensorMessage.pid == sensors[i].pid)
                        {
                            printf("New Sensed Data from Pid %d : %d\n", newSensorMessage.pid, newSensorMessage.newdata);
                            ActuMessage newActuMessage;
                            if (newSensorMessage.newdata > sensors[i].threshold)//check if new data biggger than threshold
                            {
                                newActuMessage.act = 1;
                            }
                            else
                            {
                                newActuMessage.act = 0;
                            }
                            if(preAct != newActuMessage.act){//check if actuator need to be changed
                                bzero(actuBuff, sizeof(actuBuff));//send new state to actuator
                                memcpy(actuBuff, &newActuMessage, sizeof(newActuMessage));
                                write(actuFd, actuBuff, MAX_BUFFER_SIZE);
                                if(newActuMessage.act == 0){//choose right signal and send it to parent program
                                    if(kill(getppid(), SIGUSR2) == -1){
                                        perror("fail to send signal\n");
                                        return -1;
                                    }
                                }
                                else{
                                    if(kill(getppid(), SIGUSR1) == -1){
                                        perror("fail to send signal\n");
                                        return -1;
                                    }
                                }
                                preAct = newActuMessage.act;//update current actuator state
                            }
                            break;
                        }
                    }
                }
            }
        }
        close(sensorFd);//close named pipe and delete it
        close(actuFd);
        unlink(SENSOR_FIFO);
    }
    else // parent program
    {
        if (access(CLOUD_FIFO, F_OK) == -1) //create named pipe
        {   
            if ((mkfifo(CLOUD_FIFO, 0777) < 0) && (errno != EEXIST))
            {  
                perror("mkfifo CLOUD_FIFO error!");
                return -1;
            }
        }
        cloudFd = open(CLOUD_FIFO, O_WRONLY);//open named pipe
        if (cloudFd == -1)
        {
            perror("open CLOUD_FIFO error!");
            return -1;
        }
	
        while(1){//keep listen signal
    	}
    	close(cloudFd);//close named pipe
    }
    return 0;
}
