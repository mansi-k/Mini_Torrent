#include "common.h"
#include <semaphore.h>

pthread_t tid_ps;
pthread_t tid_pr;

struct ChunkStruct {
    string dpath;
    vector<int> fchunks;
};

struct DownlConfig {
    string gid;
    string srcfile;
    string destp;
    int totchunks;
    int dl_sock;
    pair<string,int> chunks_from;
    vector<int> which_chunks;
};

pair<string,int> THIS_PEER_SOCK;
vector<pair<string,int>> TRACK_SOCK_VEC;
char MSG_BUFF[BUFFER_SIZE];
string CURR_USER="";
map<pair<string,string>,ChunkStruct> FILE_CHUNKS_INFO;  //<gid,filename>
sem_t m;

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
    while(true) {
        if(client_sock = accept(socketfd, (struct sockaddr *) &client_addr, &addr_len)) {
//        if(pthread_create(&tid_pr,NULL,serveRequest,&client_sock)!=0) {
//            perror("\nFailed to create server request service thread ");
//        }
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
            recv(client_sock,MSG_BUFF,BUFFER_SIZE,0);
            vector<string> rmsg = split_string(MSG_BUFF,'|');
            cout << "Request for gid="+rmsg[0]+" file="+rmsg[1] << endl;
            pair<string,string> pgf = make_pair(rmsg[0],rmsg[1]);
            cout << FILE_CHUNKS_INFO[pgf].fchunks.size() << endl;
            string bvec = bitvec_toString(FILE_CHUNKS_INFO[pgf].fchunks);
            cout << "after bvec" << endl;
            send(client_sock,bvec.c_str(),bvec.length(),0);
            cout << "sent bvec" << endl;
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
            recv(client_sock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            rmsg = split_string(MSG_BUFF,'|');
//            vector<int> chunks_tosend = split_bitvector(rmsg[1],';',stoi(rmsg[0]));
            vector<int> chunks_tosend;
            cout << rmsg[1] << endl;
            vector<string> chkmsg = split_string(rmsg[1],';');
            for(int i=0;i<chkmsg.size();i++) {
                chunks_tosend.push_back(stoi(chkmsg[i]));
                cout << chunks_tosend[i] << " ";
            }
            cout << endl;
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
            string fpath = FILE_CHUNKS_INFO[pgf].dpath+"/"+pgf.second;
            FILE *fp = fopen(fpath.c_str(),"rb");
            if(fp == NULL) {
                perror("\nFile null");
            }
//            ifstream fp(fpath, ifstream::binary);
//            if(!fp.is_open()) {
//                perror("\nFile null");
//            }
            char CHUNK_BUFF[CHUNK_SIZE];
            int readsize, fs;
            cout << fpath << " " << chunks_tosend.size() << " " << pgf.second << endl;
            for(int ci=0;ci<chunks_tosend.size();ci++) {
                memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
                recv(client_sock,MSG_BUFF,BUFFER_SIZE,0);
                memset(&CHUNK_BUFF, 0, sizeof(CHUNK_BUFF));
                cout << "Request for chunk " << MSG_BUFF << " ........ " << "Sending chunk " << chunks_tosend[ci] << endl;
                fs = fseek(fp,chunks_tosend[ci]*CHUNK_SIZE,SEEK_SET);
//                fp.seekg(chunks_tosend[ci]*CHUNK_SIZE,ios::beg);
                if(fs != 0) {
                    perror("\nseek nonzero ");
                }
                readsize = fread(&CHUNK_BUFF,sizeof(char),CHUNK_SIZE,fp);
                cout << readsize << endl;
                cout << CHUNK_BUFF << endl;
                if(readsize <= 0) {
                    perror("\nnot read ");
                }
//                fp.read(CHUNK_BUFF,CHUNK_SIZE);
                string testmsg = "hello";
                send(client_sock,CHUNK_BUFF,readsize,0);
            }
            cout << "All chunks sent" << endl;
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
//            string msg = "Download complete";
//            send(client_sock,msg.c_str(),msg.length(),0);
            close(client_sock);
        }
    }
    pthread_exit(NULL);
}

