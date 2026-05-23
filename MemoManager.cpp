#include "MemoManager.h" // 引入对应的头文件，获取类声明
#include <fstream>       // 引入文件输入输出流库（ifstream, ofstream），用于读写本地文件
#include <iostream>      // 引入标准输入输出流库（虽然当前代码未直接使用 std::cout）

using namespace std;     // 使用标准命名空间，省去写 std:: 的麻烦

// ============================================================================
// 构造函数：初始化列表赋值，并从文件加载数据
// ============================================================================
MemoManager::MemoManager(const string& filename) : dataFile(filename) {
    loadFromFile(); // 在对象创建（系统启动）时，自动将本地文件中的历史备忘录加载到内存 map 中
}

// ============================================================================
// 析构函数：在对象销毁时自动安全保存数据
// ============================================================================
MemoManager::~MemoManager() {
    saveToFile();   // 利用 RAII 机制，在程序退出或对象销毁时，自动将内存数据同步回本地文件
}

// ============================================================================
// 辅助函数：将 int 类型的年月日转换为统一的 "YYYY-M-D" 字符串键值
// ============================================================================
string MemoManager::formatDate(int year, int month, int day) const {
    // 例如：输入 2026, 5, 23 -> 返回字符串 "2026-5-23"
    return to_string(year) + "-" + to_string(month) + "-" + to_string(day);
}

// ============================================================================
// 添加/追加备忘录内容
// ============================================================================
void MemoManager::addMemo(int year, int month, int day, const string& content) {
    string key = formatDate(year, month, day); // 生成当前日期的唯一字符串 key

    // memos.find(key) 用于在 map 中查找该日期是否已经记录过备忘录
    if (memos.find(key) != memos.end()) {
        // 如果 find 返回的迭代器不等于 end()，说明该日期已存在备忘录
        memos[key] += "；" + content; // 用分号“；”隔开，将新内容追加到原本的内容后面
    } else {
        // 如果不存在，则直接在 map 中新建一个键值对
        memos[key] = content; 
    }
    
    saveToFile(); // 每次修改后即时写入本地文件，防止程序意外崩溃或断电导致内存数据丢失
}

// ============================================================================
// 获取特定日期的备忘录内容
// ============================================================================
string MemoManager::getMemo(int year, int month, int day) const {
    string key = formatDate(year, month, day); // 生成日期 key
    auto it = memos.find(key);                  // 在 map 中查找该 key

    if (it != memos.end()) {
        return it->second; // 如果找到了，返回迭代器指向的 value（即备忘录具体文本内容）
    }
    return "今日无备忘录。"; // 如果没找到，返回默认的友好提示字符串
}

// ============================================================================
// 判断某一天是否存在备忘录
// ============================================================================
bool MemoManager::hasMemo(int year, int month, int day) const {
    string key = formatDate(year, month, day); // 生成日期 key
    // 如果 find 结果不等于 end()，说明存在，返回 true；反之不存在则返回 false
    return memos.find(key) != memos.end();
}

// ============================================================================
// 删除特定日期的备忘录
// ============================================================================
void MemoManager::deleteMemo(int year, int month, int day) {
    string key = formatDate(year, month, day); // 生成日期 key
    memos.erase(key);                           // 从 map 容器中彻底移除该键值对
    saveToFile();                               // 同步更新到本地文件
}

// ============================================================================
// 从本地磁盘文件中读取并解析数据（格式：日期|内容）
// ============================================================================
void MemoManager::loadFromFile() {
    ifstream inFile(dataFile); // 创建文件输入流，尝试打开绑定的文件名
    if (!inFile.is_open()) return; // 如果文件不存在（如第一次运行程序），直接退出函数，不报错

    string line;
    // 逐行读取文件内容，直到文件末尾
    while (getline(inFile, line)) {
        // 在当前行中寻找分隔符 '|' 的索引位置
        size_t pos = line.find('|');
        
        // 如果找到了分隔符 (string::npos 表示没找到)
        if (pos != string::npos) {
            // 截取分隔符前面的部分作为日期字符串 (从索引0开始，截取长度为 pos)
            string dateStr = line.substr(0, pos); 
            // 截取分隔符后面的部分作为备忘录内容 (从索引 pos + 1 开始直到行尾)
            string content = line.substr(pos + 1); 
            
            memos[dateStr] = content; // 将解析出的键值对存入内存 map 容器中
        }
    }
    inFile.close(); // 读取完毕，主动关闭文件输入流
}

// ============================================================================
// 将内存中的所有备忘录数据序列化持久化到本地文件
// ============================================================================
void MemoManager::saveToFile() const {
    ofstream outFile(dataFile); // 创建文件输出流，以覆盖写入的方式打开/创建文件
    if (!outFile.is_open()) return; // 如果文件打开失败（例如权限问题），直接返回

    // 基于范围的 for 循环，遍历内存中红黑树 map 里的每一个键值对（pair）
    for (const auto& pair : memos) {
        // 按照“日期|内容\n”的固定格式写入文件（例如：2026-5-23|去超市买牛奶\n）
        outFile << pair.first << "|" << pair.second << "\n";
    }
    outFile.close(); // 写入完毕，主动关闭文件输出流
}
