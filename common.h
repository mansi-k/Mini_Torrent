#include <iostream>
#include <bits/stdc++.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cmath>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define QLIMIT 32
//#define CHUNK_SIZE 1024*512
#define CHUNK_SIZE 1024

using namespace std;

pair<string,int> split_address(string addr) {
    int pos = addr.find(":");
    pair<string,int> sock;
    sock.first = addr.substr(0,pos);
    sock.second = stoi(addr.substr(pos+1,addr.length()));
    return sock;
}

vector<string> split_string(string s,char d) {
    vector<string> v;
    if(s=="") {
        return v;
    }
    stringstream ss(s);
    string temp;
    while(getline(ss,temp,d)) {
        v.push_back(temp);
    }
    return v;
}

vector<int> split_bitvector(string s,char d, int totchunks) {
    vector<int> v;
    v.resize(totchunks,0);
    stringstream ss(s);
    string temp;
    while(getline(ss,temp,d)) {
        v[stoi(temp)] = 1;
    }
    return v;
}

string bitvec_toString(vector<int> v) {
    string bitstr = to_string(*v.begin());
    for(auto it=v.begin();it!=v.end();it++) {
        if(it == v.begin())
            continue;
        bitstr += ";"+to_string(*it);
    }
    return bitstr;
}