void* fileDownloader(void *args) {
    cout << "in fileDownloader" << endl;
    DownlConfig down_config = *((DownlConfig *)args);
    cout << down_config.chunks_from.first << ":" << down_config.chunks_from.second << endl;
    for(auto it=down_config.which_chunks.begin();it!=down_config.which_chunks.end();it++) {
        cout << *it << " ";
    }
    cout << endl;
    string chunks = bitvec_toString(down_config.which_chunks);
    cout << chunks << endl;
    struct sockaddr_in cpeerSock;
    memset(&cpeerSock, '\0', sizeof(cpeerSock));
    cpeerSock.sin_addr.s_addr = inet_addr(down_config.chunks_from.first.c_str());
    cpeerSock.sin_port = htons(down_config.chunks_from.second);
    cpeerSock.sin_family = AF_INET;
    string chnkmsg = to_string(down_config.totchunks)+"|"+chunks;
    sendto(down_config.dl_sock,chnkmsg.c_str(),chnkmsg.length()+1,0,(struct sockaddr*) &cpeerSock,sizeof(cpeerSock));
    memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
    sem_wait(&m);
//    char fChunk[CHUNK_SIZE];
    int rcvlen;
    string dfpath = down_config.destp+"/"+down_config.srcfile;
    cout << dfpath << endl;
    pair<string,string> gf = make_pair(down_config.gid,down_config.srcfile);
//    fstream fin;
//    fin.open(dfpath.c_str(), fstream::in|fstream::out|fstream::trunc);
    FILE *fin = fopen(dfpath.c_str(),"wb");
//    fin.open(dfpath.c_str(), ios::out|ios::in|ios::binary);


    for(int ci=0;ci<down_config.which_chunks.size();ci++) {
        char CHUNK_BUFF[CHUNK_SIZE];
        memset(&CHUNK_BUFF, '\0', CHUNK_SIZE);
        string chnkno = to_string(down_config.which_chunks[ci]);
        send(down_config.dl_sock,chnkno.c_str(),chnkno.length()+1,0);
//        sendto(down_config.dl_sock,chnkno.c_str(),chnkno.length()+1,0,(struct sockaddr*) &cpeerSock,sizeof(cpeerSock));
//        fin.seekp(down_config.which_chunks[ci]*CHUNK_SIZE,ios::beg);
        cout << "Downloading chunk " << down_config.which_chunks[ci] << " ........ ";
//        rcvlen = recvfrom(down_config.dl_sock, CHUNK_BUFF, CHUNK_SIZE, 0, (struct sockaddr*) &cpeerSock,(socklen_t *)(sizeof(cpeerSock)));
        rcvlen = recv(down_config.dl_sock, CHUNK_BUFF, CHUNK_SIZE, 0);
        cout << "Recieved chunk " << down_config.which_chunks[ci] << " " << rcvlen << endl;
        cout << CHUNK_BUFF << endl;
//        if(rcvlen <= 0) {
//            perror("\nnot recieved");
//        }

//        flock(fileno(fin), LOCK_EX|LOCK_NB);
        string to_write = string(CHUNK_BUFF);
//        lseek(fileno(fin), down_config.which_chunks[ci]*CHUNK_SIZE, SEEK_SET);
        fseek(fin,down_config.which_chunks[ci]*CHUNK_SIZE,SEEK_SET);
        fwrite(CHUNK_BUFF,sizeof(char),rcvlen,fin);
//        fwrite(to_write.c_str(),sizeof(char),to_write.length(),fin);
//        flock(fileno(fin), LOCK_UN);
        FILE_CHUNKS_INFO[gf].fchunks.push_back(ci);

//        CHUNK_BUFF = to_string(ci).c_str();
//        fin.write(to_string(ci).c_str(),to_string(ci).length());
        // send info to tracker as leecher
    }
//    memset(&CHUNK_BUFF, 0, sizeof(CHUNK_BUFF));
    memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
    fclose(fin);
    sem_post(&m);
//    fin.close();


//    recvfrom(down_config.dl_sock, MSG_BUFF, BUFFER_SIZE, 0, (struct sockaddr*) &cpeerSock,(socklen_t *)(sizeof(cpeerSock)));
//    memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
    cout << "exiting fileDownloader" << endl;
    close(down_config.dl_sock);
    pthread_exit(NULL);
}

