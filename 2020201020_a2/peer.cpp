

/****************************************************************************************************************************************
 *                                            OS Assignment - 2  ( P2P FILE SHARING APPLICATION )                                        *                                                  
 *                                                                                                                                       *
 *                                                                                                                                       *
 *                                                                                                                                       *
 *                      Name :  Abhisek  Mohapatra                                       Roll Number : 2020201020                        *                                                           
 *                                                                                                                                       *
 *                                                                                                                                       *
 *                                            Last Update :  Nov 8 5:24 pm                                                               *
 *                                                                                                                                       *                                                                                                                                                                                                                                   
 * ****************************************************************************************************************************************/

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <pthread.h>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <fstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <iterator>
#include <set>
#include <utility>
#include <openssl/sha.h>
#include <fcntl.h>

using namespace std;

/*

   argv[1]=client Ip
   argv[2]=client Port
   argv[3]=server IP
   argv[4]=server Port   // client IP and server IP will be same for all the cases



*/

string ipPeerInfo; // Used to communicate with other peers
string portPeerInfo;

string current_peer_user_id = ""; // Stores the current user id after 'S' acknowledgement from  server

unordered_map<string, string> groupAdmin_peer;        // groupId,userId
unordered_map<string, set<string>> groupMembers_peer; // groupId,Members of the group

vector<string> fileUploaderDetails; // To fetch details of uploader from the download_file command

int tracker_port = 8880;

int serv_port_no;
string self_serv_portNo = ""; // Port No of current peer
string self_cli_Id = "";      // Server Port No of current peer

vector<string> checkVector; // For Testing of download

unsigned long long int getFileSize(string filePath)
{

  struct stat sb;
  int f_size = 0;
  if (stat(filePath.c_str(), &sb) != 0)
  {
    return 0;
  }

  f_size = sb.st_size;
  return f_size;
}

string getHash(string filePath)
{
  char *input_buffer;
  int fd;

  unsigned long long int f_size = getFileSize(filePath);

  unsigned char *output_buffer = new unsigned char[20];
  fd = open(filePath.c_str(), O_RDONLY);

  input_buffer = (char *)mmap(0, f_size, PROT_READ, MAP_SHARED, fd, 0);

  SHA1((unsigned char *)input_buffer, f_size, output_buffer);
  munmap(input_buffer, f_size);
  close(fd);

  string finalRes = string(reinterpret_cast<char *>(output_buffer));
  free(output_buffer);

  return finalRes;
}

/**************************  CAll PEER Function *******************************************/

