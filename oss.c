#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#define SHMKEY 1840831
#define MSGKEY 1840832
#define BUFF_SIZE sizeof(int)

pid_t child;
int shmid, msqid;

void alarmTimer(int sig);
void alarmInterrupt(int sig);

typedef struct msgbuf {
    long mtype;
    char mtext[1];
} critMsg;

int main(int argc, char* argv[]) {
    
    int opt;
    int n = 4;
    int s = 2;
    int m = 1;
    critMsg msg;
    msg.mtype = 1;

    while((opt = getopt(argc, argv, ":hn:s:m:")) != -1) {
        switch(opt) {

            case 'h':
                printf("Invalid input. Expected arguments:\n./oss -n x -s y -m z\nWhere 'n' is the max number of processes to fork (max 18, default 4)\n's' is the number of children that can exist at the same time (default 2)\nand 'm' is the amount to increase the clock multiplied by 1 million (default 1)\n");
                return 0;

            case 'n':
                n = atoi(optarg);
                if(n > 18) {
                    printf("'n' cannot excede 18 processes. Default value being used.");
                    n = 4;
                }
                break;

            case 's':
                s = atoi(optarg);
                break;

            case 'm':
                m = atoi(optarg);
                break;

            case '?':
                printf("Invalid input. Expected arguments:\n./oss -n x -s y -m z\nWhere 'n' is the max number of processes to fork (max 18, default 4)\n's' is the number of children that can exist at the same time (default 2)\nand 'm' is the amount to increase the clock multiplied by 1 million (default 1)\n");
                return 0;
        }
    }

    signal(SIGALRM, alarmTimer);
    alarm(30);

    signal(SIGINT, alarmInterrupt);


    if((shmid = shmget(SHMKEY, BUFF_SIZE, 0777 | IPC_CREAT)) == -1) {
        perror("Error: shmget:\n");
        exit(1);
    }

    char *shmem;

    if((shmem = shmat(shmid, 0, 0)) == (char *) -1) {
        perror("Error: shmat:\n");
        exit(1);
    }

    if((msqid = msgget(MSGKEY, 0777 | IPC_CREAT)) == -1) {
        perror("Error: msgget:\n");
        exit(1);
    }

    if(msgsnd(msqid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("Error: msgsend:\n");
        exit(1);
    }

    int *clockSec = (int *) shmem;
    *clockSec = 0;
    int *clockMilli = clockSec + 1;
    *clockMilli = 0;

    char argument[10];
    snprintf(argument, 10, "%d", m);
    int i;

    for(i = 0; i <= n - s; i++) {
        wait();
        child = fork();

        if (child == 0) {
            execlp("./worker", "./worker", argument, (char *) NULL);

            fprintf(stderr, "%s failed to exec. Terminating program.", argv[0]);
            exit(-1);
        }
    }

    wait();

    printf("\nSeconds: %d\n", *clockSec);
    printf("Milliseconds: %d\n", *clockMilli);

    if(shmdt(shmem) == -1) {
        perror("Error: shmdt:\n");
    }

    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: smhctl:\n");
    }

    if(msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("Error: msgctl:\n");
    }

    killpg(getpid(), SIGTERM);
    return 0;

}

void alarmTimer(int sig) {
    signal(sig, SIG_IGN);
    printf("Program has been running for 30 seconds. Ending all processes.\n");

    kill(child, SIGKILL);

    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: shmctl:\n");
    }

    exit(0);

}

void alarmInterrupt(int sig) {
    signal(sig, SIG_IGN);
    printf("'ctrl + c' pressed. Ending all processes.\n");

    kill(child, SIGKILL);

    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: shmctl:\n");
    }

    exit(0);

}
