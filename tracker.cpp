#include "common.h"

#define PEERLIMIT 10

pthread_t tid_fd;
pair<string,int> THIS_TRACK_SOCK;
pthread_t peer_TID[PEERLIMIT];
char MSG_BUFF[BUFFER_SIZE];
struct UserStruct {
    string uid;
    string password;
    string ip;
    int port;
};
map<string,UserStruct> ALL_PEERS;
map<string,UserStruct> ACTIVE_PEERS;
struct GroupStruct {
    string gid;
    string owner;
    vector<string> members;
//    vector<string> files;
};
struct FileStruct {
    string sha;
    int totalchunks;
    map<pair<string,int>,string> seeders;  //<<ip,port>,path>
    map<pair<string,int>,string> leechers; //<<ip,port>,path>
};
struct DownRqstStruct {
    string uid;
    string gid;
    string ip;
    int port;
    string srcfile;
    string destp;
    int totchunks;
};
map<string,GroupStruct> GROUP_INFO;
map<pair<string,string>,FileStruct> FILE_INFO;  //<gid,filename>
queue<DownRqstStruct> DOWN_RQST_Q;
map<pair<string,pair<string,int>>,string> DOWN_RSPN;  //<filename,<ip,port>>
struct DownlConfig {
    int dl_sock;
    DownRqstStruct down_rqst;
    pair<string,int> chunks_from;
    vector<int> which_chunks;
};

void setSocket(string trkfile) {
    fstream fs(trkfile,ios::in);
    string tadr;
    getline(fs,tadr);
    THIS_TRACK_SOCK = split_address(tadr);
    cout << THIS_TRACK_SOCK.first << " : " << THIS_TRACK_SOCK.second << endl;
}