void *callPeerFun(void *n)
{

  int cli_peerfd = 0;
  int port = stoi(checkVector[1]);
  //  cout<<"Port of Peer client is: "<<port<<endl;

  struct sockaddr_in serv_addr;
  char msg_buffer[1024];

  cli_peerfd = socket(AF_INET, SOCK_STREAM, 0);
  if (cli_peerfd < 0)
  {
    perror("Error in creating server socket..");
    exit(-1);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  // cout<<"In here in call Fun"<<endl;

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) // tells the server to connect with this IP Address. Converts string IP to binary IP
  {
    perror("Server IP Address could not be validated/linked...");
    exit(-1);
  }

  if (connect(cli_peerfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("Failed to connect with server...");
    exit(-1);
  }

  cout << "Connected to Peer server.." << endl;

  //   while(1)
  //   {
  string temp_msg = "Hi Peer 1";

  memset(msg_buffer, 0, sizeof(msg_buffer));
  //   fgets(msg_buffer,1024,stdin);     // stdin not required
  strcpy(msg_buffer, temp_msg.c_str());

  int num = write(cli_peerfd, msg_buffer, strlen(msg_buffer));

  // cout<<"Message topeer client: "<<msg_buffer<<endl;  // Nov 5
  cout << "Message sent to peer client sent " << endl;

  if (num < 0)
  {
    perror("Error in writing..");
    exit(-1);
  }

  memset(msg_buffer, 0, sizeof(msg_buffer));

  num = read(cli_peerfd, msg_buffer, 1024);

  //  cout<<"Msg read from Peer server:: "<<msg_buffer<<endl;   // Nov 5
  cout << "Msg read from Peer server:: " << endl;

  if (num < 0)
  {
    perror("Error in reading..");
    exit(-1);
  }
  cout << "Peer Server: " << msg_buffer;

  cout << "\nMsg Received from Peer server.." << endl;

  //  memset(msg_buffer,0,sizeof(msg_buffer));

  /*  int i=strncmp("Bye",msg_buffer,3);
         
         if(i==0)
         break;   */

  //  } // Commented Nov 5

  close(cli_peerfd);

  return NULL;
}

/*void downloadFn()   // Function where client thread is created
{
  // int cli_peerfd=0;

    pthread_t cli_thread;
    int n=0;


      if( pthread_create( &cli_thread ,NULL ,callPeerFun ,(void*) &n) < 0)   // Client Thread
        {
            perror("Error in creating thread...");
            exit(-1);
        }




   /*  int port=stoi(checkVector[1]);

      struct sockaddr_in serv_addr1;
      char msg_buffer[1024];


   cli_peerfd=socket(AF_INET,SOCK_STREAM,0);
   if(cli_peerfd<0)
   {
       perror("Error in creating server socket..");
       exit(-1);
   }

   serv_addr.sin_family=AF_INET;
   serv_addr.sin_port=htons(port);

   if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)   // tells the server to connect with this IP Address. Converts string IP to binary IP
    { 
        perror("Server IP Address could not be validated/linked...");
        exit(-1); 
    } 
   
    if (connect(cli_peerfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    { 
       perror("Failed to connect with server...");
       exit(-1);
    } 
  



    while(1)
    {

        memset(msg_buffer,0,sizeof(msg_buffer)); 
        fgets(msg_buffer,1024,stdin);        

       int n=write(cli_peerfd,msg_buffer,strlen(msg_buffer));

         if(n<0)
         {
           perror("Error in writing..");
           exit(-1);
         }

         memset(msg_buffer,0,sizeof(msg_buffer));


         n = read( cli_sockfd , msg_buffer, 1024); 

         if(n<0)
         {
           perror("Error in reading..");
           exit(-1);
         }
        cout<<"Peer Server: "<<msg_buffer;


        int i=strncmp("Bye",msg_buffer,3);
         
         if(i==0)
         break;


    }


    close(cli_peerfd);  

} */

void *request_connection(void *sockfd)
{
  // int port_no=*(int *) port;

  int new_serv_sockfd = *(int *)sockfd;

  char msg_buffer[1024];

  //  while(1)  // Commented NOv 5
  //  {

  memset(msg_buffer, 0, sizeof(msg_buffer));
  int n = read(new_serv_sockfd, msg_buffer, 1024);

  if (n < 0)
  {
    perror("Error in reading..");
    exit(-1);
  }
  cout << "Client Peer: " << msg_buffer;

  memset(msg_buffer, 0, sizeof(msg_buffer));

  //  fgets(msg_buffer,1024,stdin);  No stdin reqd. // Nov 5
  string temp = "Helooo . Good to hear from ..";

  strcpy(msg_buffer, temp.c_str());

  n = write(new_serv_sockfd, msg_buffer, strlen(msg_buffer));

  //    cout<<"\n Message sent from Peer server: "<<msg_buffer<<endl;  // Nov 5
  cout << "\n Message sent from Peer server.. " << endl;

  if (n < 0)
  {
    perror("Error in writing..");
    exit(-1);
  }

  // memset(msg_buffer,0,sizeof(msg_buffer));

  /*  int i=strncmp("Bye",msg_buffer,3);

         if(i==0)
         break;  */

  //  }
  return NULL;
}

void *serverHandler(void *port) // For client acting as server ...  // Port No is passed as argument here
{

  // int port_no=*(int *)port;

  int port_no = serv_port_no;
  // cout<<"I m listening"<<endl;

  //  cout<<"My port number binded is: "<<port_no<<endl;

  int serv_sockfd;
  int new_serv_sockfd;
  struct sockaddr_in serv_addr;
  int opt = 1;

  serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (serv_sockfd < 0)
  {
    perror("Error in creating server socket..");
    exit(-1);
  }

  if (setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
  {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY; // Makes it compatible against all available interfaces (0.0.0.0)
  serv_addr.sin_port = htons(port_no);    // Takes the port number in integer format

  if (bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("Binding with Port Number failed....");
    exit(-1);
  }

  if (listen(serv_sockfd, 10) < 0) // 5 is the queue size assigned
  {
    perror("Failed to listen.... ");
    exit(-1);
  }

  socklen_t addr_len = sizeof(serv_addr);

  pthread_t t_id;

  while (1)
  {

    if ((new_serv_sockfd = accept(serv_sockfd, (struct sockaddr *)&serv_addr, (socklen_t *)&addr_len)) < 0)
    {
      perror("Failed to accept...");
      exit(-1);
    }

    if (pthread_create(&t_id, NULL, request_connection, (void *)&new_serv_sockfd) < 0)
    {
      perror("Error in creating Peer client thread...");
      exit(-1);
    }

    //  cout<<"Check"<<endl;

    //pthread_join(t_id , NULL);

    //  pthread_join(client_id,NULL);
  }

  close(new_serv_sockfd);
  close(serv_sockfd);

  return NULL;
}

/* void * callPeerFun(void *n)
  {

     int cli_peerfd=0;
     int port=stoi(portPeerInfo);

      struct sockaddr_in serv_addr;
      char msg_buffer[1024];


   cli_peerfd=socket(AF_INET,SOCK_STREAM,0);
   if(cli_peerfd<0)
   {
       perror("Error in creating server socket..");
       exit(-1);
   }

   serv_addr.sin_family=AF_INET;
   serv_addr.sin_port=htons(port);

   if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)   // tells the server to connect with this IP Address. Converts string IP to binary IP
    { 
        perror("Server IP Address could not be validated/linked...");
        exit(-1); 
    } 
   
    if (connect(cli_peerfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    { 
       perror("Failed to connect with server...");
       exit(-1);
    } 
  



    while(1)
    {

        memset(msg_buffer,0,sizeof(msg_buffer)); 
        fgets(msg_buffer,1024,stdin);        

       int n=write(cli_peerfd,msg_buffer,strlen(msg_buffer));

         if(n<0)
         {
           perror("Error in writing..");
           exit(-1);
         }

         memset(msg_buffer,0,sizeof(msg_buffer));


         n = read( cli_sockfd , msg_buffer, 1024); 

         if(n<0)
         {
           perror("Error in reading..");
           exit(-1);
         }
        cout<<"Peer Server: "<<msg_buffer;


        int i=strncmp("Bye",msg_buffer,3);
         
         if(i==0)
         break;


    }


    close(cli_peerfd);

    return NULL;

  } */

/**   ..............                      MAIN Function starts here........              **/

int main(int argc, char *argv[])
{

  /*--------------------------------------------     Server Thread  ------------------------------------------------------*/

  pthread_t serv_thread; // Create a server thread for this peer
  int num = 0;

  self_serv_portNo = string(argv[2]);
  self_cli_Id = string(argv[1]);

  serv_port_no = stoi(string(argv[2])); // Args have been updated

  if (pthread_create(&serv_thread, NULL, serverHandler, (void *)&num) < 0) // Server Thread
  {
    perror("Error in creating thread...");
    exit(-1);
  }

  // pthread_join(serv_thread,NULL);

  /*  ---------------------------------------------------------------------------------------------------------------*/

  int cli_port_no = stoi(string(argv[4])); // Args have been updated

  int cli_sockfd;
  struct sockaddr_in serv_addr;
  char msg_buffer[1024];

  cli_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (cli_sockfd < 0)
  {
    perror("Error in creating server socket..");
    exit(-1);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(cli_port_no);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) // tells the server to connect with this IP Address. Converts string IP to binary IP
  {
    perror("Server IP Address could not be validated/linked...");
    exit(-1);
  }

  if (connect(cli_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("Failed to connect with server...");
    exit(-1);
  }

  while (1)
  {

    string cmd = "";
    string inputString = "";
    int fileSize = 0;    // For upload file functionality
    string shaHash = ""; // For upload file functionality

    /*  getline(cin,command_name);

        switch(command_name)
        {
           case "create_account":  // create_account

           string ipPeer;
           string portPeer;

           cin>>ipPeer>>portPeer;

            ipPeerInfo=ipPeer;
            portPeerInfo=portPeer;

           pthread_t cli_thread;
            if( pthread_create( &serv_thread ,NULL ,callPeerFun() ,(void*) &num) < 0)   // Client Thread
            {
               perror("Error in creating client peer thread...");
               exit(-1);
            }

             
             break;

 
        }   */

    memset(msg_buffer, 0, sizeof(msg_buffer));

    fgets(msg_buffer, 1024, stdin);

    inputString = string(msg_buffer);

    int i = 0;
    int j = 0;
    string dumStr = "";
    vector<string> parseVector;

    while (i < inputString.length())
    {
      if (inputString[i] == ' ' || inputString[i] == '\n')
      {
        parseVector.push_back(dumStr);
        dumStr = "";
        j = i + 1;
        while (j < inputString.length() && (inputString[j] == ' ' || inputString[j] == '\n'))
        {
          j++;
        }
        i = j - 1;
      }
      else
      {
        dumStr += inputString[i];
      }

      i++;
    }
    parseVector.push_back(dumStr);

    cmd = parseVector[0];
    inputString = "";
    string parseInput = "";

    if (cmd == "upload_file")
    {
      parseInput = string(msg_buffer);
      string path = parseVector[1];
      unsigned long long int fileSize = getFileSize(path);
      string sha_hash = getHash(path);
      parseInput += to_string(fileSize) + " " + sha_hash + " " + self_serv_portNo + " " + self_cli_Id;
      memset(msg_buffer, 0, sizeof(msg_buffer));
    }
    else
    {
      parseInput = string(msg_buffer);
      parseInput += self_serv_portNo + " " + self_cli_Id;
      memset(msg_buffer, 0, sizeof(msg_buffer));
    }

    /* string parseInput=string(msg_buffer);
          parseInput+=self_serv_portNo+" "+self_cli_Id;
          memset(msg_buffer,0,sizeof(msg_buffer)); */

    strcpy(msg_buffer, parseInput.c_str());

    int n = write(cli_sockfd, msg_buffer, strlen(msg_buffer));

    if (n < 0)
    {
      perror("Error in writing..");
      exit(-1);
    }

    memset(msg_buffer, 0, sizeof(msg_buffer));

    n = read(cli_sockfd, msg_buffer, 1024);

    if (n < 0)
    {
      perror("Error in reading..");
      exit(-1);
    }
    cout << "Server: " << msg_buffer << endl;

    string ackString = string(msg_buffer);

    // string current_peer_user_id="";   // Stores the current user id after 'S' acknowledgement from  server

    // unordered_map<string,string> groupAdmin_peer;     // groupId,userId
    // unordered_map<string,set<string>> groupMembers_peer;     // groupId,Members of the group

    if (ackString[0] == 'S' && cmd == "download_file")
    {

      string newStr = ackString.substr(3, ackString.length() - 4); // Temporary
      cout << newStr << endl;

      string s = "";

      for (int i = 0; i < newStr.length(); i++)
      {
        if (newStr[i] == ' ')
        {
          checkVector.push_back(s);
          s = "";
        }
        else
        {
          s = s + newStr[i];
        }
      }
      checkVector.push_back(s);

      //  downloadFn();

      pthread_t cli_thread;
      int n = 0;

      if (pthread_create(&cli_thread, NULL, callPeerFun, (void *)&n) < 0) // Client Thread
      {
        perror("Error in creating thread...");
        exit(-1);
      }
    }

    /*  else if(ackString[0]=='S' && cmd=='create_group')
        {
            string gid=parseVector[1];
            groupAdmin_peer[gid]=current_peer_user_id;
            groupMembers_peer[gid].insert(current_peer_user_id);
        }
        else if(ackString[0]=='S' && cmd=='create_group')
        {

        }  */

    /*   int i=strncmp("Bye",msg_buffer,3);
         
         if(i==0)
         break;  */
  }

  close(cli_sockfd);

  return 0;
}
