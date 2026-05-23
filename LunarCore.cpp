#include "LunarCore.h"
#include <vector>

using namespace std;

// 1900-2099 农历数据表 (经典 20 位压缩算法)
const int LunarCore::lunarInfo[200] = {
    0x04bd8, 0x04ae0, 0x0a570, 0x054d5, 0x0d260, 0x0d950, 0x16554, 0x056a0, 0x09ad0, 0x055d2,
    0x04ae0, 0x0a5b6, 0x0a4d0, 0x0d250, 0x1d255, 0x0b540, 0x0d6a0, 0x0ada2, 0x095b0, 0x14977,
    0x04970, 0x0a4b0, 0x0b4b5, 0x06a50, 0x06d40, 0x1ab54, 0x02b60, 0x09570, 0x052f2, 0x04970,
    0x06566, 0x0d4a0, 0x0ea50, 0x06e95, 0x05ad0, 0x02b60, 0x186e3, 0x092e0, 0x1c8d7, 0x0c950,
    0x0d4a0, 0x1d8a6, 0x0b550, 0x056a0, 0x1a5b4, 0x025d0, 0x092d0, 0x0d2b2, 0x0a950, 0x0b557,
    0x06ca0, 0x0b550, 0x15355, 0x04da0, 0x0a5b0, 0x14573, 0x052b0, 0x0a9a8, 0x0e950, 0x06aa0,
    0x0aea6, 0x0ab50, 0x04b60, 0x0aae4, 0x0a570, 0x05260, 0x0f263, 0x0d950, 0x05b57, 0x056a0,
    0x096d0, 0x04dd5, 0x04ad0, 0x0a4d0, 0x0d4d4, 0x0d250, 0x0d558, 0x0b540, 0x0b6a0, 0x195a6,
    0x095b0, 0x049b0, 0x0a974, 0x0a4b0, 0x0b27a, 0x06a50, 0x06d40, 0x0af46, 0x0ab60, 0x09570,
    0x04af5, 0x04970, 0x064b0, 0x074a3, 0x0ea50, 0x06b58, 0x05ac0, 0x0ab60, 0x096d5, 0x092e0,
    0x0c960, 0x0d954, 0x0d4a0, 0x0da50, 0x07552, 0x056a0, 0x0abb7, 0x025d0, 0x092d0, 0x0cab5,
    0x0a950, 0x0b4a0, 0x0baa4, 0x0ad50, 0x055d9, 0x04ba0, 0x0a5b0, 0x15176, 0x052b0, 0x0a930,
    0x07954, 0x06aa0, 0x0ad50, 0x05b52, 0x04b60, 0x0a6e6, 0x0a4e0, 0x0d260, 0x0ea65, 0x0d530,
    0x05aa0, 0x076a3, 0x096d0, 0x04bd7, 0x04ad0, 0x0a4d0, 0x1d0b6, 0x0d250, 0x0d520, 0x0dd45,
    0x0b5a0, 0x056d0, 0x055b2, 0x049b0, 0x0a577, 0x0a4b0, 0x0aa50, 0x1b255, 0x06d20, 0x0ada0,
    0x14b63, 0x09370, 0x049f8, 0x04970, 0x064b0, 0x168a6, 0x0ea50, 0x06b20, 0x1a6c4, 0x0aae0,
    0x092e0, 0x0d2e3, 0x0c960, 0x0d557, 0x0d4a0, 0x0da50, 0x05d55, 0x056a0, 0x0a6d0, 0x055d4,
    0x052d0, 0x0a9b8, 0x0a950, 0x0b4a0, 0x0b6a6, 0x0ad50, 0x055a0, 0x0aba4, 0x0a5b0, 0x052b0,
    0x0b273, 0x06930, 0x07337, 0x06aa0, 0x0ad50, 0x14b55, 0x04b60, 0x0a570, 0x054e4, 0x0d160,
    0x0e968, 0x0d520, 0x0daa0, 0x16aa6, 0x056d0, 0x04ae0, 0x0a9d4, 0x0a2d0, 0x0d150, 0x0f252
};

// 传回农历 y年的总天数
int LunarCore::getYearDays(int y) {
    int i, sum = 348; // 12个月 x 29天 = 348天
    for (i = 0x8000; i > 0x8; i >>= 1) {
        if ((lunarInfo[y - 1900] & i) != 0) sum += 1; // 大月加1天
    }
    return sum + leapDays(y);
}

// 传回农历 y年闰月的天数
int LunarCore::leapDays(int y) {
    if (leapMonth(y) != 0) {
        return ((lunarInfo[y - 1900] & 0x10000) != 0) ? 30 : 29;
    }
    return 0;
}

// 传回农历 y年闰哪个月 1-12，没闰传回 0
int LunarCore::leapMonth(int y) {
    return lunarInfo[y - 1900] & 0xf;
}

// 传回农历 y年m月的总天数
int LunarCore::monthDays(int y, int m) {
    return ((lunarInfo[y - 1900] & (0x10000 >> m)) != 0) ? 30 : 29;
}

bool LunarCore::isGregorianLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int LunarCore::getGregorianDays(int year, int month) {
    int days[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2 && isGregorianLeap(year)) return 29;
    return days[month];
}

// 计算到 1900-1-31 (农历1900正月初一) 的偏移天数
int LunarCore::getOffsetDays(int year, int month, int day) {
    int offset = 0;
    // 累加整年的天数
    for (int y = 1900; y < year; ++y) {
        offset += isGregorianLeap(y) ? 366 : 365;
    }
    // 累加当年前几个月的天数
    for (int m = 1; m < month; ++m) {
        offset += getGregorianDays(year, m);
    }
    // 累加当月的天数，并减去 1900年1月的31天偏移
    offset += day - 31;
    return offset;
}

