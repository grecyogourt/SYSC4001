#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>

#define SHM_SEM_S_KEY 	0x1234  		
#define SHM_SEM_E_KEY 	0x5678  		
#define SHM_SEM_N_KEY 	0xabcd  		
#define SHM_MEM_KEY   	0xfedc

#define SEM_N_INIT_NUM	0
#define SEM_S_INIT_NUM 	1
#define SEM_E_INIT_NUM 	(NUMBER_OF_BUFFERS - 1)


#define MICRO_SEC_IN_SEC 	1000000

#define SHM_PC_ERR_EXIT(m) do { \
    perror(m); \
    exit(EXIT_FAILURE); \
    } while(0)

//each buffers having 128 bytes
#define BUF_SIZE 		128

//The shared memory contains 100 buffers
#define NUMBER_OF_BUFFERS 	100 




//buffer item Structures
typedef struct shm_buf_s {
    int count;
    char buffer[BUF_SIZE];
} shm_buf_t;

//buffer Structures
typedef struct {
    int file_size;
    int buf_index;
    shm_buf_t shm_buf[NUMBER_OF_BUFFERS];  
} shm_file_buf;




union semun {
    int val;                    /* value for SETVAL */
    struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
    unsigned short int *array;  /* array for GETALL, SETALL */
    struct seminfo *__buf;      /* buffer for IPC_INFO */
};

//create key
int sem_create(key_t key);

// set value for sem
int sem_set_value(int sem_id, int sem_value);

//get value for sem
int sem_getval(int semid);

//delete sem
void sem_delete(int sem_id);

// wait sem
int sem_wait(int sem_id);

//comsume sem
int sem_signal(int sem_id);

//Creating Shared Memory
void *shm_creat(int skm_key, int shm_size, int *shm_id);

//delete Shared Memory
void shm_delete(int shm_id, void *shm_addr);

int 	sem_n_id, sem_s_id, sem_e_id;
int 	shmid;
void 	*shm_addr;

shm_file_buf *g_shared_file_buf;


int  produce_number = 1;

pthread_t   *p_threadProduce;

int file_fd, readn;
char *filename;

void *pRev(void *d)
{
	  char file_data[BUF_SIZE];
    while (1) {

        readn = read(file_fd, file_data, BUF_SIZE);
        //printf("read file size %d\n", readn);
        if (readn <= 0) {
            break;
        }	    
        
        //wait buf have space size
        sem_wait(sem_e_id);
        
        //mutex lock
        sem_wait(sem_s_id);
        
        g_shared_file_buf->shm_buf[g_shared_file_buf->buf_index].count = readn;			
        memset(g_shared_file_buf->shm_buf[g_shared_file_buf->buf_index].buffer, 0, BUF_SIZE);	
        memcpy( g_shared_file_buf->shm_buf[g_shared_file_buf->buf_index].buffer, file_data, readn);
        g_shared_file_buf->buf_index++;
        if (g_shared_file_buf->buf_index == NUMBER_OF_BUFFERS) {
            g_shared_file_buf->buf_index = 0;
        }
        
        //mutex unlock
        sem_signal(sem_s_id);
        
        //wakeup consumer
      sem_signal(sem_n_id);
      }
}

int main(int argc, char *argv[]) 
{   

    struct stat st;
    
    if (argc != 3) {
        printf("usag:%s filename produce_number\n", argv[0]);
        return -1;
    }
    
    filename = argv[1];
    produce_number = atoi(argv[2]);
    
    //Open file
    file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        printf("open %s error\n", filename);
        return -1;
    }
    p_threadProduce = malloc(sizeof(pthread_t) * produce_number); 
    //Creating and initializing semaphores sem_n_id
    sem_n_id = sem_create(SHM_SEM_N_KEY);
    sem_set_value(sem_n_id, SEM_N_INIT_NUM);
    sem_getval(sem_n_id);
    
    //Creating and initializing semaphores sem_s_id
    sem_s_id = sem_create(SHM_SEM_S_KEY);
    sem_set_value(sem_s_id, SEM_S_INIT_NUM);
    sem_getval(sem_s_id);
    
    //Creating and initializing semaphores sem_e_id
    sem_e_id = sem_create(SHM_SEM_E_KEY);
    sem_set_value(sem_e_id, SEM_E_INIT_NUM);
    sem_getval(sem_e_id);
    
    //Creating Shared Memory
    shm_addr = shm_creat(SHM_MEM_KEY, sizeof(shm_file_buf), &shmid);
    g_shared_file_buf = (shm_file_buf *)shm_addr;
    
    stat(filename, &st);
    g_shared_file_buf->file_size = st.st_size;
    printf("file %s size: %d\n", filename, g_shared_file_buf->file_size);
    
    
    for(int i = 0; i < produce_number;i++)
    {
    	 pthread_create(&p_threadProduce[i], NULL, pRev, NULL);
    }
    
    for(int i = 0; i < produce_number;i++)
    {
    	 pthread_join(p_threadProduce[i], NULL);
    }
    
    sleep(1);
    
    close(file_fd);
    free(p_threadProduce);
    return 0;
}

int sem_create(key_t key)
{
    int semid;
    
    semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        if (errno == EEXIST) {
            printf("semget EEXIST\n");
            semid = semget(key, 0, 0);
            if (semid == -1) {
                SHM_PC_ERR_EXIT("semget");
            }
        } else {
            SHM_PC_ERR_EXIT("semget");
        }
    }
    
    return semid;
}
int sem_set_value(int sem_id, int sem_value)
{
    union semun sem_union;
    sem_union.val = sem_value;
    
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        SHM_PC_ERR_EXIT("semctl");
        return 0;
    }
    
    return 1;
}

int sem_getval(int semid)
{
    int ret;
    ret = semctl(semid, 0, GETVAL,0);
    if (ret==-1)
        SHM_PC_ERR_EXIT("sem getval");
    
    printf("semid 0x%x, current val is %d\n", semid, ret);
    
    return ret;
}

void sem_delete(int sem_id)
{
    union semun sem_union;
    
    semctl(sem_id, 0, IPC_RMID, sem_union);
}

int sem_wait(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = 0;
    
    if (semop(sem_id, &sem_b, 1) == -1) {
        SHM_PC_ERR_EXIT("semop");
        return 0;
    }
    
    return 1;
}

int sem_signal(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = 0;
    if (semop(sem_id, &sem_b, 1) == -1) {
        SHM_PC_ERR_EXIT("semop");
        return 0;
    }
    
    return 1;
}

void *shm_creat(int skm_key, int shm_size, int *shm_id)
{
    void *addr;
    
    *shm_id = shmget((key_t)skm_key, shm_size, 0666 | IPC_CREAT);
    if (*shm_id == -1) {
        SHM_PC_ERR_EXIT("shmget");
        return NULL;
    }
    
    addr = shmat(*shm_id, 0, 0);
    if (addr == (void *)-1) {
        SHM_PC_ERR_EXIT("shmget");
        return NULL;
    }
    
    return addr;
}

void shm_delete(int shm_id, void *shm_addr)
{
    shmdt(shm_addr);
    
    shmctl(shm_id, IPC_RMID, 0);
    
    return;
}