//void* fileDownloader(void *args) {
//    cout << "in fileDownloader" << endl;
//    DownlConfig down_config = *((DownlConfig *)args);
//    cout << down_config.chunks_from.first << ":" << down_config.chunks_from.second << endl;
//    for(auto it=down_config.which_chunks.begin();it!=down_config.which_chunks.end();it++) {
//        cout << *it << " ";
//    }
//    cout << endl;
//    string chunks = bitvec_toString(down_config.which_chunks);
//    cout << chunks << endl;
//    struct sockaddr_in cpeerSock;
//    memset(&cpeerSock, '\0', sizeof(cpeerSock));
//    cpeerSock.sin_addr.s_addr = inet_addr(down_config.chunks_from.first.c_str());
//    cpeerSock.sin_port = htons(down_config.chunks_from.second);
//    cpeerSock.sin_family = AF_INET;
//    string chnkmsg = to_string(down_config.down_rqst.totchunks)+"|"+chunks;
//    sendto(down_config.dl_sock,chnkmsg.c_str(),chnkmsg.length()+1,0,(struct sockaddr*) &cpeerSock,sizeof(cpeerSock));
//    memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
//    recvfrom(down_config.dl_sock, MSG_BUFF, BUFFER_SIZE, 0, (struct sockaddr*) &cpeerSock,(socklen_t *)(sizeof(cpeerSock)));
//    pair<string,pair<string,int>> drspn = make_pair(down_config.down_rqst.srcfile,make_pair(down_config.down_rqst.ip,down_config.down_rqst.port));
//    DOWN_RSPN[drspn] = MSG_BUFF;
//    memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
//    cout << "exiting fileDownloader" << endl;
//    pthread_exit(NULL);
//}
//
//void* configDownload(void *args) {
//    cout << "in configDownload" << endl;
////    while(true) {
//        if(DOWN_RQST_Q.empty()) {
////            continue;
//        }
//        while(!DOWN_RQST_Q.empty()) {
//            DownRqstStruct dlrq = DOWN_RQST_Q.front();
//            DOWN_RQST_Q.pop();
//            int DlSock = socket(PF_INET,SOCK_STREAM, 0);
//            if(DlSock < 0) {
//                perror("\nFailed to create downloader socket ");
//                exit(1);
//            }
//            pair<string,string> gf = make_pair(dlrq.gid,dlrq.srcfile);
//            map<pair<string,int>,vector<int>> active_peer_chunks;
//            for(auto sit=FILE_INFO[gf].seeders.begin();sit!=FILE_INFO[gf].seeders.end();sit++) {
//                for(auto apit=ACTIVE_PEERS.begin();apit!=ACTIVE_PEERS.end();apit++) {
//                    if(apit->second.ip == sit->first.first && apit->second.port == sit->first.second) {
//                        vector<int> chkvec;
//                        chkvec.resize(FILE_INFO[gf].totalchunks,0);
////                        active_peer_chunks[make_pair(dlrq.ip,dlrq.port)] = chkvec;
//                        active_peer_chunks[make_pair(sit->first.first,sit->first.second)] = chkvec;
//                        cout << apit->first << " ";
//                    }
//                }
//            }
//            for(auto lit=FILE_INFO[gf].leechers.begin();lit!=FILE_INFO[gf].leechers.end();lit++) {
//                for(auto apit=ACTIVE_PEERS.begin();apit!=ACTIVE_PEERS.end();apit++) {
//                    if(apit->second.ip == lit->first.first && apit->second.port == lit->first.second) {
//                        vector<int> chkvec;
//                        chkvec.resize(FILE_INFO[gf].totalchunks,0);
////                        active_peer_chunks[make_pair(dlrq.ip,dlrq.port)] = chkvec;
//                        active_peer_chunks[make_pair(lit->first.first,lit->first.second)] = chkvec;
//                        cout << apit->first << " ";
//                    }
//                }
//            }
//            cout << "\n" << "selected" << endl;
//            // only to display for testing
//            for(auto apcit=active_peer_chunks.begin();apcit!=active_peer_chunks.end();apcit++) {
//                cout << apcit->first.first << ":" << apcit->first.second << " " << endl;
//            }
//
//            for(auto apcit=active_peer_chunks.begin();apcit!=active_peer_chunks.end();apcit++) {
//                //connect n request for chunk info
//                struct sockaddr_in peerSock;
//                memset(&peerSock, '\0', sizeof(peerSock));
//                peerSock.sin_addr.s_addr = inet_addr(apcit->first.first.c_str());
//                peerSock.sin_port = htons(apcit->first.second);
//                peerSock.sin_family = AF_INET;
//                if(connect(DlSock,(struct sockaddr*) &peerSock,sizeof(peerSock)) < 0) {
//                    perror("\nFailed to connect to tracker");
//                }
//                string send_msg = dlrq.gid+"|"+dlrq.srcfile;
//                memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
//                send(DlSock,send_msg.c_str(),send_msg.length()+1,0);
//                recv(DlSock,MSG_BUFF,BUFFER_SIZE,0);
////                vector<string> rmsg = split_string(MSG_BUFF,'|');  //fpath,bitvec
//                cout << MSG_BUFF << endl;
//                apcit->second = split_bitvector(MSG_BUFF,';',FILE_INFO[gf].totalchunks);
//                memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
//            }
//            // loop to choose chunks from each peer
//            map<pair<string,int>,vector<int>> chunks_from;
//            int ci=0;
//            int pcnt=0;
//            while(ci<FILE_INFO[gf].totalchunks) {
//                for(auto apcit=active_peer_chunks.begin();apcit!=active_peer_chunks.end();apcit++) {
//                    if(pcnt==active_peer_chunks.size() && apcit->second[ci]==0) {
//                        pair<string,pair<string,int>> p = make_pair(dlrq.srcfile,make_pair(dlrq.ip,dlrq.port));
//                        DOWN_RSPN[p] = "Chunk "+to_string(ci)+" is missing\n";
//                        break;
//                    }
//                    else if(apcit->second[ci]==0) {
//                        continue;
//                    }
//                    else {
//                        chunks_from[apcit->first].push_back(ci);
//                        ci++;
//                    }
//                }
//            }
//            cout << "printing chunksfrom" << endl;
//            for(auto itr=chunks_from.begin();itr!=chunks_from.end();itr++) {
//                cout << itr->first.first << ":" << itr->first.second << " tc=" << itr->second.size() << " --> ";
//                for(auto jtr=itr->second.begin();jtr!=itr->second.end();jtr++) {
//                    cout << *jtr << " ";
//                }
//                cout << endl;
//            }
//            int d=0;
//            cout << "print while assigning" << endl;
//            pthread_t downl_TID[chunks_from.size()];
//            vector<DownlConfig> DlCfgVec;
//            for(auto cfit=chunks_from.begin();cfit!=chunks_from.end();cfit++) {
//                cout << "using cfit" << " --> ";
//                for(auto jtr=cfit->second.begin();jtr!=cfit->second.end();jtr++) {
//                    cout << *jtr << " ";
//                }
//                cout << endl;
//                DownlConfig down_config;
//                down_config.down_rqst = dlrq;
//                down_config.dl_sock = DlSock;
//                down_config.chunks_from = cfit->first;
//                down_config.which_chunks = cfit->second;
//                cout << "using struct" << " --> ";
//                for(auto jtr=down_config.which_chunks.begin();jtr!=down_config.which_chunks.end();jtr++) {
//                    cout << *jtr << " ";
//                }
//                cout << endl;
//                DlCfgVec.push_back(down_config);
////                if (pthread_create(&downl_TID[d++], NULL, fileDownloader, &down_config) != 0) {
////                    perror("\nFailed to create downloader thread ");
////                }
//            }
//            for(auto vsix=0;vsix<DlCfgVec.size();vsix++) {
//                cout << "using vector" << " --> ";
//                for(auto jtr=DlCfgVec[vsix].which_chunks.begin();jtr!=DlCfgVec[vsix].which_chunks.end();jtr++) {
//                    cout << *jtr << " ";
//                }
//                cout << endl;
//                if (pthread_create(&downl_TID[d++], NULL, fileDownloader, &DlCfgVec[vsix]) != 0) {
//                    perror("\nFailed to create downloader thread ");
//                }
//            }
//        }
////    }
//    cout << "exiting configDownload" << endl;
//}