void downloadConfigure(string gid, string filename, string destp, int totalchunks, string sha, vector<pair<string,int>> active_peer_chunks) {
    cout << "in downloadConfigure" << endl;
    map<pair<string,int>,vector<int>> chunks_peers_have;
    map<pair<string,int>,int> peer_DlSocks;
    for(auto apcit=active_peer_chunks.begin();apcit!=active_peer_chunks.end();apcit++) {
        int DlSock = socket(PF_INET,SOCK_STREAM, 0);
        if(DlSock < 0) {
            perror("\nFailed to create downloader socket ");
            exit(1);
        }
        // connect & request for chunk info
        cout << "connecting " << apcit->first << ":" << apcit->second << endl;
        struct sockaddr_in peerSock;
        memset(&peerSock, '\0', sizeof(peerSock));
        peerSock.sin_addr.s_addr = inet_addr(apcit->first.c_str());
        peerSock.sin_port = htons(apcit->second);
        peerSock.sin_family = AF_INET;
        if(connect(DlSock,(struct sockaddr*) &peerSock,sizeof(peerSock)) < 0) {
            perror("\nFailed to connect to peer");
        }
        cout << "connected to " << apcit->first << ":" << apcit->second << " --> ";
        string send_msg = gid+"|"+filename;
        memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        send(DlSock,send_msg.c_str(),send_msg.length()+1,0);
        recv(DlSock,MSG_BUFF,BUFFER_SIZE,0);
        cout << MSG_BUFF << endl;
        chunks_peers_have[*apcit] = split_bitvector(MSG_BUFF,';',totalchunks);
        peer_DlSocks[*apcit] = DlSock;
        memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
    }
    // loop to choose chunks from each peer
    map<pair<string,int>,vector<int>> pick_chunks_from;
    int ci=0;
    int pcnt=0;
    while(ci<totalchunks) {
        for(auto apcit=chunks_peers_have.begin();apcit!=chunks_peers_have.end();apcit++) {
            if(pcnt==chunks_peers_have.size() && apcit->second[ci]==0) {
                cout << "Could not find chunk " << ci << endl;
                return;
            }
            else if(apcit->second[ci]==0) {
                pcnt++;
                continue;
            }
            else {
                pick_chunks_from[apcit->first].push_back(ci);
                ci++;
                pcnt = 0;
            }
        }
    }
    // vector of DownConfig
    vector<DownlConfig> downcfg_vec;
    for(auto cfit=pick_chunks_from.begin();cfit!=pick_chunks_from.end();cfit++) {
//        cout << "using cfit" << " --> ";
//        for(auto jtr=cfit->second.begin();jtr!=cfit->second.end();jtr++) {
//            cout << *jtr << " ";
//        }
//        cout << endl;
        DownlConfig down_config;
        down_config.totchunks = totalchunks;
        down_config.srcfile = filename;
        down_config.gid = gid;
        down_config.destp = destp;
        down_config.dl_sock = peer_DlSocks[cfit->first];
        down_config.chunks_from = cfit->first;
        down_config.which_chunks = cfit->second;
//        cout << "using struct" << " --> ";
//        for(auto jtr=down_config.which_chunks.begin();jtr!=down_config.which_chunks.end();jtr++) {
//            cout << *jtr << " ";
//        }
//        cout << endl;
        downcfg_vec.push_back(down_config);
    }
    // create thread for each peer
    int d = 0;
    pthread_t downl_TID[downcfg_vec.size()];
    for(auto vsix=0;vsix<downcfg_vec.size();vsix++) {
//        cout << "using vector" << " --> ";
//        for(auto jtr=downcfg_vec[vsix].which_chunks.begin();jtr!=downcfg_vec[vsix].which_chunks.end();jtr++) {
//            cout << *jtr << " ";
//        }
//        cout << endl;
        if (pthread_create(&downl_TID[vsix], NULL, fileDownloader, &downcfg_vec[vsix]) != 0) {
            perror("\nFailed to create downloader thread ");
        }
        pthread_join(downl_TID[vsix],NULL);
    }
//    close(DlSock);
    cout << "exiting downloadConfigure" << endl;
}

