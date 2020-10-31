#include "common.h"

#define PEERLIMIT 10

pair<string,int> THIS_TRACK_SOCK;
pthread_t peer_TID[PEERLIMIT];
char MSG_BUFF[BUFFER_SIZE];
map<string,string> ALL_PEERS;
map<string,string> ACTIVE_PEERS;

void setSocket(string trkfile) {
    fstream fs(trkfile,ios::in);
    string tadr;
    getline(fs,tadr);
    THIS_TRACK_SOCK = split_address(tadr);
    cout << THIS_TRACK_SOCK.first << " : " << THIS_TRACK_SOCK.second << endl;
}

string handle_create_user(string user, string pswd) {
    auto itr = ALL_PEERS.find(user);
    string status;
    if(itr == ALL_PEERS.end()) {
        ALL_PEERS[user] = pswd;
        status = "New user created with --> NAME:"+user+" PASSWORD:"+pswd+"\n";
    }
    else {
        status = "User "+user+" already exists\n";
    }
    return status;
}

string handle_logout(string user) {
    auto itr = ACTIVE_PEERS.find(user);
    string status;
    if(itr == ACTIVE_PEERS.end()) {
        status = "You are not logged in\n";
    }
    else {
        ACTIVE_PEERS.erase((user));
        status = user+" logged out\n";
    }
    return status;
}

string handle_login(string user, string pswd) {
    string status;
    if(!ACTIVE_PEERS.empty() && ACTIVE_PEERS.find(user) != ACTIVE_PEERS.end()) {
        status = "You are already logged in\n";
    }
    else if(ALL_PEERS.empty() || ALL_PEERS.find(user) == ALL_PEERS.end()) {
        status = user+" does not exist";
    }
    else {
        if(ALL_PEERS[user] == pswd) {
            ACTIVE_PEERS[user] = pswd;
            status = "Login success\n";
        }
        else {
            status = "Wrong password\n";
        }
    }
    return status;
}

void* serveRequest(void *args) {
    cout << "here" << endl;
    int client_sock = *((int *)args);
    while(true) {
        recv(client_sock, MSG_BUFF, BUFFER_SIZE, 0);
        vector<string> rcvd_cmd = split_string(MSG_BUFF, '|');
        string send_msg;
        if (rcvd_cmd[0] == "create_user") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_create_user(rcvd_cmd[1], rcvd_cmd[2]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "login") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_login(rcvd_cmd[1], rcvd_cmd[2]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "logout") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_logout(rcvd_cmd[1]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "exit") {
            cout << rcvd_cmd[0] << "request" << endl;
            send_msg = handle_logout(rcvd_cmd[1]);
            send(client_sock,send_msg.c_str(),BUFFER_SIZE,0);
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
            break;
        }
    }
//    cout << "got " << string(MSG_BUFF) << endl;
//    string send_msg = "Hello ";
//    send_msg += string(MSG_BUFF);
//    send(client_sock,send_msg.c_str(),BUFFER_SIZE,0);
    pthread_exit(NULL);
}

int main(int argc,char ** argv) {
    if(argc<2) {
        cout << "Usage : tracker.cpp <tracker_file>" << endl;
        return 0;
    }
    setSocket(argv[1]);
    cout << "in server" << endl;
    struct sockaddr_in serverSock;
//    int newsock,i;
    int socketfd = socket(PF_INET, SOCK_STREAM, 0);
    if(socketfd < 0)	//tcp -sock_stream  af_inet - ipv4
    {
        perror("\nSocket failed ");
        exit(1);
    }
    cout<<"Socket created\n";
    memset(&serverSock, '\0', sizeof(serverSock));
    serverSock.sin_family = AF_INET;
    serverSock.sin_addr.s_addr = inet_addr(THIS_TRACK_SOCK.first.c_str());
    unsigned int number = (unsigned short) THIS_TRACK_SOCK.second;
//    strcpy(port,argv[2]);
//    cout<<port;
    serverSock.sin_port = htons(number);
    if(bind(socketfd,(struct sockaddr*) &serverSock, sizeof(serverSock)) < 0) {
        perror("\nError in binding ");
        exit(1);
    }
//    cout<<"Bind Successful\n";
    if(listen(socketfd,QLIMIT) < 0) {
        perror("\nError in listen ");
        exit(1);
    }
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_sock;
    int i=0;
    while(true) {
        if (client_sock = accept(socketfd, (struct sockaddr *) &client_addr, &addr_len)) {
            //send(newsock,str,strlen(str),0);
            if (pthread_create(&peer_TID[i++], NULL, serveRequest, &client_sock) != 0) {
                perror("\nFailed to create server request service thread ");
            }

        }
    }
    pthread_exit(NULL);
}

