#include<stdio.h>
#include<sys/time.h>
#include<unistd.h>
 
int main()
{
        struct  timeval    tv;
        struct  timezone   tz;
        gettimeofday(&tv,&tz);
        printf("tv_sec:%d\n",tv.tv_sec);
        printf("tv_usec:%d\n",tv.tv_usec);
        printf("tz_minuteswest:%d\n",tz.tz_minuteswest);
        printf("tz_dsttime:%d\n",tz.tz_dsttime);
}