int main(int argc,char ** argv) {
    sem_init(&m,0,1);
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
            int totchunks = ceil((float)fsz/CHUNK_SIZE);
            string cmd_params = rqst_vec[0]+"|"+rqst_vec[1]+"|"+rqst_vec[2]+"|"+THIS_PEER_SOCK.first+"|"+to_string(THIS_PEER_SOCK.second)+"|"+CURR_USER+"|"+to_string(totchunks);
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            string rspmsg = string(MSG_BUFF);
            int idx = rqst_vec[1].find_last_of('/');
            string filename = rqst_vec[1].substr(idx+1,rqst_vec[1].length());
            pair<string,string> gf = make_pair(rqst_vec[2],filename);
//            FILE_CHUNKS_INFO[gf].dpath = rqst_vec[1].substr(0,idx);
//            cout << FILE_CHUNKS_INFO[gf].dpath << endl;
            if(rspmsg.find("is now uploaded to group") != string::npos) {
//                pair<string,string> gf = make_pair(rqst_vec[2],rqst_vec[1]);
                ChunkStruct chst;
                chst.dpath = rqst_vec[1].substr(0,idx);
                for(int i=0;i<totchunks;i++) {
                    chst.fchunks.push_back(i);
                }
                FILE_CHUNKS_INFO[gf] = chst;
            }
            cout << FILE_CHUNKS_INFO[gf].fchunks.size() << " " << fsz << " " << totchunks << endl;
            for(auto itr=FILE_CHUNKS_INFO[gf].fchunks.begin();itr!=FILE_CHUNKS_INFO[gf].fchunks.end();itr++) {
                cout << *itr << " ";
            }
            cout << endl;
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
            if(rqst_vec.size()<4) {
                cout << "Usage : download_file <group> <filename> <destination_path>" << endl;
                continue;
            }
            if(CURR_USER=="") {
                cout << "You are not logged in" << endl;
                continue;
            }
            string fDestn = rqst_vec[3];
            string cmd_params = rqst_vec[0]+"|"+rqst_vec[1]+"|"+rqst_vec[2]+"|"+THIS_PEER_SOCK.first+"|"+to_string(THIS_PEER_SOCK.second)+"|"+CURR_USER;
            send(clientSock,cmd_params.c_str(),cmd_params.length()+1,0);
            // total_chunks | SHA
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            vector<string> vrmsg = split_string(MSG_BUFF,'|');
            int totchunks = stoi(vrmsg[0]);
            string sha = vrmsg[1];
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
            string ack = "send peers";
            send(clientSock,ack.c_str(),ack.length()+1,0);
            // peer addresses
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << "peers ";
            cout << MSG_BUFF << endl;
            vrmsg.clear();
            vrmsg = split_string(MSG_BUFF,'|');
            if(vrmsg.empty()) {
                cout << "No peers available" << endl;
                continue;
            }
            vector<pair<string,int>> afpeers;
            for(auto it=vrmsg.begin();it!=vrmsg.end();it++) {
                afpeers.push_back(split_address(*it));
            }
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
            pair<string,string> gf = make_pair(rqst_vec[1],rqst_vec[2]);
            FILE_CHUNKS_INFO[gf].dpath = rqst_vec[3];
            string lchrq = "add_leecher|"+rqst_vec[1]+"|"+rqst_vec[2]+"|"+THIS_PEER_SOCK.first+"|"+to_string(THIS_PEER_SOCK.second)+"|"+rqst_vec[3];
            send(clientSock,lchrq.c_str(),lchrq.length()+1,0);
            recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
            cout << MSG_BUFF << endl;
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
            downloadConfigure(rqst_vec[1],rqst_vec[2],rqst_vec[3],totchunks,sha,afpeers);
            if(FILE_CHUNKS_INFO[gf].fchunks.size() == totchunks) {
                cout << "Download complete" << endl;
                string sedq = "add_seeder|"+rqst_vec[1]+"|"+rqst_vec[2]+"|"+THIS_PEER_SOCK.first+"|"+to_string(THIS_PEER_SOCK.second)+"|"+rqst_vec[3];
                send(clientSock,sedq.c_str(),sedq.length()+1,0);
                recv(clientSock,MSG_BUFF,BUFFER_SIZE,0);
                cout << MSG_BUFF << endl;
                memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
            }
            else {
                cout << "Incomplete download" << endl;
            }
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
