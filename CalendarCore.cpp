#include "CalendarCore.h"

CalendarCore::CalendarCore(int targetYear, int targetMonth) {
    year = targetYear;
    month = targetMonth;
}

int CalendarCore::getYear() const { return year; }
int CalendarCore::getMonth() const { return month; }

bool CalendarCore::isLeapYear() const {
    // 闰年规则：四年一闰，百年不闰，四百年再闰
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int CalendarCore::getDaysInMonth() const {
    // 使用数组存储平年各个月份的天数
    int daysPerMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // 如果是 2 月，且是闰年，返回 29 天
    if (month == 2 && isLeapYear()) {
        return 29;
    }
    return daysPerMonth[month];
}

int CalendarCore::getFirstDayOfWeek() const {
    int y = year;
    int m = month;
    int d = 1; // 我们只需要计算1号是星期几

    // 在基姆拉尔森公式中，每年的 1月 和 2月 要当作上一年的 13月 和 14月 来计算
    if (m == 1 || m == 2) {
        m += 12;
        y--;
    }

    // 基姆拉尔森公式核心代码
    // 计算结果 week = 0 表示星期日，1-6 表示星期一至星期六
    int week = (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400 + 1) % 7;
    
    return week;
}

void CalendarCore::updateDate(int newYear, int newMonth) {
    year = newYear;
    month = newMonth;
}