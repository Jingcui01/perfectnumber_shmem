/*************************************************************

THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING

A TUTOR OR CODE WRITTEN BY OTHER STUDENTS-JING CUI

*************************************************************/


#include "shmstruct.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>

uint8_t *addr;
void handler(int);

int main(int argc, char *argv[])
{
    if(argc > 2 || (argc ==2 && strcmp(argv[1],"-k")!=0))
    {
        printf("usage: report [option]-k\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        /*settle signal*/
        struct sigaction act;
        memset(&act,0,sizeof(act));
        act.sa_handler = handler;
        sigaction(SIGINT, &act, NULL);
        sigaction(SIGHUP, &act, NULL);
        sigaction(SIGQUIT, &act, NULL);
        
        /*shared memery*/
        int shmid;
        //unit8_t *addr;
        int *perf;
        pid_t *proc;
        
        shmid = shmget(KEY, 0,0);
        if(shmid == -1)
        {
            perror("Shared Memory Not Exist. Run Manage First.\n");
            exit(EXIT_FAILURE);
        }
        addr = (uint8_t *)shmat(shmid, NULL, 0);
        perf = (pid_t *) &addr[BIT_SIZE/8];
        proc = (pid_t *)&perf[PERF_NUM];
        
        /*message queue*/
        int messID;       
        struct  msg
        {
            long type;
            pid_t data;
		}; 
        struct msg my_msg;
        struct msg rec_msg;
        my_msg.type = 3;//6 represets mypid
        my_msg.data = getpid();
        //open MQ
        if ((messID=msgget(MessKey,IPC_CREAT |0660))== -1) 
        {
            perror("msgget error\n");
			shmdt(addr);
            exit(1);
		}
        //send request
        msgsnd(messID,&my_msg,sizeof(my_msg.data),0);
        //sleep(2);
        msgrcv(messID,&rec_msg,sizeof(rec_msg.data),6,0);//6 for manage pid
        
        if(argc == 2)
        {
            kill(rec_msg.data, SIGINT);
        }
        else
        {
            pid_t total;
            int i;
            printf("Perfect Number: ");
            for(i=0; i<PERF_NUM; i++)
            {
                if(perf[i]!=0)
                    printf("%8d ", perf[i]);
            }
            printf("\n\n");
            total = 0;
            for(i=0; i<PROC_NUM; i++)
            {
                if(proc[0+i*4]>0)
                {
                    printf("pid: %8d; Found: %8d; Tested: %8d; Skiped: %8d\n",
                    proc[0+i*4],proc[1+i*4],proc[2+i*4],proc[3+i*4]);
                    total = total + proc[2+i*4];
                }
            }
            printf("\nTotal Tested: %8d\n", total);
        }
        shmdt(addr);
    }
    return 0;
}

void handler(signum)
{
    shmdt(addr);
    exit(1);
}