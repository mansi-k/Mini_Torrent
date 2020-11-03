#include "common.h"

pthread_t tid_ps;
pthread_t tid_pr;

//struct ChunkStruct {
//    long totalchunks;
//    char chunks[totalchunks];
//};

pair<string,int> THIS_PEER_SOCK;
vector<pair<string,int>> TRACK_SOCK_VEC;
char MSG_BUFF[BUFFER_SIZE];
string CURR_USER="";
map<pair<string,string>,set<int>> FILE_CHUNKS_INFO;  //<gid,filename>


void* serveRequest(void *args) {
    
}

void* peerServer(void *args) {
    cout << "in server" << endl;
    struct sockaddr_in serverSock;
//    int newsock,i;
    int socketfd = socket(PF_INET, SOCK_STREAM, 0);
    if(socketfd < 0) {
        perror("socket failed\n");
        exit(1);
    }
    cout<<"Socket created\n";
    memset(&serverSock, '\0', sizeof(serverSock));
    serverSock.sin_family = AF_INET;
    serverSock.sin_addr.s_addr = inet_addr(THIS_PEER_SOCK.first.c_str());
    unsigned int portno = (unsigned short) THIS_PEER_SOCK.second;
    serverSock.sin_port = htons(portno);
    if(bind(socketfd,(struct sockaddr*) &serverSock, sizeof(serverSock)) < 0) {
        perror("\nError in binding ");
        exit(1);
    }
    if(listen(socketfd,QLIMIT) < 0) {
        perror("\nError in listen ");
        exit(1);
    }
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_sock;
    if(client_sock = accept(socketfd,(struct sockaddr*) &client_addr,&addr_len))
    {
        //send(newsock,str,strlen(str),0);
        if(pthread_create(&tid_pr,NULL,serveRequest,&client_sock)!=0) {
            perror("\nFailed to create server request service thread ");
        }
    }
    pthread_exit(NULL);
}

