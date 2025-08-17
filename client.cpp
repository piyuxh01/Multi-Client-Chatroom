#include <bits/stdc++.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include <csignal>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

#define NUM_COLORS 6

SOCKET client_socket;
atomic<bool> exit_flag(false);
thread t_send, t_recv;
string colors[] = {"\033[31m","\033[32m","\033[33m","\033[34m","\033[35m","\033[36m"};
string def_col = "\033[0m";
unordered_map<string,int> user_colors;

string color(int code) { return colors[code % NUM_COLORS]; }
void eraseText(int cnt) { for(int i=0;i<cnt;i++) cout << "\b"; }

string current_time() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm *ltm = localtime(&t);
    stringstream ss;
    ss << setw(2) << setfill('0') << ltm->tm_hour << ":"
       << setw(2) << setfill('0') << ltm->tm_min << ":"
       << setw(2) << setfill('0') << ltm->tm_sec;
    return ss.str();
}

void print_prompt() {
    cout << colors[1] << "You : " << def_col;
    fflush(stdout);
}

void send_message() {
    while(!exit_flag) {
        print_prompt();
        string msg;
        getline(cin,msg);

        int len = msg.size();
        send(client_socket,(char*)&len,sizeof(len),0);
        send(client_socket,msg.c_str(),len,0);

        if(msg == "#exit") {
            exit_flag = true;
            t_recv.detach();
            closesocket(client_socket);
            return;
        }
    }
}

void recv_message() {
    while(!exit_flag) {
        int len_name;
        if(recv(client_socket,(char*)&len_name,sizeof(len_name),0) <= 0) continue;
        char name[len_name+1];
        recv(client_socket,name,len_name,0);
        name[len_name]='\0';

        int color_code;
        recv(client_socket,(char*)&color_code,sizeof(color_code),0);

        int len_msg;
        recv(client_socket,(char*)&len_msg,sizeof(len_msg),0);
        char msg[len_msg+1];
        recv(client_socket,msg,len_msg,0);
        msg[len_msg]='\0';

        string username(name);
        if(username != "#NULL") user_colors[username] = color_code;

        cout << "\r\33[K";

        if(username != "#NULL")
            cout << "[" << current_time() << "] " << color(user_colors[username]) << username
                 << " : " << def_col << msg << endl;
        else
            cout << "[" << current_time() << "] " << color(color_code) << msg << def_col << endl;

        print_prompt();
    }
}

void catch_ctrl_c(int signal) {
    string msg = "#exit";
    int len = msg.size();
    send(client_socket,(char*)&len,sizeof(len),0);
    send(client_socket,msg.c_str(),len,0);
    exit_flag = true;
    t_send.detach();
    t_recv.detach();
    closesocket(client_socket);
    exit(signal);
}

int main() {
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0) { cerr<<"WSAStartup failed"<<endl; return 1; }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == INVALID_SOCKET) { cerr<<"Socket creation failed"<<endl; return 1; }

    string server_ip = "127.0.0.1";
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(10000);
    server.sin_addr.s_addr = inet_addr(server_ip.c_str());
    memset(&server.sin_zero,0,sizeof(server.sin_zero));

    if(connect(client_socket,(sockaddr*)&server,sizeof(server)) < 0) { cerr<<"Connection failed"<<endl; return 1; }

    signal(SIGINT, catch_ctrl_c);

    cout << "Enter your name: ";
    string name;
    getline(cin,name);
    int len_name = name.size();
    send(client_socket,(char*)&len_name,sizeof(len_name),0);
    send(client_socket,name.c_str(),len_name,0);

    cout << colors[NUM_COLORS-1] << "\n\t  ====== Welcome to the chat-room ======   " << def_col << endl;

    t_send = thread(send_message);
    t_recv = thread(recv_message);

    if(t_send.joinable()) t_send.join();
    if(t_recv.joinable()) t_recv.join();

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
