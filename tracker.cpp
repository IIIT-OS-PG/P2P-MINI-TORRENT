#include<bits/stdc++.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h> 
using namespace std;

string myport;
string myip;

void * trackerthread(void * sock)
{
	int ack=0;
	int sockfd= *((int*)sock);
	int file_size;
	char msg[1500];
	recv(sockfd, (char*)&msg, sizeof(msg), 0);
	send(sockfd, &ack, sizeof(ack), 0);

	ifstream myfile ("logfile");
	vector <string> ports;
	
	int portcount=0, size=-1;
	if (myfile.is_open())
  	{
		string strline; 
		while(getline(myfile, strline))
		{
			
			char temp[1024];
			char * str[1024];

			strcpy(temp,strline.data());
			int index=0;
			char *ptr= strtok(temp," ");
			while(ptr != NULL)
			{
				str[index]=ptr;
				ptr = strtok(NULL, " ");
				index++;
			}

			str[index]=NULL;
			if(strcmp(str[0],msg)==0)
			{
				printf("%s\n",str[1]);
				ports.push_back(str[1]);
				if(size==-1){
					size=atoi(str[2]);
				}
			}

		}

		for(int i =0;i<ports.size();i++)
		{
			portcount++;
		}

		send ( sockfd , &portcount, sizeof(portcount), 0);
		recv(sockfd, &ack, sizeof(ack), 0);

		for(int i =0;i< portcount; i++)
		{
			char buffer[255];
			bzero(buffer, 255);
			strcpy(buffer, ports[i].data());
			send ( sockfd , &buffer, strlen(buffer), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
		}
		send ( sockfd , &size, sizeof(size), 0);
		recv(sockfd, &ack, sizeof(ack), 0);
		myfile.close();
  	}
  	else
  	{
  		cout<<"tracker data opening error";
  	}

	close( sockfd);
	return NULL;
}

void * serverThread(void * arg)
{
	int socktracker = socket (AF_INET, SOCK_STREAM, 0);
 	struct sockaddr_in   addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(stoi( myport ));
	addr.sin_addr.s_addr=INADDR_ANY;
	int addrlen = sizeof(sockaddr);

	if(	bind (socktracker  , (struct sockaddr *)&addr , sizeof ( addr ) ) < 0) 
	{ 
	 printf("Error binding\n");
	 return 0;
	}

	listen (socktracker, 10);
	cout<<"\nListening";
	while(1)
	{
		int sockfd = accept ( socktracker , (struct sockaddr *)&addr , (socklen_t*)&addrlen);
		cout<<"\nConnected to tracker";
		int ack=0;
		char cmd[1500];
		memset(cmd,'\0',1500);

		recv(sockfd, (char*)&cmd, sizeof(cmd), 0);
		cout<<"command in tracker is \n"<<cmd<<endl;
		send(sockfd, &ack, sizeof(ack), 0);

		if(strcmp(cmd,"login")==0)
		{
			char username[50],password[50];
			memset(password,'\0',50);
			memset(username,'\0',50);
			int check=1;
			while(check)
			{
				recv(sockfd, &username, sizeof(username), 0);
				send(sockfd, &ack, sizeof(ack), 0);
				fstream f;
				string line;
				f.open("Authenticationinfo",ios::in);
				ack=0;
				while (getline (f,line))
			    {
			    	char un[50];
			    	memset(un,'\0',50);
			    	strcpy(un,line.data());
			    	if(strcmp(un,username)==0)
			    	{
	    				getline(f,line);
	    				char pw[50];
	    				memset(pw,'\0',50);
	    				cout<<line<<endl;
	    				cout<<"*";
	    				strcpy(pw,line.data());
	    				recv(sockfd, &password, sizeof(password),0);
	    				// send(sockfd, &ack, sizeof(ack), 0);
	    				if(strcmp(pw,password)==0)
	    				{
	    					ack=1;
	    					check=0;
	    					send(sockfd, &ack, sizeof(ack), 0);
	    					cout<<"\nk";
	    					break;
	    				}
	    				else
	    				{
	    					ack=0;
	    					send(sockfd, &ack, sizeof(ack), 0);
	    					cout<<"\nn";
	    					break;	
	    				}
			    	}
			    }
			    f.close();
			}
		}

		else if(strcmp(cmd,"download_file")==0)
		{

			pthread_t t;
			pthread_create(&t,NULL,trackerthread,(void*)&sockfd);
		}

		else if(strcmp(cmd,"show")==0)
		{
			  recv(sockfd, &ack, sizeof(ack), 0);
			  ifstream myfile ("logfile");
			  if (myfile.is_open())
			  {
			  	cout<<"u r here\n";
			  	string line;
			  	ack=0;
			    while ( getline (myfile,line) )
			    {
			    	send(sockfd, &ack, sizeof(ack), 0);
    				recv(sockfd, &ack, sizeof(ack), 0);
			     	char pw[50];
			     	memset(pw,'\0',50);
    				strcpy(pw,line.data());
    				send(sockfd, &pw, sizeof(pw), 0);
    				recv(sockfd, &ack, sizeof(ack), 0);
			    }
			    ack=1;
			    cout<<"final\n";
			    send(sockfd, &ack, sizeof(ack), 0);
    			recv(sockfd, &ack, sizeof(ack), 0);
    			cout<<"finally\n";
			    myfile.close();
			  }
		}

		else if(strcmp(cmd,"upload_file")==0)
		{
		    fstream myfile;
		    myfile.open ("logfile",ios::out | ios::app);
		  	char fname[1500];
		  	// memset(fname,'\0',1500);
		  	long long fsize;
		  	int fport;
			recv(sockfd, (char*)&fname, sizeof(fname), 0);
			// cout<<"command in tracker is \n"<<cmd<<endl;
			// cout<<"f";
			send(sockfd, &ack, sizeof(ack), 0);
			recv(sockfd, &fport, sizeof(fport), 0);
			// cout<<"g";
			send(sockfd, &ack, sizeof(ack), 0);
			recv(sockfd, &fsize, sizeof(fsize), 0);
			// cout<<"h";
			send(sockfd, &ack, sizeof(ack), 0);
			string filenm=fname;
			myfile<<filenm<<" "<<fport<<" "<<fsize<<endl;
		    myfile.close();
		   // cout<<"poora\n";
		}

		else if (strcmp(cmd,"create_user")==0)
		{
			  fstream myfile ("Authenticationinfo",ios::out | ios::app);
			  if (myfile.is_open())
			  {
			  	cout<<"open\n";
			  	char uname[50],pword[50];
			  	long long fsize;
			  	int fport;
			  	memset(pword,'\0',50);
				memset(uname,'\0',50);
				recv(sockfd, (char*)&uname, sizeof(uname), 0);
				send(sockfd, &ack, sizeof(ack), 0);
				recv(sockfd, (char*)&pword, sizeof(pword), 0);
				send(sockfd, &ack, sizeof(ack), 0);
				
				string usname=uname,psword=pword;
				myfile<<usname<<endl;
				myfile<<psword<<endl;
			    myfile.close();
			  }
		}
		
		else 
			continue;


	}

	close(socktracker);
	return NULL;


}
int main(int argc, char *argv[])
{
	
	if(stoi(argv[2])!=1 && stoi(argv[2])!=2)
	{
		printf("Tracker can be 1 or 2 only\n");
		return 0;
	}
	// if(argc!=3)
	// {
	// 	printf("pass proper arguments\n");
	// 	return 0;
	// }
	cout<<"\nStart Tracker \n";
	string iptracker1 , porttracker1,iptracker2,porttracker2;
	// cout<<"argv1 "<<argv[1]<<endl;
	ifstream myfile (argv[1]);	
	if (myfile.is_open())
  	{
		string str; 
		std::getline(myfile, str);
		iptracker1=str;
		cout<<"iptracker1 "<<iptracker1<<endl;
		str.clear();

		std::getline(myfile, str);
		porttracker1=str;
		cout<<"track1p "<<porttracker1<<endl;
		str.clear();

		std::getline(myfile, str);
		iptracker2=str;
		cout<<"iptracker2 "<<iptracker2<<endl;
		str.clear();

		std::getline(myfile, str);
		porttracker2=str;
		cout<<"track2p "<<porttracker2<<endl;
		str.clear();    
 	    myfile.close();
  	}
  	else
  	{
  		printf("Unable to open file\n"); 
  		return 0;
  	}	  
	if(stoi(argv[2])==1)
	{
		myport=porttracker1;
		myip=iptracker1;
		cout<<"\nTracker 1 is "<<stoi( myport )<<"\n";
	}
	if(stoi(argv[2])==2)
	{
		myport=porttracker2;
		myip=iptracker2;
		cout<<"\nTracker 2 is "<<stoi( myport )<<"\n";
	}
	pthread_t tid;
	pthread_create(&tid, NULL, serverThread, NULL);
	pthread_join(tid,NULL);

  	return 0;

} 
