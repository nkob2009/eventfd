#include <sys/eventfd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct thdata {
    int		efd;
    int		prev_id;
    int		own_id;
    int		next_id;
    pthread_t	th;
};

int efd;
int evfd;
int thread_efd[5];

void *ul_proc(void *pThdata){
	int	i,j,k,ret,rtn;
	struct thdata	*priv = (struct thdata *) pThdata;
	printf("thread created (%d,%d,%d)\n",priv->prev_id,priv->own_id,priv->next_id);
	while (1){
		//printf("waiting for eventfd from %d \n",priv->prev_id);
		//recv_eventfd(priv->efd);	// waiting for event
		recv_eventfd(thread_efd[priv->own_id]);	// waiting for event
		printf("eventfd from %d to %d\n",priv->prev_id,priv->own_id);
		sleep(1);	// some processing
		eventfd_write(thread_efd[priv->next_id], priv->own_id);  // send event to next thread
	} 
	return (void*) NULL;
}

void recv_eventfd(int efd){
    int 	ret;
    u_int64_t	value;
    fd_set	rfds;

//    printf("waiting for fd .... %d\n",efd);
    FD_ZERO(&rfds);  
    FD_SET(efd,&rfds);  
    ret = select(efd+1,&rfds,NULL,NULL,NULL);  
//    printf("[recv_eventfd] select completed\n");

    if( FD_ISSET(efd,&rfds) ) {  
      eventfd_read(efd,&value);  
//      printf("[recv_eventfd] eventfd_read value=%lld\n",value);  
    } 
}

int main(int argc, char ** argv ) {
    int      i,j, ret, count, rtn;
    eventfd_t u=1;
    struct thdata    t_data[5];

    for (j=0;j<4;j++) thread_efd[j] = eventfd(0 , 0); 

    for (i=0;i<4;i++) printf("eventfd[%d]=%d\n",i,thread_efd[i]); 

    t_data[0].efd = thread_efd[0];
    t_data[0].own_id = 1;
    t_data[0].prev_id  = 0;
    t_data[0].next_id  = 2;
    rtn = pthread_create(&t_data[0].th, NULL, ul_proc, (void*) (&t_data[0]));

    t_data[1].efd = thread_efd[1];
    t_data[1].own_id = 2;
    t_data[1].prev_id  = 1;
    t_data[1].next_id  = 3;
    rtn = pthread_create(&t_data[1].th, NULL, ul_proc, (void*) (&t_data[1]));

    t_data[2].efd = thread_efd[2];
    t_data[2].own_id = 3;
    t_data[2].prev_id  = 2;
    t_data[2].next_id  = 1;
    rtn = pthread_create(&t_data[2].th, NULL, ul_proc, (void*) (&t_data[2]));

    sleep(1);

    printf("*** send eventfd to thread 0 ***\n");
    eventfd_write(thread_efd[1], u);

    recv_eventfd(thread_efd[4]);
    printf("*** recv eventfd to thread 0 ***\n");

    sleep(10);

    for (j=0;j<4;j++) close( thread_efd[j]);
    close( evfd );
}

