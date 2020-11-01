#include "common.h"

#define PEERLIMIT 10

pair<string,int> THIS_TRACK_SOCK;
pthread_t peer_TID[PEERLIMIT];
char MSG_BUFF[BUFFER_SIZE];
map<string,string> ALL_PEERS;
map<string,string> ACTIVE_PEERS;
struct GroupStruct {
    string gid;
    string owner;
    vector<string> members;
    vector<string> files;
};
struct FileStruct {
    string sha;
    long totalchunks;
    map<pair<string,int>,string> seeders;  //<peer,path>
    map<pair<string,int>,string> leechers; //<peer,path>
};
map<string,GroupStruct> GROUP_INFO;
map<pair<string,string>,FileStruct> FILE_INFO;  //<filename,gid>

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

string handle_create_group(string gid, string user) {
    string status;
    if(GROUP_INFO.find(gid)!=GROUP_INFO.end()) {
        status = "Group "+gid+" already exists\n";
    }
    else {
        GroupStruct grpst;
        grpst.gid = gid;
        grpst.owner = user;
        grpst.members.push_back(user);
        GROUP_INFO[gid] = grpst;
        status = "Group "+gid+" created\n";
    }
    return status;
}

string handle_join_group(string gid, string user) {
    string status;
    if(GROUP_INFO.find(gid)==GROUP_INFO.end()) {
        status = "Group "+gid+" does not exist\n";
    }
    else {
        if(find(GROUP_INFO[gid].members.begin(),GROUP_INFO[gid].members.end(),user)!=GROUP_INFO[gid].members.end()) {
            status = "You are already a member of group "+gid+"\n";
        }
        else {
            GROUP_INFO[gid].members.push_back(user);
            status = "Joined group "+gid+"\n";
        }
    }
    return status;
}

string handle_upload_file(string filep, string gid, string ipadr, string port, string user, string fsize) {
    string status;
    if(ALL_PEERS.empty() || ALL_PEERS.find(user) == ALL_PEERS.end()) {
        status = user+" does not exist";
    }
    else if(GROUP_INFO.find(gid)==GROUP_INFO.end()) {
        status = "Group "+gid+" does not exist\n";
    }
    else if(find(GROUP_INFO[gid].members.begin(),GROUP_INFO[gid].members.end(),user)==GROUP_INFO[gid].members.end()) {
        status = "You are not a member of group "+gid+"\n";
    }
    else {
        int idx = filep.find_last_of('/');
        string filename = filep.substr(idx+1,filep.length());
        pair<string,string> fg = make_pair(gid,filename);
        if(FILE_INFO.find(fg) != FILE_INFO.end()) {
            status = "File "+filename+" is already shared in group "+gid+"\n";
        }
        else {
            int portno = stoi(port);
            long fsz = stol(fsize);
            int totchunks = ceil(fsz/CHUNK_SIZE);
            FileStruct flst;
            pair<string,int> psock = make_pair(ipadr,portno);
            flst.seeders[psock] = filep;
            flst.totalchunks = totchunks;
            // calculate SHA1
            FILE_INFO[fg] = flst;
            status = "File "+filename+" is now uploaded to group "+gid+"\n";
        }
    }
    return status;
}

string handle_list_groups(string user) {
    string status;
    if(GROUP_INFO.empty()) {
        status = "No groups";
    }
    else {
        for(auto grp : GROUP_INFO) {
            status += grp.first+"\t";
        }
    }
    return status;
}

void* serveRequest(void *args) {
//    cout << "here" << endl;
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
        }
        else if(rcvd_cmd[0] == "create_group") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_create_group(rcvd_cmd[1],rcvd_cmd[2]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "join_group") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_join_group(rcvd_cmd[1],rcvd_cmd[2]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "upload_file") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_upload_file(rcvd_cmd[1],rcvd_cmd[2],rcvd_cmd[3],rcvd_cmd[4],rcvd_cmd[5],rcvd_cmd[6]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "list_groups") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_list_groups(rcvd_cmd[1]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "exit") {
            cout << rcvd_cmd[0] << " request" << endl;
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
            cout << "Connected to " << client_addr.sin_addr.s_addr << " : " << client_addr.sin_port << endl;
            if (pthread_create(&peer_TID[i++], NULL, serveRequest, &client_sock) != 0) {
                perror("\nFailed to create server request service thread ");
            }

        }
    }
    pthread_exit(NULL);
}

