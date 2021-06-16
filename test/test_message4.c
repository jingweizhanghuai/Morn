#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
 
int main()
{
    mqd_t mqID = mq_open("/testmQueue", O_RDWR | O_CREAT, 0666, NULL);
 
    if (mqID < 0)
    {
        // if (errno == EEXIST)
        // {
        //     mq_unlink("/testmQueue");
        //     mqID = mq_open("/testmQueue", O_RDWR | O_CREAT, 0666, NULL);
        // }
        // else
        {
            printf("open message queue error.mqID=%d..\n",mqID);
            return -1;
        }
    }
 
    if (fork() == 0)
    {
        struct mq_attr mqAttr;mq_getattr(mqID, &mqAttr);
        char *buf = malloc(mqAttr.mq_msgsize*sizeof(char));
        for (int i = 1; i <= 5; ++i)
        {
            if (mq_receive(mqID, buf, mqAttr.mq_msgsize, NULL) < 0)
            {
                printf("receive message  failed.\n");
                continue;
            }
 
            printf("receive message %d: %s\n",i,buf);   
        }
        free(buf);
        exit(0);
    }
 
    char msg[] = "yuki";
    for (int i = 1; i <= 5; ++i)
    {
        if (mq_send(mqID, msg, sizeof(msg), i) < 0)
        {
            printf("send message %d failed.\n",i);
        }
        printf("send message %d success.\n",i);
 
        sleep(1);
    }
}