#include "MemoManager.h"
#include <fstream>
#include <iostream>

using namespace std;

MemoManager::MemoManager(const string& filename) : dataFile(filename) {
    loadFromFile(); // 启动时自动加载数据
}

MemoManager::~MemoManager() {
    saveToFile();   // 销毁时自动保存数据
}

string MemoManager::formatDate(int year, int month, int day) const {
    return to_string(year) + "-" + to_string(month) + "-" + to_string(day);
}

void MemoManager::addMemo(int year, int month, int day, const string& content) {
    string key = formatDate(year, month, day);
    // 如果已经存在，则追加内容；如果不存在，则新建
    if (memos.find(key) != memos.end()) {
        memos[key] += "；" + content;
    } else {
        memos[key] = content;
    }
    saveToFile(); // 每次修改后即时保存，防止意外断电丢失
}

string MemoManager::getMemo(int year, int month, int day) const {
    string key = formatDate(year, month, day);
    auto it = memos.find(key);
    if (it != memos.end()) {
        return it->second;
    }
    return "今日无备忘录。";
}

bool MemoManager::hasMemo(int year, int month, int day) const {
    string key = formatDate(year, month, day);
    return memos.find(key) != memos.end();
}

void MemoManager::deleteMemo(int year, int month, int day) {
    string key = formatDate(year, month, day);
    memos.erase(key);
    saveToFile();
}

void MemoManager::loadFromFile() {
    ifstream inFile(dataFile);
    if (!inFile.is_open()) return; // 文件不存在则跳过

    string line;
    while (getline(inFile, line)) {
        // 寻找分隔符 '|'
        size_t pos = line.find('|');
        if (pos != string::npos) {
            string dateStr = line.substr(0, pos);
            string content = line.substr(pos + 1);
            memos[dateStr] = content;
        }
    }
    inFile.close();
}

void MemoManager::saveToFile() const {
    ofstream outFile(dataFile);
    if (!outFile.is_open()) return;

    for (const auto& pair : memos) {
        // 格式：日期|内容
        outFile << pair.first << "|" << pair.second << "\n";
    }
    outFile.close();
}