#include <bits/stdc++.h>
#include <unistd.h> 
#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h> 

#define BUFF_SIZE 2048

using namespace std;
int fc=0;
int servport;

struct arg_struct
{
    int sockid;
    char *command;
    char * file;
};

struct ChunkDetails 
{
    int fd;
    char *name;
    int start;
    int end;
    int size;
    int port;
};

void ActualDownloadOfChunk(int sockfd,char* name, int start,int end,int size,int port)
{
		int ack=0;
		cout<<"name is "<<name<<endl;
		cout<<"start"<<start<<endl;	
		cout<<"end "<<end<<endl;	
		cout<<"size "<<size<<endl;	
		cout<<"port "<<port<<endl;	

		char msg1[1500];
		strcpy(msg1,name);
		cout<<"msg is "<<msg1<<"sending msg"<<endl;

		send(sockfd, (char*)&msg1, strlen(msg1), 0);
		recv(sockfd, &ack, sizeof(ack), 0);

		send(sockfd, &start, sizeof(start),0);
		recv(sockfd, &ack, sizeof(ack), 0);

		send(sockfd, &end, sizeof(end), 0);
		recv(sockfd, &ack, sizeof(ack), 0);

		FILE *fp = fopen ( name  , "r+b");
		fseek ( fp , 0 , start+1);
		char Buffer [BUFF_SIZE] ; 	
		int n;	
		while ( ( n = recv( sockfd , Buffer ,   BUFF_SIZE, 0) ) > 0  && end > 0)
		{
			fwrite (Buffer , sizeof (char), n, fp);
			memset ( Buffer , '\0', BUFF_SIZE);
			send(sockfd, &ack, sizeof(ack), 0);
			end = end - n;
		}

		fclose ( fp );
		close( sockfd);

}

void * DownloadChunksofFile(void * arguments)
{
	struct ChunkDetails *chunkargs = (struct ChunkDetails *)arguments;
    int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	struct sockaddr_in  serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(chunkargs->port);
	serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed in client with port %d \n",chunkargs->port); 
        return NULL; 
    }

    ActualDownloadOfChunk(sockfd, chunkargs->name, chunkargs->start,chunkargs->end, chunkargs->size,chunkargs->port);
	return NULL;
}

void* SendFileChunks(void * p_sockfd)
{
	int ack=0;
	int sockfd= *((int*)p_sockfd);
	int start,end;
	char msg[1500];
	memset(msg,'\0',1500);

	recv(sockfd, (char*)&msg, sizeof(msg), 0);
	send(sockfd, &ack, sizeof(ack), 0);

	recv(sockfd, &start, sizeof(start), 0);
	send(sockfd, &ack, sizeof(ack), 0);
	
	recv(sockfd, &end, sizeof(end), 0);
	send(sockfd, &ack, sizeof(ack), 0);

	FILE *fp = fopen ( msg  , "rb" );
	if (fp == NULL) {
	    cout<<"Error in opening file\n";
	}
	fseek ( fp , 0 , start+1);
	char Buffer [ BUFF_SIZE] ; 
	int n;
	while ( ( n = fread( Buffer , sizeof(char) , BUFF_SIZE , fp ) ) > 0  && end > 0 )
	{
		
		send (sockfd , Buffer, n, 0 );
	 	memset ( Buffer , '\0', BUFF_SIZE);
	 	recv(sockfd, &ack, sizeof(ack), 0);
		end = end - n ;
	}
	cout<<"Part sent\n";
	fclose ( fp );
	close( sockfd);
	return NULL;
}
// void* copyfileserver(void * p_sockfd)
// {
// 	int sockfd= *((int*)p_sockfd);
// 	int file_size;
// 	char msg[1500];
// 	recv(sockfd, (char*)&msg, sizeof(msg), 0);
// 	recv(sockfd, &file_size, sizeof(file_size), 0);

// 	FILE *fp = fopen ( msg  , "wb" );
// 	char Buffer [ BUFF_SIZE] ; 
// 	int n;
// 	while ( ( n = recv( sockfd , Buffer ,   BUFF_SIZE, 0) ) > 0  &&  file_size > 0)
// 	{	
// 		fwrite (Buffer , sizeof (char), n, fp);
// 		memset ( Buffer , '\0', BUFF_SIZE);
// 		file_size = file_size - n;
// 	}

