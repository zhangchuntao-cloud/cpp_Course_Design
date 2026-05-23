#ifndef MEMO_MANAGER_H    // 【头文件保护】如果当前编译单元未定义 MEMO_MANAGER_H 宏
#define MEMO_MANAGER_H    // 则定义该宏，防止当前头文件被重复包含（导致类重复定义错误）

#include <string>         // 引入标准字符串库，用于处理文件名、备忘录文本内容以及格式化的日期键值
#include <map>            // 引入标准关联容器 map 库，用于在内存中高效存储和检索“日期-备忘录”键值对

// 备忘录管理类：负责在内存中维护备忘录，并提供与本地磁盘文件同步（读写）的功能
class MemoManager {
public:
    /**
     * @brief 构造函数
     * @param filename 需要绑定的本地数据文件名（例如 "memos.txt"）
     * @note 在创建对象时触发，通常会在内部调用 loadFromFile() 自动将历史数据从磁盘加载到内存中
     */
    MemoManager(const std::string& filename);

    /**
     * @brief 析构函数
     * @note 在对象生命周期结束（如程序退出、离开作用域）时自动调用。
     * 利用 C++ 的 RAII (资源获取即初始化) 机制，在此处调用 saveToFile() 确保内存中的新数据自动安全持久化到磁盘。
     */
    ~MemoManager();

    /**
     * @brief 添加或覆盖指定日期的备忘录内容
     * @param year 年份 (如 2026)
     * @param month 月份 (1-12)
     * @param day 日期 (1-31)
     * @param content 备忘录的具体文本内容
     */
    void addMemo(int year, int month, int day, const std::string& content);

    /**
     * @brief 获取特定日期的备忘录内容
     * @param year/month/day 年/月/日
     * @return std::string 返回对应日期的备忘录文本；若不存在则返回空字符串
     * @note 末尾的 const 表示该函数为“常成员函数”，承诺绝不修改类中的任何数据成员
     */
    std::string getMemo(int year, int month, int day) const;

    /**
     * @brief 判断某一天是否存在备忘录
     * @param year/month/day 年/月/日
     * @return true 存在备忘录 / false 不存在备忘录
     * @note 常用于万年历 UI 绘制时判断是否需要在对应的日期格子上标记“*”号或特殊颜色
     */
    bool hasMemo(int year, int month, int day) const;

    /**
     * @brief 删除指定日期的备忘录
     * @param year/month/day 年/月/日
     */
    void deleteMemo(int year, int month, int day);

private:
    std::string dataFile; // 【私有成员变量】存储绑定的本地数据文件路径/文件名

    /**
     * @brief 【内存核心数据结构】存储所有备忘录的关联容器
     * Key (键):   格式化后的日期字符串，如 "2026-5-23"
     * Value (值): 该日期对应的备忘录文本内容
     * @note  std::map 底层是红黑树，能够提供 O(log N) 时间复杂度的快速查找、插入和删除
     */
    std::map<std::string, std::string> memos;

    /**
     * @brief 内部辅助工具函数：将整型的年月日转换为统一的字符串格式
     * @param year/month/day 年/月/日
     * @return 返回形如 "YYYY-M-D" 的标准字符串（例如输入 2026, 5, 23 -> 返回 "2026-5-23"）
     * @note 作为 map 的唯一 Key 值，确保日期格式的一致性
     */
    std::string formatDate(int year, int month, int day) const;

    /**
     * @brief 文件加载函数（私有）
     * @note 从 dataFile 指定的文件中读取磁盘数据，解析并填充到 memos 容器中。通常在构造函数中被调用。
     */
    void loadFromFile();

    /**
     * @brief 文件保存函数（私有）
     * @note 将当前内存 memos 容器中的所有数据按照特定格式序列化写入到 dataFile 文件中。通常在析构函数中被调用。
     */
    void saveToFile() const;
};

#endif // MEMO_MANAGER_H // 对应第一行的 #ifndef，结束头文件保护块
