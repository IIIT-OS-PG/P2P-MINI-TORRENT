#include<string.h>
#include<stdio.h>
#include <fstream>
#include <iostream>
using namespace std;
 
int main () 
{
	fstream f;
	string s="pok";
	char a[]="g,t";
	s=a;
	f.open("movie.mp4",ios::out | ios :: app);
	for (int i = 0; i < 1077373315; ++i)
	{
		f<<'\0';
	}
	f.close();
		// f.open("logfile.txt",ios::in);
		// char fn[]="movie.mp4";
		// char fk[]="copy";
		// while(!f.eof())
		// {
		// 	string s;
		// 	getline(f,s);

		// 	char str[1024];
		// 	char *cports[1024];
		// 	int i,fl=0;
		// 	strcpy(str,s.data());
			
		// 	char *token = strtok(str, " "); 
  			
  // 			int c=0;
  //   // Keep printing tokens while one of the 
  //   // delimiters present in str[]. 
		//     while (token != NULL) 
		//     { 
		//         // printf("%s\n", token); 
		//         cports[c]=token;
		//         c++;
		//         // printf("%s\n",token);
		//         token = strtok(NULL, " "); 
		//     } 
		//     cports[c]=NULL;
		//     // cout<<c<<"j";
		//     cout<<"\nports having this file";
		//     for (int i = 0; i < c; ++i)
		//     {
		// 		if(*cports[i]==*fn || *cports[i]==*fk)
		// 			cout<<"j";
		// 		else
		// 		{
		// 			string sp=cports[i];
		// 			cout<<sp<<"\n";
		// 			long long k=stoi(sp);
		// 			cout<<k<<"";
		// 		}
 	// 	    }
 	// 	}
		
}