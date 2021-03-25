#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define IP_addr "127.0.0.1"    /*IP地址*/
#define Host_port 50000        /*端口号*/
#define Listen_num 4           /*监听数*/

struct addr
{
	struct sockaddr_in caddr;    /*存放客户端地址信息的结构体*/
	int confd;                   /*存放通信套接字*/
};

void *fun(void *arg)
{
	struct addr *p = (struct addr *)arg;
	
	/*读取数据*/
	char buf[100];
	
	while(1)
	{
		bzero(buf, 100);
		read(p->confd, buf, 100);
		if(!strncmp(buf, "quit", 4))
		{
			break;
		}
		
		/*显示客户端发送的指令*/
		printf("[%s][%d]:%s\n", inet_ntoa(p->caddr.sin_addr), ntohs(p->caddr.sin_port), buf);
		
		if(!strncmp(buf, "LED_ON", 6))
		{
			printf("输入的指令为:LED_ON\n");
		}
	}
	close(p->confd);
	printf("[%s][%d]客户端退出\n", inet_ntoa(p->caddr.sin_addr), ntohs(p->caddr.sin_port));
	
	return NULL;
}

int main(void)
{
	/*建立待连接套接字*/
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);    /*AF_INET网际协议(IPV4协议) SOCK_STREAM流式套接字,TCP通信*/
	if(-1 == sockfd)
	{
		perror("获取待连接套接字失败");
		return -1;
	}
	
	/*定义IPv4地址结构体变量*/
	struct sockaddr_in saddr;
	
	/*初始化结构体数据*/
	saddr.sin_family      = AF_INET;               /*地址族使用AF_INET网际协议(IPV4协议)*/
	saddr.sin_port        = htons(Host_port);      /*将端口号转换为网络字节序*/
	saddr.sin_addr.s_addr = inet_addr(IP_addr);    /*将点分式IP地址转换为32位二进制数*/
	
	/*绑定服务器地址*/
	int bindfd = bind(sockfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
	if(-1 == bindfd)
	{
		perror("绑定地址失败");
		return -1;
	}
	
	/*设置监听*/
	int listenfd = listen(sockfd, Listen_num);
	if(-1 == listenfd)
	{
		perror("设置监听失败");
		return -1;
	}
	
	int confd;                                   /*定义一个存放通信套接字变量*/
	struct sockaddr_in caddr;                    /*存放客户端地址信息的结构体*/
	socklen_t clen = sizeof(struct sockaddr);    /*存放客户端结构体地址大小的变量*/
	struct addr addr1[100];
	int i = 0;
	
	while(1)
	{
		/*建立通信套接字*/
		confd = accept(sockfd, (struct sockaddr *)&caddr, &clen);
		if(-1 == confd)
		{
			perror("连接失败");
			return -1;
		}
		addr1[i].caddr = caddr;
		addr1[i].confd = confd;
		
		/*初始化线程属性，并将分离属性加入该变量*/
		pthread_attr_t attr;      /*线程属性变量*/
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		
		printf("[%s][%d]客户端连接成功！\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
		
		char buf[100];
		snprintf(buf, sizeof("system: your id is %d\n"), "system: your id is %d\n", confd);
		write(confd, buf, strlen(buf));
		
		/*用分离属性变量产生一条新线程*/
		pthread_t thread;    /*线程号变量*/
		pthread_create(&thread, &attr, fun, (void *)&addr1[i]);
		i++;
	}
	
	/*关闭通信接套接字*/
	close(confd);
	close(sockfd);
	
	return 0;
	
}