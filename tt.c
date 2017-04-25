#include<time.h>
#include<stdio.h>

int main(int argc,char *argv[]){
	struct tm *tt;
	time_t now;
	time(&now);
	tt = localtime(&now);


	printf("%d %d %d %d %d %d \n",tt->tm_year+1900,tt->tm_mon,tt->tm_mday,tt->tm_hour,tt->tm_min,tt->tm_sec);
	
}
