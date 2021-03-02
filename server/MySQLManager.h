/*MySQLManager.h 文件：
    文件名:        MySQLManager.h
    内  容:        MySQL连接数据库管理类
    创建日期:    2016年10月18日
    创建人:        AceTan
*/

#pragma once

// 网络通信头文件
#include <WinSock.h>

// 引入mysql头文件(比较好的做法是把文件夹拷到工程目录，也可以在vc目录里面设置)
#include "mysql.h"

#include <Windows.h>
using namespace std;
// 包含附加依赖项，也可以在工程--属性里面设置
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "libmysql.lib")

// 连接数据库的一些必要信息
struct ConnectionInfo
{
    const char* host;            // 主机地址
    const char* user;            // 用户名
    const char* password;        // 密码
    const char* database;        // 数据库名
    unsigned int port;            // 端口号
    const char* unix_socket;    // unix连接标识
    unsigned long clientflag;    // 客户端连接标志

    // 构造函数，设置一些默认值
    ConnectionInfo() :
        host("127.0.0.1"),
        port(3306),
        unix_socket(NULL),
        clientflag(0)
    {

    }
};

class MySQLManager
{
public:

    // 连接数据库
    bool Init(ConnectionInfo& info);

    // 释放连接
    bool FreeConnect();

    // 增加数据
    // bool InsertData(const char* sql);

    // 删除数据
    // bool DeleteData(const char* sql);

    // 更新数据
    // bool UpdateData(const char* sql);

    // 执行sql语句, 包括增加、删除、更新数据
    bool ExecuteSql(const char* sql);

    // 查询数据
    MYSQL_RES* QueryData(const char* sql);
    //得到行数据
    MYSQL_ROW get_ROW();
    int get_col();
    // 打印结果集
    void PrintQueryRes();

private:
    MYSQL m_mysql;                // mysql连接
    MYSQL_RES* m_res;            // 查询结果集

};

// 连接数据库
bool MySQLManager::Init(ConnectionInfo& info)
{
    // 初始化mysql,连接mysql，数据库
    mysql_init(&m_mysql);

    // 连接失败
    if (!(mysql_real_connect(&m_mysql, info.host, info.user, info.password, info.database, info.port, info.unix_socket, info.clientflag)))
    {
        return false;
    }

    return true;
}

// 释放连接
bool MySQLManager::FreeConnect()
{
    //释放资源
    mysql_free_result(m_res);
    mysql_close(&m_mysql);

    return false;
}

// 执行sql语句, 包括增加、删除、更新数据
bool MySQLManager::ExecuteSql(const char* sql)
{
    if (mysql_query(&m_mysql, sql))
    {
        // 打错误log，这里直接显示到控制台
        cerr << "erro:" << mysql_error(&m_mysql) << endl;
        return false;
    }
    else
    {
        cout << "sucess!" << endl;
    }

    return true;
}

// 查询数据
MYSQL_RES* MySQLManager::QueryData(const char* sql)
{
    if (mysql_query(&m_mysql, sql))
    {
        // 打错误log，这里直接显示到控制台
        cerr << "error:" << mysql_error(&m_mysql) << endl;
        return nullptr;
    }
    else
    {
        cout << "select sucess!" << endl;
    }

    // 存储查询结果
    m_res = mysql_store_result(&m_mysql);

    return m_res;
}

// 遍历结果集
void MySQLManager::PrintQueryRes()
{
    if (nullptr == m_res || NULL == m_res)
    {
        return;
    }

    // 获取行数
    // unsigned int rows = mysql_affected_rows(m_mysql);

    // 字段列数组
    MYSQL_FIELD* field = nullptr;
    //存字段名二维数组
    char fieldName[64][32];

    // 获取字段名
    for (int i = 0; field = mysql_fetch_field(m_res); ++i)
    {
        strcpy_s(fieldName[i], field->name);
    }

    // 获取列数
    int columns = mysql_num_fields(m_res);
    for (int i = 0; i < columns; ++i)
    {
        // 使用C语言的printf格式化更方便一点
        printf("%10s\t", fieldName[i]);
    }
    cout << endl;

    MYSQL_ROW row;
    while (row = mysql_fetch_row(m_res))
    {
        for (int i = 0; i < columns; ++i)
        {
            printf("%10s\t", row[i]);
        }

        cout << endl;
    }

}
int MySQLManager::get_col() {
    return mysql_num_fields(m_res);
}
MYSQL_ROW MySQLManager::get_ROW() {
    MYSQL_ROW row;
    int columns = mysql_num_fields(m_res);
    row = mysql_fetch_row(m_res);
    return row;
}