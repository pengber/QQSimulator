#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <cstdio>
#include <ctime>
using namespace std;

vector<string> split3(const string& str, const char pattern)
{
    vector<string> res;
    stringstream input(str);   //读取str到字符串流中
    string temp;
    //使用getline函数从字符串流中读取,遇到分隔符时停止,和从cin中读取类似
    //注意,getline默认是可以读取空格的
    while (getline(input, temp, pattern))
    {
        res.push_back(temp);
    }
    return res;
}

map<string, string> get_map(const vector<string>& result_vector) {
    map <string, string> result;
    for (auto iter = result_vector.begin(); iter != result_vector.end(); iter++) {
        string temp = *iter;
        int index = temp.find_first_of("=");
        string key = temp.substr(0, index);
        string value = temp.substr(index + 1, temp.length() - index - 1);
        result[key] = value;
    }
    return result;
}

string get_time() {
    std::time_t rawtime;
    std::tm* timeinfo;
    char buffer[80];
    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);
    std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    string time(buffer);
    return time;
}

void str_to_ch(string temp, char* ch) {
    int length = temp.copy(ch, temp.length());
    ch[length] = '\0';
}

void ch_to_str(char* ch, string & temp) {
    temp = ch;
}
string addFix(string a) {
    return "'" + a + "'";//数据库字符串
}