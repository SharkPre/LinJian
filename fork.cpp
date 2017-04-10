#include <stdio.h>
#include <unistd.h>

int main()
{
	int pid[10];
	
	for ( int i = 0; i < 5; i ++ )
	{
		if ( (pid[i] = fork() ) < 0 )
		{
			printf("fork error.\n");
		}
		else if( pid[i] == 0 )
		{
			printf("child %d\n",i);
			while(1);  // 子进程进行逻辑处理 一直循环 
		}
	}
	
	printf("monitor.\n");	// 主进程进入监听状态 
}