// 	fclose ( fp );
// 	close( sockfd);
// 	return NULL;

// }

void* serverthread(void* vargp)
{
	// cout<<"enter server port\n";
	// cin>>servport;

	int server_fd = socket (AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in   addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(servport);
	addr.sin_addr.s_addr=INADDR_ANY;
	int addrlen = sizeof(sockaddr);

	bind (server_fd  , (struct sockaddr *)&addr , sizeof ( addr ) );
	
	listen (server_fd, 10);
	cout<<"\nListening\n";
	while(1){

		int sockfd = accept ( server_fd , (struct sockaddr *)&addr , (socklen_t*)&addrlen);
		cout<<"server port no - "<<addr.sin_port<<endl;
		cout<<"Connection Established in server\n";
		
		pthread_t t;
		pthread_create(&t,NULL,SendFileChunks ,(void*)&sockfd);
	}
	close( server_fd);
	return NULL;

}

void* Download(void *arguments)
{
	int ack=0;
	struct arg_struct *args = (struct arg_struct *)arguments;
    int sockfd=args->sockid;
    char *fn=(char*)malloc(100*sizeof(char));
    fn=args->file;

    char *command=(char*)malloc(100*sizeof(char));
    command=args->command;
	
	char msg[1500];
	strcpy(msg,fn);
	send(sockfd, (char*)&msg, strlen(msg), 0);
	recv(sockfd, &ack, sizeof(ack), 0);
	int peerscount=0;

	recv(sockfd, &peerscount, sizeof(peerscount), 0);
	send(sockfd, &ack, sizeof(ack), 0);

	cout<<"file exist in "<<peerscount<<" ports. "<<endl;
	vector <int> portlist;

	for(int i =0; i<peerscount ; i++)
	{
		char buffer[255];
		int ack=1;
		bzero(buffer, 255);
		recv(sockfd, &buffer, sizeof(buffer), 0);
		cout<<"ports are :"<<buffer<<endl;
		portlist.push_back(atoi(buffer));
		send ( sockfd , &ack, sizeof(ack), 0);
	}

	int sizef=-1;
	recv(sockfd, &sizef, sizeof(sizef), 0);
	cout<<"received file size as "<<sizef<<endl;
	send(sockfd, &ack, sizeof(ack), 0);

	fstream f;
	f.open(fn ,ios::out | ios :: app | ios::in);
	for (int i = 0; i < sizef; ++i)
	{
		f<<'\0';
	}
	f.close();

	FILE *fp = fopen ( fn  , "wb" );
	char Buffer [ BUFF_SIZE] ; 
	char nl[1];
	nl[0]='\0';
	size_t n=1;
	int filesz=sizef;
	while (sizef > 0)
	{
		fwrite (nl , sizeof (char),n , fp);
		sizef = sizef - n;
	}

	int partsize=(filesz/peerscount);
	pthread_t copythread[peerscount];
	int threadno=0;
	if(filesz%peerscount==0)
	{
		long long start=0,end=(filesz/peerscount);
		while(end != (filesz))
		{
		    struct ChunkDetails *chunkargs=(struct ChunkDetails*)malloc(sizeof(struct ChunkDetails));
			chunkargs->fd=sockfd;
			chunkargs->name=fn;
			chunkargs->start=start;
			chunkargs->end=end;
			chunkargs->size=filesz;
			chunkargs->port=portlist[threadno%peerscount];
			pthread_create(&copythread[threadno],NULL,DownloadChunksofFile,(void *)chunkargs);
			threadno++;
			start=end+1;
			if(start+partsize-1<=filesz)
				end=start+partsize-1;
			else
				end=filesz;
		}
		struct ChunkDetails *chunkargs=(struct ChunkDetails*)malloc(sizeof(struct ChunkDetails));
		chunkargs->fd=sockfd;
		chunkargs->name=fn;
		chunkargs->start=start;
		chunkargs->end=end;
		chunkargs->port=portlist[threadno%peerscount];
		chunkargs->size=filesz;

		pthread_create(&copythread[threadno],NULL,DownloadChunksofFile,(void *)chunkargs);
		threadno++;
	}
	else
	{
		long long start=0,end=(filesz/peerscount)-1;
		while(end< (filesz))
		{
		
			struct ChunkDetails *chunkargs=(struct ChunkDetails*)malloc(sizeof(struct ChunkDetails));
			chunkargs->fd=sockfd;
			chunkargs->name=fn;
			chunkargs->start=start;
			chunkargs->end=end;
			chunkargs->port=portlist[threadno%peerscount];
			chunkargs->size=filesz;

			pthread_create(&copythread[threadno],NULL,DownloadChunksofFile,(void *)chunkargs);
			threadno++;
			start=end+1;
			if(start+partsize-1<=filesz)
				end=start+partsize-1;
			else
				end=filesz;
		}

		struct ChunkDetails *chunkargs=(struct ChunkDetails*)malloc(sizeof(struct ChunkDetails));
		chunkargs->fd=sockfd;
		chunkargs->name=fn;
		chunkargs->start=start;
		chunkargs->end=end;
		chunkargs->port=portlist[threadno%peerscount];
		chunkargs->size=filesz-1;

		pthread_create(&copythread[threadno],NULL,DownloadChunksofFile,(void *)chunkargs);
		threadno++;
	}

	fclose ( fp );
	close( sockfd);
	return NULL;

}
void updatetrackerserver(char command[], char filename[],int port)
{

	int ack=0;	
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	struct sockaddr_in  serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( port );

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed in client \n"); 
        // return NULL; 
    }
	char cmd[1500];
	memset(cmd,'\0',1500);
	strcpy(cmd,command);
	send(sockfd, (char*)&cmd, strlen(cmd), 0);
	recv(sockfd, &ack, sizeof(ack), 0);
  	long long fsize;
  	int fport=servport;
  	string flname;
  	flname=filename;
 //  	cout<<"enter filname\n";
	// cin>>flname;
	strcpy(cmd,flname.data());
	cout<<cmd<<" k\n";
	FILE *fl = fopen ( cmd , "rb" );
	if (fl == NULL)
	{
	    cout<<"Error in opening file C\n";
	}
	fseek ( fl , 0 , SEEK_END);
	fsize = ftell ( fl );
	rewind ( fl );
	fclose(fl);

	send(sockfd, (char*)&cmd, strlen(cmd), 0);
	recv(sockfd, &ack, sizeof(ack), 0);
	send(sockfd, &fport, sizeof(fport), 0);
	recv(sockfd, &ack, sizeof(ack), 0);
	send(sockfd, &fsize, sizeof(fsize), 0);
	recv(sockfd, &ack, sizeof(ack), 0);
}

