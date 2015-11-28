/*************************************************************

THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING

A TUTOR OR CODE WRITTEN BY OTHER STUDENTS-JING CUI

*************************************************************/


#include "shmstruct.h"
#include <stdio.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>

int shmid;
int messID;
uint8_t *addr;
pid_t *proc;

void handler(int);

int main(int argc, char *argv[])
{
    if(argc != 1)
    {
        printf("usage: manage\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        /*settle signal*/
        struct sigaction act;
        memset(&act,0,(size_t)sizeof(act));//add #include <string.h> will solve the warning!!!
        act.sa_handler = handler;
        sigaction(SIGINT, &act, NULL);
        sigaction(SIGHUP, &act, NULL);
        sigaction(SIGQUIT, &act, NULL);
        
        /*shared memery*/
        //int shmid;
        //unit8_t *addr;
        pid_t *perf;
        //pid_t *proc;
        struct shmem shm;
        shmid = shmget(KEY, sizeof(shm), IPC_CREAT|0660);
        if(shmid == -1)
        {
            perror("shmget\n");
            exit(EXIT_FAILURE);
        }
        if ((addr=((uint8_t *) shmat(shmid,NULL,0)))== (uint8_t *)-1) 
        {
            perror("shmat");
            shmctl(shmid,IPC_RMID,0);
            exit(EXIT_FAILURE);
		}
		memset(addr,0,(size_t)sizeof(struct shmem));///////////
        //addr = shmat(shmid,NULL,0);
        perf = (pid_t *) &addr[BIT_SIZE/8];
        proc = (pid_t *)&perf[PERF_NUM];
        
        /*message queue*/
        //int messID;
        struct  msg
        {
            long type;
            pid_t data;
		}; 
        struct msg my_msg;
        struct msg rec_msg;
        if ((messID=msgget(MessKey,IPC_CREAT |0660))== -1) 
        {
            perror("msgget error\n");
			shmdt(addr);
            shmctl(shmid,IPC_RMID,0);
            exit(1);
		}
        while(1)
        {
            msgrcv(messID,&rec_msg,sizeof(rec_msg.data),-4,0);
            if (rec_msg.type == 2)//perfectnumber--write to share mem
            {
				int j, flag=0;
				for(j=0;j<20;j++)//equal?
				{
					if(perf[j]==rec_msg.data)
					{
						flag = 1;
						break;
					}
				}
				if(flag == 0)
				{
					int i;					
					for(i=PERF_NUM-2;i>=0;i--)
					{
						if(perf[0]==0)
						{
							perf[0]=rec_msg.data;
							break;
						}
						if(perf[i]!=0)
						{
							if(perf[i]<rec_msg.data)
							{
								perf[i+1]=rec_msg.data;
							}
							else
							{
								perf[i+1]=perf[i];
								perf[i]=rec_msg.data;								
							}
						}
					}
				}
                
            }
            if (rec_msg.type == 4)//compute pid--send a entry
            {
                int i,j=0;
                for(i=0;i<PROC_NUM;i++)
                {
                    if(proc[4*i]==0)
                    {
                        j = 1;
                        my_msg.type = 5;
                        my_msg.data = i;
                        msgsnd(messID,&my_msg,sizeof(my_msg.data),0);
                        break;
                    }
                }
                if(j==0)
                {
                    my_msg.type = 5;
                    my_msg.data = -1;
                    msgsnd(messID,&my_msg,sizeof(my_msg.data),0);
                }
                //sleep(2);
            }
            if (rec_msg.type == 3)//send mypid
            {
                my_msg.type = 6;
                my_msg.data = getpid();
                msgsnd(messID,&my_msg,sizeof(my_msg.data),0);
                //sleep(2);
            }
        }
        
        
        
    }
    return 0;
}

void handler(signum)
{
    int i;
    for(i=0;i<PROC_NUM;i++)
    {
        if(proc[4*i]>0)
        {
            kill(proc[4*i], SIGINT);
		}
		
    }
    sleep(5);
	if (msgctl(messID, IPC_RMID, NULL) == -1) //should be global!!!change later
    {
        perror("msgctl\n");
		shmctl(shmid,IPC_RMID,0);
        exit(EXIT_FAILURE);
    }
    if (shmdt(addr) == -1) //should be global!!!change later
    {
        perror("shmdt\n");
		shmctl(shmid,IPC_RMID,0);
        exit(EXIT_FAILURE);
    }
	if (shmctl(shmid,IPC_RMID,0) == -1) //should be global!!!change later
    {
		perror("shmctl\n");
		exit(EXIT_FAILURE);
	}
    exit(1);
}