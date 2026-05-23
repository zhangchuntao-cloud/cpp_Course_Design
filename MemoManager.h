#ifndef MEMO_MANAGER_H
#define MEMO_MANAGER_H

#include <string>
#include <map>

class MemoManager {
public:
    // 构造函数，传入用于保存数据的文件名
    MemoManager(const std::string& filename);
    
    // 析构函数，在程序退出时自动保存数据
    ~MemoManager();

    // 添加或覆盖备忘录
    void addMemo(int year, int month, int day, const std::string& content);

    // 获取特定日期的备忘录内容
    std::string getMemo(int year, int month, int day) const;

    // 判断某一天是否有备忘录 (用于在日历上画星号)
    bool hasMemo(int year, int month, int day) const;

    // 删除某天的备忘录
    void deleteMemo(int year, int month, int day);

private:
    std::string dataFile;
    std::map<std::string, std::string> memos;

    // 内部工具：将年月日格式化为 "YYYY-M-D" 形式的字符串键值
    std::string formatDate(int year, int month, int day) const;

    // 文件读写操作
    void loadFromFile();
    void saveToFile() const;
};

#endif // MEMO_MANAGER_H