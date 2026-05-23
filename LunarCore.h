#ifndef LUNAR_CORE_H
#define LUNAR_CORE_H

#include <string>

class LunarCore {
public:
    // 主接口：输入公历年月日，返回农历日期字符串（如 "五月初五", "闰四月廿十"）
    static std::string getLunarString(int year, int month, int day);

private:
    // 1900 - 2099 年的农历数据表
    static const int lunarInfo[200];

    // 位运算提取函数
    static int getYearDays(int y);           // 返回农历 y 年的总天数
    static int leapMonth(int y);             // 返回农历 y 年闰哪个月 (1-12)，没闰返回 0
    static int leapDays(int y);              // 返回农历 y 年闰月的天数 (0, 29, 30)
    static int monthDays(int y, int m);      // 返回农历 y 年 m 月的平月天数 (29, 30)

    // 公历日期辅助计算
    static bool isGregorianLeap(int year);
    static int getGregorianDays(int year, int month);
    // 计算从 1900年1月31日 (农历1900年正月初一) 到目标公历日期的总天数差
    static int getOffsetDays(int year, int month, int day);

    // 格式化输出辅助
    static std::string getLunarDayString(int day);
    static std::string getLunarMonthString(int month, bool isLeap);
};

#endif // LUNAR_CORE_H