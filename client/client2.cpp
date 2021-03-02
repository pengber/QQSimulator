#pragma warning(disable:4996) 
#pragma comment(lib,"ws2_32")
#include <cstdio>
#include "winsock2.h"
#include "stdlib.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <string>
#include <windows.h>
#include "util.h"   //工具函数库
using namespace std;;
#define MAXCLIENTS 4


DWORD WINAPI ProcessClientReceive(LPVOID lpParam)
{
    SOCKET* clientsocket = (SOCKET*)lpParam;
    char mess[MAXBYTE] = { 0 };
    while (TRUE)
    {

        recv(*clientsocket, mess, MAXBYTE, NULL);
        string str_result;
        ch_to_str(mess, str_result);
        vector<string> vec_result = split3(str_result, ';');
        map<string, string> result = get_map(vec_result);

        if (result["init_mess"] == "2") {
            std::cout << endl;
            std::cout << setw(40) << right << result["content"] << "<<" << result["from_id"] << endl;
            std::cout << ">>" << result["from_id"] << ":";
        }
        else if(result["init_mess"] == "1"){
            //不会用到，用作以后扩展
            if (result["status"] == "0") {
                std::cout << result["error"] << endl;
            }
            else if (result["status"] == "1") {
                std::cout << result["tip"] << endl;
            }
        }
        
    }
    return 0;
}
DWORD WINAPI ProcessClientSend(LPVOID lpParam)
{
    SOCKET* clientsocket = (SOCKET*)lpParam;
    int count_status = 0; //0表示未登陆，1表示已登陆
    int choice;
    int id = -1;
    string nick_name;
    string pass_word;
    int init_mess = 0;         //1表示为登入消息，0表示为通信消息
    char data[MAXBYTE] = { 0 };
    char result[MAXBYTE] = { 0 };
    while (true) {
        std::cout << "Please choose choice,1 to log in,2 to sign up:";
        std::cin >> choice;
        if (choice == 1) {
            std::cout << "Please input the user_id:";
            std::cin >> id;
            std::cout << "Please input the pass_word:";
            std::cin >> pass_word;
            init_mess = 1;
            string temp = "init_mess="+ to_string(init_mess) + ";id=" + to_string(id) + ";pass_word=" + pass_word;
            int length = temp.copy(data, temp.length());
            data[length] = '\0';
            send(*clientsocket, data, strlen(data)+sizeof(char),NULL);
            recv(*clientsocket, result, MAXBYTE, NULL);
            vector<string> result_vector = split3(string(result), ';');
            map<string, string> result_map = get_map(result_vector);
            if (result_map["status"] == "1") {
                //登陆成功
                count_status = 1;
                std::cout << result_map["tip"] << std::endl;
                //收消息线程锁打开
                break;
            }
            else {
                std::cout << result_map["error"] << std::endl;
            }
        }
        else if (choice == 2) {
            std::cout << "Please input the nick_name:";
            std::cin >> nick_name;
            std::cout << "Please input the pass_word:";
            std::cin >> pass_word;
            init_mess = 1;          //表示初始化状态消息
            string temp = "init_mess=" + to_string(init_mess) + ";nick_name=" + nick_name + ";pass_word=" + pass_word + ";time=" + get_time();
            int length = temp.copy(data, temp.length());
            data[length] = '\0';
            send(*clientsocket, data, strlen(data) + sizeof(char), NULL);
            recv(*clientsocket, result, MAXBYTE, NULL);
            vector<string> result_vector = split3(string(result), ';');
            map<string, string> result_map = get_map(result_vector);
            if (result_map["status"] == "1") {
                //注册成功
                std::cout << result_map["tip"] << std::endl;
            }
            else {
                std::cout << result_map["erorr"] << std::endl;
            }
        }
        else {
            std::cout << "Your choice is wrong, please input again" << std::endl;
        }
    }
    char mymsg[MAXBYTE] = { 0 };
    HANDLE receiveHandle = CreateThread(NULL, 0, &ProcessClientReceive, clientsocket, 0, NULL);

    while (true) {
        int to_id;
        std:cout << "Please input the id you want to connect, 0 to quit:";
        std::cin >> to_id;
        if (to_id == 0) {
            //
            string temp = "init_mess=" + to_string(init_mess) + ";from_id=" + to_string(id) + ";to_id=" + to_string(to_id) + ";time=" + get_time() + ";content=exit";
            str_to_ch(temp, data);
            send(*clientsocket, data, strlen(data) + sizeof(char), NULL);//发送过去，让服务器修改在线状态
            break;
        }
        init_mess = 3;          //表示这是接受旧消息通信包
        string temp = "init_mess=" + to_string(init_mess) + ";from_id=" + to_string(id) + ";to_id=" + to_string(to_id);
        str_to_ch(temp, data);
        send(*clientsocket, data, strlen(data) + sizeof(char), NULL);
        Sleep(100);//等待接收消息线程接受之前的旧消息
        while (TRUE)
        {
            std::cout << ">>" << to_id << ":";
            cin >> mymsg;
            init_mess = 2;          //表示这是通信包
            string temp = "init_mess=" + to_string(init_mess) + ";from_id=" + to_string(id)+";to_id="+to_string(to_id)+";time="+get_time()+";content="+mymsg;
            str_to_ch(temp, data);
            if (strcmp(mymsg, "exit") == 0) {
                
                break;
            }
            send(*clientsocket, data, strlen(data) + sizeof(char), NULL);
        }
    }
    
    //WaitForSingleObject(receiveHandle, INFINITE);发送线程结束，直接关闭接受线程0
    //这里应该手动结束线程,要不还会receiveHandle还会执行一段时间

    return 0;
}
void main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in sockaddr;
    sockaddr.sin_family = PF_INET;
    sockaddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    sockaddr.sin_port = htons(9000);
 
    connect(s, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));

    HANDLE threads[MAXCLIENTS];    //线程存放 数组
    int CountClient = 0;
    
    //HANDLE receive = CreateThread(NULL, 0, &ProcessClientReceive, &s, 0, NULL);
    HANDLE send = CreateThread(NULL, 0, &ProcessClientSend, &s, 0, NULL);
    
    WaitForSingleObject(send, INFINITE);
    closesocket(s);
    WSACleanup();
    getchar();
    exit(0);

}