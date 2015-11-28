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

uint8_t *addr;
pid_t *procEntr;
void handler(int);
void markCheck(pid_t);
int check(pid_t);

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("usage: ./compute startNumber\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        /*settle signal*/
        struct sigaction act;
        memset(&act,0,(size_t)sizeof(struct sigaction));//??
        act.sa_handler = handler;
        sigaction(SIGINT, &act, NULL);
        sigaction(SIGHUP, &act, NULL);
        sigaction(SIGQUIT, &act, NULL);
        
        /*shared memery*/
        int shmid;
//        uint8_t *addr;
//        pid_t *procEntr;
        shmid = shmget(KEY,0,0);
        if(shmid==-1)
        {
            perror("Shared Memory Not Exist. Run Manage First.\n");
            exit(EXIT_FAILURE);
        }
		if ((addr=((uint8_t *) shmat(shmid,NULL,0)))== (uint8_t *)-1)
        {
            perror("shmat");
            exit(EXIT_FAILURE);
		}
            
        
        /*request entry use message queue*/
        
        int messID;
        int entry;
        struct  msg
        {
            long type;
            pid_t data;
		};
        struct msg my_msg;
        struct msg rec_msg;
        
        struct process comp;        
        comp.pid = getpid();
        comp.find = 0;
        comp.test = 0;
        comp.skip = 0;
        
        my_msg.type = 4;//4 represets pid
        my_msg.data = comp.pid;
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
        msgrcv(messID,&rec_msg,sizeof(rec_msg.data),5,0);//5 for process entrance
        if(rec_msg.data < 0) 
        {
            printf("more than 20 computing\n");
			shmdt(addr);
            exit(EXIT_FAILURE);
        }
        entry = rec_msg.data;
        pid_t offset = BIT_SIZE/8 + PERF_NUM*sizeof(pid_t) + entry*sizeof(struct process);
        procEntr = (pid_t *) &addr[offset];
        pid_t startN;        
        startN = atoll(argv[1]);
        pid_t i, n, sum;
        n = startN;
        procEntr[0] = comp.pid;
        while(n<=BIT_SIZE && n>=1)
        {
            //test checked or not
            if(check(n)==0)
            {
                sum = 1;
                for(i=2;i<n;i++)
                    if(!(n%i)) sum += i;
                if(sum==n && n!=1)
                {
                    my_msg.type = 2;//2 for perfect number
                    my_msg.data = n;
                    msgsnd(messID,&my_msg,sizeof(my_msg.data),0);
                    comp.find++;
                }
                comp.test++;           
                markCheck(n);//mark this number
                //n++;
            }
            else comp.skip++;    
			n++;
            procEntr[1] = comp.find;
            procEntr[2] = comp.test;
            procEntr[3] = comp.skip;
        }
		procEntr[0]=0;
		procEntr[1]=0;
		procEntr[2]=0;
		procEntr[3]=0;
		shmdt(addr);
    }
    
    return 0;
}


int check(pid_t num)
{
    int result;
	pid_t m;
    uint8_t n;
	num--;
    m = num/8;
    n = num%8;
    result = (addr[m]>>n & 1);
    return result;    
}
void markCheck(pid_t num)
{   
    pid_t m;
    uint8_t n;
	num--;
    m = num/8;
    n = num%8;
    addr[m] |= (1 << n);
}
void handler(signum)
{
    //clean entry
    procEntr[0]=0;//all globale!!!!!!
    procEntr[1]=0;
    procEntr[2]=0;
    procEntr[3]=0;
    shmdt(addr);//maybe globale!!!!!
    exit(1);
}