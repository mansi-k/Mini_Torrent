#include <iostream>
#include <bits/stdc++.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define BUFFER_SIZE 64
#define QLIMIT 32
#define CHUNK_SIZE 1024

pthread_t tid_ps;
pthread_t tid_pr;

using namespace std;

pair<string,int> THIS_PEER_SOCK;
vector<pair<string,int>> TRACK_SOCK_VEC;

pair<string,int> split_address(string addr) {
    int pos = addr.find(":");
    pair<string,int> sock;
    sock.first = addr.substr(0,pos);
    sock.second = stoi(addr.substr(pos+1,addr.length()));
    return sock;
}

vector<string> split_string(string s,char d) {
    vector<string> v;
    stringstream ss(s);
    string temp;
    while(getline(ss,temp,d)) {
        v.push_back(temp);
    }
    return v;
}

void* serveRequest(void *args) {
    
}

void* peerServer(void *args) {
    cout << "in server" << endl;
    struct sockaddr_in serverSock;
//    int newsock,i;
    int socketfd = socket(PF_INET, SOCK_STREAM, 0);
    if(socketfd < 0)	//tcp -sock_stream  af_inet - ipv4
    {
        perror("socket failed\n");
        exit(1);
    }
    cout<<"Socket created\n";
    memset(&serverSock, '\0', sizeof(serverSock));
    serverSock.sin_family = AF_INET;
    serverSock.sin_addr.s_addr = inet_addr(THIS_PEER_SOCK.first.c_str());
    unsigned int number = (unsigned short) THIS_PEER_SOCK.second;
//    strcpy(port,argv[2]);
//    cout<<port;
    serverSock.sin_port = htons(number);
    if(bind(socketfd,(struct sockaddr*) &serverSock, sizeof(serverSock)) < 0) {
        perror("Error in binding\n");
        exit(1);
    }
//    cout<<"Bind Successful\n";
    if(listen(socketfd,QLIMIT) < 0) {
        perror("error in listen\n");
        exit(1);
    }
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_sock;
    if(client_sock = accept(socketfd,(struct sockaddr*) &client_addr,&addr_len))
    {
        //send(newsock,str,strlen(str),0);
        if(pthread_create(&tid_pr,NULL,serveRequest,&client_sock)!=0) {
            perror("Failed to create server request service thread\n");
        }
    }
    pthread_exit(NULL);
}

int main(int argc,char ** argv) {
    if(argc!=3) {
        cout << "Usage : peer.cpp <peer IP:Port> <tracker_file>" << endl;
        return 0;
    }
    string tracker_file = argv[2];
    THIS_PEER_SOCK = split_address(argv[1]);
//    cout << THIS_PEER_SOCK.first << " : " << THIS_PEER_SOCK.second << endl;
    fstream fs(tracker_file,ios::in);
    string tadr;
//    int i=0;
    while(getline(fs,tadr)) {
        TRACK_SOCK_VEC.push_back(split_address(tadr));
//        cout << TRACK_SOCK_VEC[i].first << " : " << TRACK_SOCK_VEC[i].second << endl;
//        i++;
    }
//    thread serverThread(peerServer);
//    serverThread.detach();

//    pthread_create(&tid1,NULL,peerServer,NULL);
    if(pthread_create(&tid_ps, NULL, peerServer, NULL)!= 0) {
        perror("Failed to create server thread\n");
    }

    int clientSock = socket(PF_INET,SOCK_STREAM, 0);
    if(clientSock < 0) {
        perror("Failed to create client socket\n");
        exit(1);
    }

    struct sockaddr_in trackerSock;
    memset(&trackerSock, '\0', sizeof(trackerSock));
    trackerSock.sin_addr.s_addr = inet_addr(TRACK_SOCK_VEC[0].first.c_str());
    trackerSock.sin_port = htons(TRACK_SOCK_VEC[0].second);
    trackerSock.sin_family = AF_INET;

    if(connect(clientSock,(struct sockaddr*) &trackerSock,sizeof(trackerSock)) < 0) {
        perror("Failed to connect to tracker\n");
    }

    string rqst;
    while(true) {
        getline(cin,rqst);
        vector<string> rqst_vec = split_string(rqst,' ');
        string cmd = rqst_vec[0];
        if(cmd == "create_user") {

        }
        else if(cmd == "exit") {
            return 0;
        }
    }
    return 0;
}
