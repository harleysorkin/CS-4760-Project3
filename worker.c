#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#define SHMKEY 1840831
#define MSGKEY 1840832
#define BUFF_SIZE sizeof(int)

typedef struct msgbuf {
    long mtype;
    char mtext[10];
} critMsg;

int main(int argc, char *argv[]) {

    int m = atoi(argv[1]);
    int shmid, msqid;
    critMsg msg;
    msg.mtype = 1;

    if((shmid = shmget(SHMKEY, BUFF_SIZE, 0777)) == -1) {
        perror("Error: shmget:\n");
        exit(1);
    }

    if((msqid = msgget(MSGKEY, 0777)) == -1) {
        perror("Error: msgget:\n");
        exit(1);
    } 

    int *shmem = (int *)(shmat(shmid, 0, 0));
    int *clockSec = shmem;
    int *clockNano = clockSec + 1;

    int increase = m * 1000000;
    int i = 0;
    
    
    while(i < m) {
        if(msgrcv(msqid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
            perror("Error: msgrcv:\n");
            exit(1);
        }

	int pid = getpid();
        time_t t = time(NULL);
        printf("worker_pid %d : ", pid);
        printf("Got in critical section: Time: %ld\n\n", t);
        
        *clockNano += increase;
        if(*clockNano >= 1000000000) {
            while(*clockNano >= 1000000000) {
                *clockNano -= 1000000000;
                *clockSec += 1;
            }
        }

        i++;
        printf("worker_pid %d : ", pid);
        printf("Iteration %d : ", i);
        printf("Incrementing by %d\n\n", increase);

        t = time(NULL);
        printf("worker_pid %d : ", pid);
        printf("Leaving critical section: Time: %ld\n\n", t);

        if(msgsnd(msqid, &msg, sizeof(msg.mtext), 0) == -1) {
            perror("Error: msgsend:\n");
            exit(1);
        }
    }

    return 0;

}
