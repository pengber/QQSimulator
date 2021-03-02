#pragma warning(disable:4996) 
#include "stdio.h"
#include "winsock2.h"
#include "stdlib.h"
#include <iostream>
#include "windows.h"
#include <string>
#include "MySQLManager.h"
#include "D:\MyCode\QT\TCP_CSDN_2_client\TCP_CSDN_2_client\util.h"
#define MAXCLIENTS 1000
MySQLManager mysql;
SOCKET sockets[MAXCLIENTS];
SOCKET socketCount[MAXCLIENTS];
#pragma comment(lib,"ws2_32")
/*
    多线程
            SOCKET
*/

DWORD WINAPI ProcessClientReceive(LPVOID lpParam)
{
    SOCKET* clientsocket = (SOCKET*)lpParam;
    char data[MAXBYTE] = { 0 };     //接受的客户端消息
    char mess[MAXBYTE] = { 0 };     //发给客户端消息
    string strSql;
    char chSql[MAXBYTE];
    int from_id;
    int to_id;
    string time;
    string content;
    while (true) {
        recv(*clientsocket, data, MAXBYTE, NULL);

        string temp(data);
        vector<string> result_vector = split3(temp, ';');
        map<string, string> result = get_map(result_vector);

        if (result["init_mess"] == "1") {//如果是初始消息
            auto iter = result.find("id");
            if (iter != result.end()) {
                //如果是登入，则
                string tempSql = "select * from user where " + iter->first + "=" + addFix(iter->second) + " and " + "pass_word=" + addFix(result["pass_word"]);
                char sql0[MAXBYTE];
                str_to_ch(tempSql, sql0);
                MYSQL_RES* res = mysql.QueryData(sql0);
                if (res->row_count == 0) {
                    //如果没有此用户或者密码错误
                    string temp = "status=0;error=Your count or password is wrong!";
                    str_to_ch(temp, mess);
                    send(*clientsocket, mess, strlen(mess) + sizeof(char), NULL);
                }
                else {
                    //登入成功
                    //更改状态
                    strSql = "UPDATE user SET if_online=1 WHERE id=" + result["id"];
                    str_to_ch(strSql, chSql);
                    mysql.QueryData(chSql);
                    //socket和id对应起来
                    char chId[20];
                    str_to_ch(result["id"], chId);
                    sockets[atoi(chId)] = *clientsocket;

                    string temp = "status=1;tip=log in success";
                    str_to_ch(temp, mess);
                    send(*clientsocket, mess, strlen(mess) + sizeof(char), NULL);

                }
            }

            iter = result.find("nick_name");
            if (iter != result.end()) {
                //如果是注册，这里不允许昵称相同，以后加健壮
                strSql = "INSERT INTO user (nick_name, pass_word,time) VALUES(" + addFix(result["nick_name"]) + "," + addFix(result["pass_word"])+","+addFix(result["time"]) + ")";
                cout << strSql;
                str_to_ch(strSql, chSql);
                mysql.QueryData(chSql);

                strSql = "SELECT * FROM USER WHERE nick_name=" + addFix(result["nick_name"]) + " and pass_word=" + addFix(result["pass_word"]) + " and time="+addFix(result["time"]);
                str_to_ch(strSql, chSql);
                MYSQL_RES* res = mysql.QueryData(chSql);
                MYSQL_ROW row = mysql_fetch_row(res);
                string strId;
                ch_to_str(row[0], strId);
                cout << row[0];
                
                string temp = "init_mess=1;status=1;tip=sign up success,your id is" + strId + ",please remember it!";
                str_to_ch(temp, mess);
                send(*clientsocket, mess, strlen(mess) + sizeof(char), NULL);
            }
        }
        else if (result["init_mess"] == "2") {//如果是通信消息
            if (result["content"] == "exit") {
                //如果是客户端退出的话

                //修改状态
                strSql = "UPDATE user SET if_online=0 WHERE id = " + result["from_id"];
                str_to_ch(strSql, chSql);
                mysql.QueryData(chSql);
               
                //结束线程
                ExitThread(0);
                break;
            }
            strSql = "INSERT INTO content(from_id, to_id, time, content) VALUES(" + result["from_id"] + "," + result["to_id"] + "," + addFix(result["time"]) + "," + addFix(result["content"]) + ")";
            str_to_ch(strSql, chSql);
            mysql.QueryData(chSql);
            

            strSql = "SELECT * FROM user WHERE id=" + result["to_id"];
            str_to_ch(strSql, chSql);
            MYSQL_RES* res = mysql.QueryData(chSql);
            MYSQL_ROW row = mysql_fetch_row(res);
            int if_online = atoi(row[3]);
            if (if_online == 1) {
                //to_id在线
                //发给to_id那个socket
                send(sockets[atoi(row[2])], data, strlen(data) + sizeof(char), NULL);
                strSql = "UPDATE content SET if_saw=1 WHERE from_id=" + result["from_id"] + " and to_id=" + result["to_id"] + " and content=" + addFix(result["content"]) + " and time=" + addFix(result["time"]);
                str_to_ch(strSql, chSql);
                mysql.QueryData(chSql);
            }
            else {
                //不在线的话就不管，接受下一条消息
                std::cout << result["to_id"] << "is'not online,store in database" << endl;
            }
        }
        else if (result["init_mess"] == "3") {//如果是接受旧消息，则to_id必定在线

            strSql = "SELECT * FROM content WHERE from_id=" + result["to_id"] + " and to_id=" + result["from_id"] + " and if_saw=0";
            str_to_ch(strSql, chSql);
            MYSQL_RES* res = mysql.QueryData(chSql);
            MYSQL_ROW row;
            while (row = mysql_fetch_row(res)) {
                string temp = "init_mess=2;from_id=" + result["to_id"] + ";to_id=" + result["from_id"] + ";time=";
                string temptime;
                ch_to_str(row[3], temptime);
                temp += temptime + ";content=";
                string tempContent;
                ch_to_str(row[4], tempContent);
                temp += tempContent;
                str_to_ch(temp, mess);
                send(*clientsocket, mess, strlen(mess) + sizeof(char), NULL);
            }
            strSql = "UPDATE content SET if_saw=1 WHERE from_id="+result["to_id"] + " and to_id=" + result["from_id"] + " and if_saw=0";
            str_to_ch(strSql, chSql);
            mysql.QueryData(chSql);
        }
           
    }
    
    /*char* mymsg = new char[10000];
    while (TRUE)
    {
       
        recv(*clientsocket, buffer, MAXBYTE, NULL);
        if (strcmp(buffer, "exit") == 0)
        {
            char* exit_msg = (char*)("exit");
            send(*clientsocket, exit_msg, strlen(exit_msg) + sizeof(char), NULL);
            break;
        }
        printf("--- Sys: %s--\n", buffer);
    }
    */
    closesocket(*clientsocket);
    return 0;
}
DWORD WINAPI ProcessClientSend(LPVOID lpParam)
{
    SOCKET* clientsocket = (SOCKET*)lpParam;
    char* mymsg = new char[10000];
    while (TRUE)
    {
        std::cin >> mymsg;
        send(*clientsocket, mymsg, strlen(mymsg) + sizeof(char), NULL);
    }
    closesocket(*clientsocket);
    return 0;
}
int main()
{
 
    ConnectionInfo info;
    // 填充ConnectionInfo这个结构体，项目中一般从配置文件这读取
    info.user = "root";
    info.password = "20010502";
    info.host = "localhost";
    info.port = 3306;
    info.database = "tim";
    info.unix_socket = NULL;
    info.clientflag = 0;

    // mysql连接
    if (!mysql.Init(info))
    {
        return -1;
    }

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in sockaddr;
    sockaddr.sin_family = PF_INET;
    sockaddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    sockaddr.sin_port = htons(9000);
    bind(s, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
    listen(s, 1);
    printf("listening on port[%d].\n", 9000);
    char* msg = new char[1000];

    HANDLE receivethreads[MAXCLIENTS];    //线程存放 数组
    HANDLE sendThreads[MAXCLIENTS];
    int ReceiveCountClient = 0;
    int SendCountClient = 0;
    while (TRUE)
    {
        SOCKADDR clientAddr;
        int size = sizeof(SOCKADDR);
        SOCKET clientsocket;
        clientsocket = accept(s, &clientAddr, &size);        //阻塞模式 直到有新的Tcp 接入
        
        printf("Sys: New client touched ID is %d .\n", ReceiveCountClient + 1);            //
        if (ReceiveCountClient < MAXCLIENTS)                        //创建新线程
        {
            socketCount[ReceiveCountClient] = clientsocket;
            receivethreads[ReceiveCountClient] = CreateThread(NULL, 0, &ProcessClientReceive, &socketCount[ReceiveCountClient], 0, NULL);
            ReceiveCountClient++;
            //sendThreads[SendCountClient++] = CreateThread(NULL, 0, &ProcessClientSend, &clientsocket, 0, NULL);
        }
        else                                                //线程数超了 拒绝服务
        {
            char* msg = (char*)(" Error Too many client Connecttion  !.\r\n");
            send(clientsocket, msg, strlen(msg) + sizeof(char), NULL);
            printf(" ** SYS **  REFUSED !.\n");
            closesocket(clientsocket);
        }
    }

    printf("Maximize clients occurred for d%.\r\n", MAXCLIENTS);
    WaitForMultipleObjects(MAXCLIENTS, receivethreads, TRUE, INFINITE);
    WaitForMultipleObjects(MAXCLIENTS, sendThreads, TRUE, INFINITE);
    closesocket(s); //关闭socket

    for (int i = 0; i < MAXCLIENTS; i++)
    {
        CloseHandle(receivethreads[i]);
        CloseHandle(sendThreads[i]);
    }

    WSACleanup();
    getchar();
    exit(0);
    return 0;
}