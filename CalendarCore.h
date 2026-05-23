#ifndef CALENDAR_CORE_H
#define CALENDAR_CORE_H

class CalendarCore {
public:
    // 构造函数，初始化目标年份和月份
    CalendarCore(int targetYear, int targetMonth);

    // 获取当前设定的年份
    int getYear() const;

    // 获取当前设定的月份
    int getMonth() const;

    // 判断当前设定的年份是否为闰年
    bool isLeapYear() const;

    // 获取当前设定的月份有多少天 (28, 29, 30 或 31)
    int getDaysInMonth() const;

    // 核心算法：获取当前月份的 1号 是星期几 (0代表星期日，1-6代表星期一至六)
    int getFirstDayOfWeek() const;

    // 更新当前的年月 (用于后续的翻页功能)
    void updateDate(int newYear, int newMonth);

private:
    int year;
    int month;
};

#endif // CALENDAR_CORE_H