void * clientthread(void* vargp )
{
	int log=0;
	sleep(5);
	while(1)
	{
		int port,ack=0;
		char * fn=(char*)malloc(100*sizeof(char));
		char * command=(char*)malloc(100*sizeof(char));

		cout<<"enter tracker port to connect\n";
		cin>>port;
		
		int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
		struct sockaddr_in  serv_addr;
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons( port );

		if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	    { 
	        printf("\nConnection Failed in client \n"); 
	        return NULL; 
	    }

	    cout<<"\nEnter command download_file/upload_file/show/exit/login/logout/create_user\n";
		cin>>command;
		char *token = strtok(command, " ");
		int dl=0;
		if(strcmp(command,"download_file")==0 && log==1)
	    {
	    	char cmd[1500];
	    	memset(cmd,'\0',1500);
			strcpy(cmd,command);

			send(sockfd, (char*)&cmd, strlen(cmd), 0);
			recv(sockfd, &ack, sizeof(ack), 0);

	    	cout<<"enter filname\n";
    		cin>>fn;

    	    struct arg_struct *args=(struct arg_struct*)malloc(sizeof(struct arg_struct));
    		args->sockid=sockfd;
    		args->file=fn;
    		args->command=command;
    		dl=1;
    		// update(command,fn,port);
    		pthread_t Downloadthread;
    		pthread_create(&Downloadthread,NULL,Download,(void *)args);
    		updatetrackerserver(command,fn,port);
	    }

	    else if(strcmp(command,"show")==0 && log==1)
	    {
	    	char cmd[1500];
	    	memset(cmd,'\0',1500);
			strcpy(cmd,command);
			send(sockfd, (char*)&cmd, strlen(cmd), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
			cout<<"commandsent\n";
			send(sockfd, &ack, sizeof(ack), 0);
			ack=0;
			while (ack==0)
		    {
		    	recv(sockfd, &ack, sizeof(ack), 0);
				send(sockfd, &ack, sizeof(ack), 0);
				if(ack==1)
				{
					// cout<<"\nbreak";
					break;
				}
		     	char pw[50];
				recv(sockfd, &pw, sizeof(pw), 0);
				send(sockfd, &ack, sizeof(ack), 0);
				cout<<"\n"<<pw;
		    }
	    }

	    else if(strcmp(command,"upload_file")==0 && log==1)
	    {
	    	char cmd[1500];
	    	memset(cmd,'\0',1500);
			strcpy(cmd,command);
			send(sockfd, (char*)&cmd, strlen(cmd), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
		  	long long fsize;
		  	int fport=servport;
		  	memset(cmd,'\0',1500);
		  	string flname;
		  	cout<<"enter filname\n";
    		cin>>flname;
    		strcpy(cmd,flname.data());
    		cout<<cmd<<" k\n";
    		FILE *fl = fopen ( cmd , "rb" );
			if (fl == NULL)
			{
			    cout<<"Error in opening file C\n";
			}
			fseek ( fl , 0 , SEEK_END);
			fsize = ftell ( fl );
			rewind ( fl );
			fclose(fl);

			send(sockfd, (char*)&cmd, strlen(cmd), 0);
			recv(sockfd, &ack, sizeof(ack), 0);

			send(sockfd, &fport, sizeof(fport), 0);
			recv(sockfd, &ack, sizeof(ack), 0);

			send(sockfd, &fsize, sizeof(fsize), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
	    }

	    else if(strcmp(command,"create_user")==0)
	    {
	    	char cmd[1500];
	    	memset(cmd,'\0',1500);
			strcpy(cmd,command);
			send(sockfd, (char*)&cmd, strlen(cmd), 0);
			recv(sockfd, &ack, sizeof(ack), 0);

	    	char uname[50],pword[50];
	    	cout<<"\nEnter username ";
	    	cin>>uname;
	    	cout<<"\nEnter password ";
	    	cin>>pword;
			send(sockfd, (char*)&uname, sizeof(uname), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
			send(sockfd, (char*)&pword, sizeof(pword), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
			log=1;
	    }

	    else if(strcmp(command,"login")==0)
	    {
	    	char cmd[1500];
	    	memset(cmd,'\0',1500);
			strcpy(cmd,command);
			send(sockfd, (char*)&cmd, strlen(cmd), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
	    	char username[50],password[50];
	    	memset(password,'\0',50);
	    	memset(username,'\0',50);
			int check=1;
			ack=1;

			while(ack)
			{
				cout<<"\nEnter login credentials";
				cout<<"\nEnter username\n";
				cin>>username;
				cout<<"\nEnter password\n";
				cin>>password;

				send(sockfd, &username, sizeof(username), 0);
				recv(sockfd, &ack, sizeof(ack), 0);

   				send(sockfd, &password, sizeof(password),0);
				recv(sockfd, &ack, sizeof(ack), 0);

				if(ack)
				{
					cout<<"\nSuccessfully logged in\n";
					break;
				}
				else
				{
					cout<<"\nWrong credentials, try again / If not registered enter create_user command to proceed \n";
					ack=1;
				}
			}
			log=1;
	    }

	    else if(strcmp(command,"logout")==0 && log==1)
	    {
	    	log=0;
	    }
	    else if(strcmp(command,"exit")==0 )
    	{
    		exit(0);
    	}
	}
	return NULL;
}
int main(int argc, char *argv[])
{
	cout<<"\nstart\n";
	servport=*argv[1];
	pthread_t cid,sid;
	pthread_create(&sid,NULL,serverthread,NULL);
	pthread_create(&cid,NULL,clientthread,NULL);

	pthread_join(cid,NULL);

	return 0;
}