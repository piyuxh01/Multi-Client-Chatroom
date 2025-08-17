#include <bits/stdc++.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <chrono>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define NUM_COLORS 6
#define MAX_LEN 512

struct Client {
    int id;
    string name;
    SOCKET socket;
    thread th;
};

vector<Client> clients;
unordered_map<string,int> name_color_map;
mutex clients_mtx, cout_mtx;
string colors[] = {"\033[31m","\033[32m","\033[33m","\033[34m","\033[35m","\033[36m"};
string def_col = "\033[0m";
int seed = 0;

string color(int code) { return colors[code % NUM_COLORS]; }
int get_color(const string &name) {
    if(name_color_map.find(name) != name_color_map.end()) return name_color_map[name];
    int color_code = name_color_map.size() % NUM_COLORS;
    name_color_map[name] = color_code;
    return color_code;
}

string current_time() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm *ltm = localtime(&t);
    char buf[16];
    sprintf(buf,"%02d:%02d:%02d",ltm->tm_hour,ltm->tm_min,ltm->tm_sec);
    return string(buf);
}

void shared_print(const string &str) {
    lock_guard<mutex> guard(cout_mtx);
    cout << str << endl;
}

void set_name(int id, const char name[]) {
    lock_guard<mutex> guard(clients_mtx);
    for(auto &c : clients)
        if(c.id == id) c.name = name;
}

void broadcast_message(const string &msg, int sender_id) {
    lock_guard<mutex> guard(clients_mtx);
    int len = msg.size();
    for(auto &c : clients) {
        if(c.id != sender_id) {
            send(c.socket,(char*)&len,sizeof(len),0);
            send(c.socket,msg.c_str(),len,0);
        }
    }
}

void broadcast_message(int num, int sender_id) {
    lock_guard<mutex> guard(clients_mtx);
    for(auto &c : clients)
        if(c.id != sender_id)
            send(c.socket,(char*)&num,sizeof(num),0);
}

void end_connection(int id) {
    lock_guard<mutex> guard(clients_mtx);
    for(int i=0;i<clients.size();i++) {
        if(clients[i].id == id) {
            closesocket(clients[i].socket);
            clients[i].th.detach();
            clients.erase(clients.begin()+i);
            break;
        }
    }
}

void handle_client(SOCKET client_socket, int id) {
    int len_name;
    recv(client_socket,(char*)&len_name,sizeof(len_name),0);
    char name[len_name+1];
    recv(client_socket,name,len_name,0);
    name[len_name]='\0';

    set_name(id,name);
    int c_color = get_color(name);

    string welcome = string(name) + " has joined 🌟";
    broadcast_message("#NULL",id);
    broadcast_message(id,id);
    broadcast_message(welcome,id);
    shared_print(color(c_color)+welcome+def_col);

    while(true) {
        int len_msg;
        int bytes = recv(client_socket,(char*)&len_msg,sizeof(len_msg),0);
        if(bytes <= 0) { end_connection(id); return; }

        char msg[len_msg+1];
        recv(client_socket,msg,len_msg,0);
        msg[len_msg]='\0';

        if(strcmp(msg,"#exit")==0) {
            string leave = string(name)+" has left 💨";
            broadcast_message("#NULL",id);
            broadcast_message(id,id);
            broadcast_message(leave,id);
            shared_print(color(c_color)+leave+def_col);
            end_connection(id);
            return;
        }

        broadcast_message(string(name),id);
        broadcast_message(id,id);
        broadcast_message(string(msg),id);
        shared_print(color(c_color)+string(name)+" : "+def_col+msg);
    }
}

int main() {
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0) { cerr<<"WSAStartup failed"<<endl; return 1; }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == INVALID_SOCKET) { cerr<<"Socket creation failed"<<endl; return 1; }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(10000);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(&server.sin_zero,0,sizeof(server.sin_zero));

    if(bind(server_socket,(sockaddr*)&server,sizeof(server)) == SOCKET_ERROR) { cerr<<"Bind failed"<<endl; return 1; }
    if(listen(server_socket,8) == SOCKET_ERROR) { cerr<<"Listen failed"<<endl; return 1; }

    cout << colors[NUM_COLORS-1] << "\n\t  ====== Welcome to the chat-room ======   " << def_col << endl;

    while(true) {
        sockaddr_in client_addr;
        int len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket,(sockaddr*)&client_addr,&len);
        if(client_socket == INVALID_SOCKET) { cerr<<"Accept failed"<<endl; continue; }

        seed++;
        thread t(handle_client,client_socket,seed);
        lock_guard<mutex> guard(clients_mtx);
        clients.push_back({seed,"Anonymous",client_socket,move(t)});
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
