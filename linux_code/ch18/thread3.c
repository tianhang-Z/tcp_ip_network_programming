#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *thread_summation(void *arg);
int sum=0;

int main(int argc,char *argv[]){

    pthread_t t_id1,t_id2;
    int range1[]={1,5};
    int range2[]={6,10};

    pthread_create(&t_id1,NULL,thread_summation,(void*)&range1);
    pthread_create(&t_id2,NULL,thread_summation,(void*)&range2);

    pthread_join(t_id1,NULL);
    pthread_join(t_id2,NULL);

    printf("sum is %d \n",sum);
    return 0;
}

void *thread_summation(void* arg){

    int st=((int* )arg)[0];
    int ed=((int* )arg)[1];


    for(int i=st;i<=ed;i++){
        sum+=i;
    }
    return NULL;
}