string handle_create_user(string user, string pswd, string ipadr, string portno) {
    auto itr = ALL_PEERS.find(user);
    string status;
    if(itr == ALL_PEERS.end()) {
        UserStruct us;
        us.uid = user;
        us.password = pswd;
        us.ip = ipadr;
        us.port = stoi(string(portno));
        ALL_PEERS[user] = us;
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
        if(ALL_PEERS[user].password == pswd) {
            ACTIVE_PEERS[user] = ALL_PEERS[user];
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

string handle_upload_file(string filep, string gid, string ipadr, string port, string user, string nchunks) {
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
            int portno = stoi(string(port));
            long nchk = stol(nchunks);
            FileStruct flst;
            pair<string,int> psock = make_pair(ipadr,portno);
            flst.seeders[psock] = filep;
            flst.totalchunks = nchk;
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
        status += "\n";
    }
    return status;
}

string handle_list_files(string gid, string user) {
    string status;
    if(GROUP_INFO.find(gid) == GROUP_INFO.end()) {
        status = "Group "+gid+" does not exist\n";
    }
    else if(find(GROUP_INFO[gid].members.begin(),GROUP_INFO[gid].members.end(),user)==GROUP_INFO[gid].members.end()) {
        status = "You are not a member of group "+gid+"\n";
    }
    else {
        for(auto fg : FILE_INFO) {
            if(fg.first.first == gid) {
                status += fg.first.second + "\t";
            }
        }
        if(status=="") {
            status = "No files shared";
        }
        status += "\n";
    }
    return status;
}

string handle_stop_share(string gid, string file, string ipadr, string port, string user) {
    string status;
    if(GROUP_INFO.find(gid)==GROUP_INFO.end()) {
        status = "Group "+gid+" does not exist\n";
    }
    else if(find(GROUP_INFO[gid].members.begin(),GROUP_INFO[gid].members.end(),user)==GROUP_INFO[gid].members.end()) {
        status = "You are not a member of group "+gid+"\n";
    }
    else {
        pair<string,string> fg = make_pair(gid,file);
        auto itr = FILE_INFO.find(fg);
        if(itr == FILE_INFO.end()) {
            status = "File "+file+" is not shared in group "+gid+"\n";
        }
        else {
            int portno = stoi(string(port));
            pair<string,int> psock = make_pair(ipadr,portno);
            bool flag = false;
            if(itr->second.seeders.find(psock) != itr->second.seeders.end()) {
                flag = true;
                itr->second.seeders.erase(psock);
            }
            if(itr->second.leechers.find(psock) != itr->second.leechers.end()) {
                flag = true;
                itr->second.leechers.erase(psock);
            }
            if(itr->second.seeders.empty() && itr->second.leechers.empty()) {
                FILE_INFO.erase(itr);
            }
            if(!flag) {
                status = "You do not have file "+file+"\n";
            }
            else {
                status = "File "+file+" is now unshared\n";
            }
        }
    }
    return status;
}

string handle_leave_group(string gid, string user) {

}

string handle_download_file(string gid, string fname, string user, int client_sock) {
    cout << "in handle_download_file" << endl;
    string status;
    if(GROUP_INFO.find(gid)==GROUP_INFO.end()) {
        status = "Group "+gid+" does not exist\n";
    }
    else if(find(GROUP_INFO[gid].members.begin(),GROUP_INFO[gid].members.end(),user)==GROUP_INFO[gid].members.end()) {
        status = "You are not a member of group "+gid+"\n";
    }
    else {
        pair<string, string> fg = make_pair(gid, fname);
        auto itr = FILE_INFO.find(fg);
        if (itr == FILE_INFO.end()) {
            status = "File " + fname + " is not shared in group " + gid + "\n";
        }
        else {
            string sha = "sha";
            string fmsg = to_string(itr->second.totalchunks)+"|"+sha;
            send(client_sock,fmsg.c_str(),fmsg.length(),0);
            fmsg = "";
            vector<string> afpeers;
            for(auto sit=itr->second.seeders.begin();sit!=itr->second.seeders.end();sit++) {
                for (auto apit = ACTIVE_PEERS.begin(); apit != ACTIVE_PEERS.end(); apit++) {
                    if (apit->second.ip == sit->first.first && apit->second.port == sit->first.second) {
                        afpeers.push_back(sit->first.first+":"+to_string(sit->first.second));
                    }
                }
            }
            recv(client_sock,MSG_BUFF,BUFFER_SIZE,0);
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
            fmsg = *afpeers.begin();
            for(auto sit=afpeers.begin();sit!=afpeers.end();sit++) {
                if(sit == afpeers.begin())
                    continue;
                fmsg += "|"+(*sit);
            }
            // send seeder adddresses
//            send(client_sock,fmsg.c_str(),fmsg.length(),0);
            afpeers.clear();
//            fmsg = "";
            for(auto lit=itr->second.leechers.begin();lit!=itr->second.leechers.end();lit++) {
                for (auto apit = ACTIVE_PEERS.begin(); apit != ACTIVE_PEERS.end(); apit++) {
                    if (apit->second.ip == lit->first.first && apit->second.port == lit->first.second) {
                        afpeers.push_back(lit->first.first+":"+to_string(lit->first.second));
                    }
                }
            }
//            recv(client_sock,MSG_BUFF,BUFFER_SIZE,0);
            memset(MSG_BUFF, 0, sizeof(MSG_BUFF));
//            fmsg = *afpeers.begin();
            for(auto lit=afpeers.begin();lit!=afpeers.end();lit++) {
//                if(lit == afpeers.begin())
//                    continue;
                fmsg += "|"+(*lit);
            }
            // send leechers adddresses
            send(client_sock,fmsg.c_str(),fmsg.length(),0);
            afpeers.clear();
            fmsg = "";
            send(client_sock,fmsg.c_str(),fmsg.length(),0);
            status = "File "+fname+" information sent\n";

//            for(auto sit=itr->second.totalchunks)
//            DownRqstStruct drs;
//            drs.uid = user;
//            drs.gid = gid;
//            drs.ip = ipadr;
//            drs.port = portno;
//            drs.srcfile = fname;
//            drs.destp = destp;
//            drs.totchunks = itr->second.totalchunks;
//            DOWN_RQST_Q.push(drs);
//            status = "File "+fname+" will be downloaded soon\n";
        }
    }
    cout << "exiting handle_download_file" << endl;
    return status;
}

string wait_for_download(string fname, string ipadr, int portno) {
    cout << "in wait_for_download" << endl;
    pair<string,pair<string,int>> drfs = make_pair(fname,make_pair(ipadr,portno));
    while(DOWN_RSPN.empty() || DOWN_RSPN.find(drfs) == DOWN_RSPN.end()) {
        continue;
    }
    string status;
    status = DOWN_RSPN[drfs];
    cout << "exiting wait_for_download" << endl;
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
            send_msg = handle_create_user(rcvd_cmd[1], rcvd_cmd[2], rcvd_cmd[3], rcvd_cmd[4]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "login") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_login(rcvd_cmd[1], rcvd_cmd[2]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "logout") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_logout(rcvd_cmd[1]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "create_group") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_create_group(rcvd_cmd[1],rcvd_cmd[2]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "join_group") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_join_group(rcvd_cmd[1],rcvd_cmd[2]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "upload_file") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_upload_file(rcvd_cmd[1],rcvd_cmd[2],rcvd_cmd[3],rcvd_cmd[4],rcvd_cmd[5],rcvd_cmd[6]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "list_groups") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_list_groups(rcvd_cmd[1]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "list_files") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_list_files(rcvd_cmd[1],rcvd_cmd[2]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "stop_share") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_stop_share(rcvd_cmd[1],rcvd_cmd[2],rcvd_cmd[3],rcvd_cmd[4],rcvd_cmd[5]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "leave_group") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_leave_group(rcvd_cmd[1],rcvd_cmd[2]);
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "download_file") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_download_file(rcvd_cmd[1],rcvd_cmd[2],rcvd_cmd[5],client_sock);
//            cout << send_msg << endl;
            send(client_sock,send_msg.c_str(),send_msg.length(),0);
//            send_msg = wait_for_download(rcvd_cmd[2],rcvd_cmd[4],stoi(rcvd_cmd[5]));
//            send(client_sock,send_msg.c_str(),send_msg.length(),0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
        }
        else if(rcvd_cmd[0] == "exit") {
            cout << rcvd_cmd[0] << " request" << endl;
            send_msg = handle_logout(rcvd_cmd[1]);
            send(client_sock,send_msg.c_str(),BUFFER_SIZE,0);
            memset(&MSG_BUFF, 0, sizeof(MSG_BUFF));
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
//    if(pthread_create(&tid_fd, NULL, configDownload, NULL)!= 0) {
//        perror("Failed to create downloader thread\n");
//    }
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

