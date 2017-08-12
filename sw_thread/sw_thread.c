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
    int                 id;
    int                 prev_id;
    int                 next_id;
    pthread_t           th;
};

//int efd;
//int evfd;
int thread_efd[3];

void *ul_proc(void *thdata)
{
    eventfd_t  u;
    struct thdata       *priv = (struct thdata *)thdata;

    printf("[thread] %d : created\n",priv->id);

    while(1){
        recv_eventfd(thread_efd[priv->prev_id]);

        printf("[thread] %d sleep(2)\n",priv->id);
      //  sleep(2);

	u = priv->id;
        eventfd_write(thread_efd[priv->next_id], u);
    }
    /* done */
    return (void *) NULL;
}

void recv_eventfd(int efd){
    int 	ret;
    u_int64_t	value;
    fd_set	rfds;

    FD_ZERO(&rfds);  
    FD_SET(efd,&rfds);  
    ret = select(efd+1,&rfds,NULL,NULL,NULL);  
    printf("[recv_eventfd] select completed\n");

    if( FD_ISSET(efd,&rfds) ) {  
      eventfd_read(efd,&value);  
      printf("[recv_eventfd] eventfd_read value=%lld\n",value);  
    } 
}

int main(int argc, char ** argv ) {
    int      i,j,rtn;
    eventfd_t u=1;
    struct thdata    t_data[3];

    for (j=0;j<4;j++) thread_efd[j] = eventfd(0 , 0);

    t_data[0].id  = 1;
    t_data[0].prev_id = 0;
    t_data[0].next_id = 1;
    rtn = pthread_create(&t_data[0].th, NULL, ul_proc, (void*) (&t_data[0]));

    t_data[1].id  = 2;
    t_data[1].prev_id = 1;
    t_data[1].next_id = 2;
    rtn = pthread_create(&t_data[1].th, NULL, ul_proc, (void*) (&t_data[1]));

    t_data[2].id  = 3;
    t_data[2].prev_id = 2;
    t_data[2].next_id = 3;
    rtn = pthread_create(&t_data[2].th, NULL, ul_proc, (void*) (&t_data[2]));

    //sleep(3);

    printf("*** send eventfd to thread 0 ***\n");
    eventfd_write(thread_efd[0], u);

    recv_eventfd(thread_efd[3]);
    printf("*** just received eventfd from thread 2 ***\n");

    for (j=0;j<4;j++) close( thread_efd[j]);
}