int main(int argc,char ** argv) {
    if(argc<3) {
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
    if(pthread_create(&tid_ps, NULL, peerServer, NULL)!= 0) {
        perror("Failed to create server thread\n");
    }

    int clientSock = socket(PF_INET,SOCK_STREAM, 0);
    if(clientSock < 0) {
        perror("\nFailed to create client socket ");
        exit(1);
    }

//    cout << getpeername()

    struct sockaddr_in trackerSock;
    memset(&trackerSock, '\0', sizeof(trackerSock));
    trackerSock.sin_addr.s_addr = inet_addr(TRACK_SOCK_VEC[0].first.c_str());
    trackerSock.sin_port = htons(TRACK_SOCK_VEC[0].second);
    trackerSock.sin_family = AF_INET;

    if(connect(clientSock,(struct sockaddr*) &trackerSock,sizeof(trackerSock)) < 0) {
        perror("\nFailed to connect to tracker");
    }
//    string send_msg;
//    cin >> send_msg;
//    if(send(clientSock,send_msg.c_str(),send_msg.length()+1,0) < 0) {
//        perror("Send failed");
//    }
//    recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
//    string msg = string(MSG_BUFF);
//    cout << msg << endl;

    string rqst;
    while(true) {
        getline(cin,rqst);
//        cout << rqst << endl;
        vector<string> rqst_vec = split_string(rqst,' ');
        string cmd = rqst_vec[0];
//        cout << cmd << "." <<  endl;
        if(cmd == "create_user") {
            if(rqst_vec.size()<3) {
                cout << "Usage : create_user <username> <password>" << endl;
                continue;
            }
            string cmd_params = rqst_vec[0]+"|"+rqst_vec[1]+"|"+rqst_vec[2]+"|"+THIS_PEER_SOCK.first+"|"+to_string(THIS_PEER_SOCK.second);
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            string rcvd_msg = string(MSG_BUFF);
            cout << rcvd_msg << endl;
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(cmd == "login") {
            if(rqst_vec.size()<3) {
                cout << "Usage : login <username> <password>" << endl;
                continue;
            }
            if(CURR_USER!="") {
                cout << "You are already Logged in" << endl;
                continue;
            }
            string cmd_params = rqst_vec[0]+"|"+rqst_vec[1]+"|"+rqst_vec[2];
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            string rcvdmsg = string(MSG_BUFF);
            cout << rcvdmsg << endl;
            if(rcvdmsg == "Login success\n")
                CURR_USER = rqst_vec[1];
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(cmd == "logout") {
            if(CURR_USER=="") {
                cout << "You are not logged in" << endl;
                continue;
            }
            string cmd_params = rqst_vec[0]+"|"+CURR_USER;
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            CURR_USER = "";
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(cmd == "create_group") {
            if(rqst_vec.size()<2) {
                cout << "Usage : create_group <groupname>" << endl;
                continue;
            }
            if(CURR_USER=="") {
                cout << "You are not logged in" << endl;
                continue;
            }
            string cmd_params = rqst_vec[0]+"|"+rqst_vec[1]+"|"+CURR_USER;
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(cmd == "join_group") {
            if(rqst_vec.size()<2) {
                cout << "Usage : join_group <groupname>" << endl;
                continue;
            }
            if(CURR_USER=="") {
                cout << "You are not logged in" << endl;
                continue;
            }
            string cmd_params = rqst_vec[0]+"|"+rqst_vec[1]+"|"+CURR_USER;
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(cmd == "upload_file") {
            if(rqst_vec.size()<3) {
                cout << "Usage : upload_file <filepath> <group>" << endl;
                continue;
            }
            if(CURR_USER=="") {
                cout << "You are not logged in" << endl;
                continue;
            }
            struct stat filestatus;
            stat(rqst_vec[1].c_str(), &filestatus);
            long fsz = filestatus.st_size;
            int totchunks = ceil(fsz/CHUNK_SIZE);
            totchunks += fsz % CHUNK_SIZE;
            string cmd_params = rqst_vec[0]+"|"+rqst_vec[1]+"|"+rqst_vec[2]+"|"+THIS_PEER_SOCK.first+"|"+to_string(THIS_PEER_SOCK.second)+"|"+CURR_USER+"|"+to_string(totchunks);
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            string rspmsg = string(MSG_BUFF);
            if(rspmsg.find("is now uploaded to group") != string::npos) {
                pair<string,string> gf = make_pair(rqst_vec[2],rqst_vec[1]);
                set<int> chnks;
                for(int i=0;i<totchunks;i++) {
                    chnks.insert(i);
                }
                FILE_CHUNKS_INFO[gf] = chnks;
            }
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(cmd == "list_groups") {
            if(CURR_USER=="") {
                cout << "You are not logged in" << endl;
                continue;
            }
            string cmd_params = rqst_vec[0]+"|"+CURR_USER;
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(cmd == "list_files") {
            if(rqst_vec.size()<2) {
                cout << "Usage : list_files <group>" << endl;
                continue;
            }
            if(CURR_USER=="") {
                cout << "You are not logged in" << endl;
                continue;
            }
            string cmd_params = rqst_vec[0]+"|"+rqst_vec[1]+"|"+CURR_USER;
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(cmd == "stop_share") {
            if(rqst_vec.size()<3) {
                cout << "Usage : stop_share <group> <filename>" << endl;
                continue;
            }
            if(CURR_USER=="") {
                cout << "You are not logged in" << endl;
                continue;
            }
            string cmd_params = rqst_vec[0]+"|"+rqst_vec[1]+"|"+rqst_vec[2]+"|"+THIS_PEER_SOCK.first+"|"+to_string(THIS_PEER_SOCK.second)+"|"+CURR_USER;
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(cmd == "leave_group") {
            if(rqst_vec.size()<2) {
                cout << "Usage : leave_group <groupname>" << endl;
                continue;
            }
            if(CURR_USER=="") {
                cout << "You are not logged in" << endl;
                continue;
            }
            string cmd_params = rqst_vec[0]+"|"+rqst_vec[1]+"|"+CURR_USER;
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(cmd == "download_file") {
            if(rqst_vec.size()<3) {
                cout << "Usage : download_file <group> <destination_path>" << endl;
                continue;
            }
            if(CURR_USER=="") {
                cout << "You are not logged in" << endl;
                continue;
            }
            string cmd_params = rqst_vec[0]+"|"+rqst_vec[1]+"|"+rqst_vec[2]+"|"+THIS_PEER_SOCK.first+"|"+to_string(THIS_PEER_SOCK.second)+"|"+CURR_USER;
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
//            string rspmsg = string(MSG_BUFF);
//            if(rspmsg.find("download complete") != string::npos) {
//                pair<string,string> gf = make_pair(rqst_vec[2],rqst_vec[1]);
//                set<int> chnks;
//                for(int i=0;i<totchunks;i++) {
//                    chnks.insert(i);
//                }
//                FILE_CHUNKS_INFO[gf] = chnks;
//            }
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(cmd == "exit") {
            if(CURR_USER=="") {
                break;
            }
            string cmd_params = rqst_vec[0]+"|"+CURR_USER;
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            CURR_USER = "";
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
            break;
        }
        else {
            cout << "Wrong command" << endl;
        }
    }
    return 0;
}