string LunarCore::getLunarDayString(int day) {
    const string numStr[] = { "日", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十" };
    if (day == 20) return "二十";
    if (day == 30) return "三十";
    if (day < 11) return "初" + numStr[day];
    if (day < 20) return "十" + numStr[day % 10];
    return "廿" + numStr[day % 10];
}

string LunarCore::getLunarMonthString(int month, bool isLeap) {
    const string monStr[] = { "错", "正", "二", "三", "四", "五", "六", "七", "八", "九", "十", "冬", "腊" };
    string prefix = isLeap ? "闰" : "";
    return prefix + monStr[month] + "月";
}

string LunarCore::getLunarString(int year, int month, int day) {
    if (year < 1901 || year > 2099) return ""; // 限制范围，保证数据表安全

    int offset = getOffsetDays(year, month, day);
    
    int iYear, iMonth, iDay;
    int daysOfYear = 0;

    // 推算农历年份
    for (iYear = 1900; iYear < 2100 && offset > 0; iYear++) {
        daysOfYear = getYearDays(iYear);
        offset -= daysOfYear;
    }
    if (offset < 0) {
        offset += daysOfYear;
        iYear--;
    }

    int leap = leapMonth(iYear); 
    bool isLeap = false;
    int daysOfMonth = 0;

    // 推算农历月份
    for (iMonth = 1; iMonth < 13 && offset > 0; iMonth++) {
        // 如果遇到了闰月
        if (leap > 0 && iMonth == (leap + 1) && !isLeap) {
            --iMonth; 
            isLeap = true;
            daysOfMonth = leapDays(iYear);
        } else {
            daysOfMonth = monthDays(iYear, iMonth);
        }

        offset -= daysOfMonth;
        
        if (isLeap && iMonth == leap) {
            isLeap = false;
        }
    }

    if (offset == 0 && leap > 0 && iMonth == leap + 1) {
        if (isLeap) {
            isLeap = false;
        } else {
            isLeap = true;
            --iMonth;
        }
    }

    if (offset < 0) {
        offset += daysOfMonth;
        --iMonth;
    }

    iDay = offset + 1;
    // ==========================================
    // 新增：节日与节气拦截系统
    // ==========================================

    // 1. 公历固定节日 (优先级最高)
    if (month == 1 && day == 1) return "元旦";
    if (month == 5 && day == 1) return "劳动";
    if (month == 10 && day == 1) return "国庆";
    // 你可以自己在这里继续加，比如 12月25日圣诞 等等

    // 2. 农历传统节日 (注意这里用的是 iMonth 和 iDay)
    if (!isLeap) { // 传统节日通常不在闰月过
        if (iMonth == 1 && iDay == 1) return "春节";
        if (iMonth == 1 && iDay == 15) return "元宵";
        if (iMonth == 5 && iDay == 5) return "端午";
        if (iMonth == 7 && iDay == 7) return "七夕";
        if (iMonth == 8 && iDay == 15) return "中秋";
        if (iMonth == 9 && iDay == 9) return "重阳";
        // 难点：除夕是农历一年的最后一天，不一定是腊月三十，也有可能是腊月二十九
        if (iMonth == 12 && iDay == daysOfMonth) return "除夕";
    }

    // 3. 二十四节气 (仅针对 2000-2099 年使用经验公式推算)
    if (year >= 2000 && year <= 2099) {
        int Y = year % 100;    // 取年份后两位
        double D = 0.2422;     // 回归年常数
        int leap = Y / 4;      // 闰年数

        // 21世纪 24 节气常数 C (按 1月小寒 到 12月冬至 顺序排列)
        const double C_term[24] = {
            5.4055, 20.12,  // 1月: 小寒, 大寒
            3.87, 18.73,    // 2月: 立春, 雨水
            5.63, 20.646,   // 3月: 惊蛰, 春分
            4.81, 20.1,     // 4月: 清明, 谷雨
            5.52, 21.04,    // 5月: 立夏, 小满
            5.678, 21.37,   // 6月: 芒种, 夏至
            7.108, 22.83,   // 7月: 小暑, 大暑
            7.50, 23.13,    // 8月: 立秋, 处暑
            7.646, 23.042,  // 9月: 白露, 秋分
            8.318, 23.438,  // 10月: 寒露, 霜降
            7.438, 22.36,   // 11月: 立冬, 小雪
            7.18, 21.94     // 12月: 大雪, 冬至
        };
        const string termNames[24] = {
            "小寒", "大寒", "立春", "雨水", "惊蛰", "春分",
            "清明", "谷雨", "立夏", "小满", "芒种", "夏至",
            "小暑", "大暑", "立秋", "处暑", "白露", "秋分",
            "寒露", "霜降", "立冬", "小雪", "大雪", "冬至"
        };

        // 算出本月对应的两个节气在数组里的索引
        int idx1 = (month - 1) * 2;
        int idx2 = idx1 + 1;

        // 计算本月两个节气的具体公历日期
        int term1Day = static_cast<int>(Y * D + C_term[idx1]) - leap;
        int term2Day = static_cast<int>(Y * D + C_term[idx2]) - leap;

        // 命中节气则返回节气名称
        if (day == term1Day) return termNames[idx1];
        if (day == term2Day) return termNames[idx2];
    }

    // ==========================================
    // 拦截结束，如果没有命中特殊日子，执行原有的拼装逻辑
    // ==========================================

    if (iDay == 1) {
        return getLunarMonthString(iMonth, isLeap);
    } else {
        return getLunarDayString(iDay);
    }
}