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

#define BUFFER_SIZE 1024
#define QLIMIT 32
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
    stringstream ss(s);
    string temp;
    while(getline(ss,temp,d)) {
        v.push_back(temp);
    }
    return v;
}
