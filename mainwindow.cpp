#include "mainwindow.h"       // 引入主窗口头文件
#include "ui_mainwindow.h"    // 引入 Qt 自动生成的 UI 布局文件
#include <QTableWidgetItem>   // 用于向表格控件单元格填入内容
#include <QMessageBox>         // 用于弹出提示、警告对话框
#include <QDialog>            // 用于创建自定义弹出对话框（备忘录管理）
#include <QVBoxLayout>        // 垂直布局管理器
#include <QHBoxLayout>        // 水平布局管理器
#include <QListWidget>        // 列表控件，用于显示日程列表
#include <QPushButton>        // 按钮控件
#include <QLineEdit>          // 单行输入框控件
#include <QInputDialog>       // 标准输入对话框（用于跳转年月）
#include <QDate>              // 【核心引入】：Qt时间类，用于计算天数差（倒计时）
#include <ctime>              // C标准时间库，用于获取系统当前时间
#include <QString>            // Qt 字符串类
#include <QStringList>        // Qt 字符串列表类（常用于切割字符串）
#include <QRegularExpression> // 正则表达式类，用于过滤非数字字符
#include <vector>             // 标准模板库容器，动态数组
#include <algorithm>          // 标准算法库，包含 std::sort, std::min

// 定义一个结构体，方便对近期的日程进行数据打包和排序
struct UpcomingSchedule {
    int year, month, day;  // 日程对应的具体年月日
    QString memoText;      // 备忘录的单条内容
    int daysLeft;          // 距离今天还有几天（倒计时天数）
};


//====================构造与解析函数==========================================
//
MainWindow::MainWindow(QWidget *parent) 
    : QMainWindow(parent), ui(new Ui::MainWindow) 
{
    ui->setupUi(this); // 初始化并构建由 Qt Designer 设计的 UI 界面

    // 1. 获取系统当前时间，作为万年历启动时的默认显示日期
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    int currentYear = now->tm_year + 1900; // tm_year 自 1900 年开始计算，需加 1900
    int currentMonth = now->tm_mon + 1;    // tm_mon 范围是 0-11，需加 1

    // 2. 实例化日历核心和备忘录管理器（数据文件定为 memos_data.txt）
    cal = new CalendarCore(currentYear, currentMonth);
    memoMgr = new MemoManager("memos_data.txt");

    // 3. 配置日历表格 (QTableWidget) 的基础行为
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁用双击直接编辑单元格文本
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 列宽自适应拉伸
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);   // 行高自适应拉伸
    ui->tableWidget->setFocusPolicy(Qt::NoFocus);  // 移除单元格被选中时的虚线框焦点
    ui->tableWidget->setShowGrid(true);            // 显示日历网格线

    // 4. QSS 界面视觉美化引擎升级 (融入高端侧边栏和现代配色)
    QString styleSheet = R"(
        QMainWindow { background-color: #F5F7FA; }
        QLabel#labelTitle { font-size: 24px; font-weight: bold; color: #2C3E50; padding: 10px; }
        QLabel#labelSidebarTitle { font-size: 16px; font-weight: bold; color: #34495E; padding: 5px; }
        QPushButton { background-color: #FFFFFF; border: 1px solid #DCDFE6; border-radius: 6px; color: #606266; font-size: 14px; padding: 6px 15px; font-weight: bold; }
        QPushButton:hover { background-color: #ECF5FF; color: #409EFF; border: 1px solid #c6e2ff; }
        QTableWidget { background-color: #FFFFFF; color: #303133; border: 1px solid #EBEEF5; border-radius: 8px; gridline-color: #F2F6FC; font-size: 15px; }
        QHeaderView::section { background-color: #F8F9FA; color: #909399; font-weight: bold; font-size: 16px; border: none; border-bottom: 2px solid #EBEEF5; padding: 8px; }
        QTableCornerButton::section { background-color: #F8F9FA; border: none; }
    
        QListWidget { background-color: #FFFFFF; border: 1px solid #EBEEF5; border-radius: 8px; padding: 5px; color: #2C3E50; }
        QListWidget::item { padding: 10px; border-bottom: 1px solid #F2F6FC; }
        QListWidget::item:hover { background-color: #F5F7FA; border-radius: 4px; color: #409EFF; }
    )";
    this->setStyleSheet(styleSheet); // 应用全局样式表

    // 5. 手动绑定侧边栏点击事件（防漏安全连接）
    connect(ui->listWidgetSidebar, &QListWidget::itemClicked, this, &MainWindow::on_listWidgetSidebar_itemClicked);

    // 6. 首次渲染刷新日历界面
    updateCalendarUI();
}

MainWindow::~MainWindow() {
    delete ui;       // 销毁界面指针
    delete cal;      // 释放日历核心内存
    delete memoMgr;  // 释放备忘录核心内存（触发 MemoManager 析构，自动安全写入磁盘）
}
//===============核心渲染:主日历网格刷新 (Lines 101-131)
void MainWindow::updateCalendarUI() {
    // 1. 更新顶部标题标签，形如 "2026 年 5 月"
    QString title = QString::number(cal->getYear()) + " 年 " + QString::number(cal->getMonth()) + " 月";
    ui->labelTitle->setText(title);

    // 2. 清空日历表格上的所有内容
    ui->tableWidget->clearContents();

    // 3. 计算本月第一天是星期几，决定从哪一个格子（哪一列）开始画数字
    int firstDay = cal->getFirstDayOfWeek() % 7; 
    int days = cal->getDaysInMonth(); // 获取当前月份总天数
    int row = 0;
    int col = firstDay; // 初始列定位到第一天对应的星期几

    // 4. 循环填充每一天的日子
    for (int day = 1; day <= days; ++day) {
        // 调用农历核心获取阴历字符串（如 "初一", "端午节"）
        std::string lunar = LunarCore::getLunarString(cal->getYear(), cal->getMonth(), day);
        // 查询今天是否有备忘录
        bool hasMemo = memoMgr->hasMemo(cal->getYear(), cal->getMonth(), day);

        // 拼接单元格文本：第一行公历数字，第二行农历文字
        QString cellText = QString::number(day) + "\n" + QString::fromStdString(lunar);
        if (hasMemo) {
            cellText = "★ " + cellText; // 如果有备忘录，在前面打上瞩目的星星标记
        }

        // 实例化单元格项，并设置居中对齐，填入表格中
        QTableWidgetItem* item = new QTableWidgetItem(cellText);
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(row, col, item);

        // 列坐标右移
        col++;
        if (col > 6) { // 如果超过星期六（第7列），换行，列回到0（星期日）
            col = 0;
            row++;
        }
    }

    // 5. 主界面刷完后，同步触发侧边栏日程列表的更新
    updateSidebarUI();
}

// ==========================================
// 亮点核心：倒计时与侧边栏动态聚合算法
// ==========================================
void MainWindow::updateSidebarUI() {
    ui->listWidgetSidebar->clear(); // 清空侧边栏日程列表
    QDate today = QDate::currentDate(); // 获取当前系统精确的 QDate 日期
    std::vector<UpcomingSchedule> list; // 暂存所有满足条件的未过期日程表

    int startYear = today.year();
    // 遍历当前年及下一年（跨年覆盖），动态搜寻备忘录
    for (int y = startYear; y <= startYear + 1; ++y) {
        for (int m = 1; m <= 12; ++m) {
            if (y < 1901 || y > 2099) continue; // 越界安全检查

            // 计算该月最大天数（考虑闰年平年）
            int maxDay = 31;
            if (m == 4 || m == 6 || m == 9 || m == 11) maxDay = 30;
            else if (m == 2) {
                maxDay = ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) ? 29 : 28;
            }

            for (int d = 1; d <= maxDay; ++d) {
                if (memoMgr->hasMemo(y, m, d)) { // 如果这天有备忘录
                    QDate memoDate(y, m, d);
                    int daysLeft = today.daysTo(memoDate); // 【核心算法】：计算今天到目标日期的天数差

                    if (daysLeft >= 0) { // 过滤掉已经过去的过期日程
                        // 获取整段原始文本
                        QString rawMemo = QString::fromStdString(memoMgr->getMemo(y, m, d));
                        // 按照中文分号“；”切割成多条独立的单项日程
                        QStringList memos = rawMemo.split("；", Qt::SkipEmptyParts);
                        
                        // 将每条日程打包存入 vector 容器
                        for (const QString& singleMemo : memos) {
                            UpcomingSchedule item = { y, m, d, singleMemo, daysLeft };
                            list.push_back(item);
                        }
                    }
                }
            }
        }
    }

    // 【标准库 Lambda 排序】：按距离今天的倒计时天数从小到大（最近的排最前面）进行升序排序
    std::sort(list.begin(), list.end(), [](const UpcomingSchedule& a, const UpcomingSchedule& b) {
        return a.daysLeft < b.daysLeft;
    });

    // 限制侧边栏最多显示最近的 8 条日程，防止列表过长破坏美观
    int showCount = std::min(8, (int)list.size());
    for (int i = 0; i < showCount; ++i) {
        QString displayStr = QString("[%1/%2] %3\n").arg(list[i].month).arg(list[i].day).arg(list[i].memoText);
        
        if (list[i].daysLeft == 0) {
            displayStr += "⏰ 今天！"; // 0天说明就是今天
        } else {
            displayStr += QString("⏳ 倒计时 %1 天").arg(list[i].daysLeft);
        }

        // 创建列表子项，并塞入侧边栏控件
        QListWidgetItem* listItem = new QListWidgetItem(displayStr, ui->listWidgetSidebar);
        
        // 【核心点】：使用 Qt 的 UserRole 隐藏存储该日程对应的真实年月日
        // 这样即使界面上没显示年份，点击时也能精确提取
        listItem->setData(Qt::UserRole, list[i].year);
        listItem->setData(Qt::UserRole + 1, list[i].month);
        listItem->setData(Qt::UserRole + 2, list[i].day);
    }
}

    std::sort(list.begin(), list.end(), [](const UpcomingSchedule& a, const UpcomingSchedule& b) {
        return a.daysLeft < b.daysLeft;
    });

    int showCount = std::min(8, (int)list.size());
    for (int i = 0; i < showCount; ++i) {
        QString displayStr = QString("[%1/%2] %3\n").arg(list[i].month).arg(list[i].day).arg(list[i].memoText);

        if (list[i].daysLeft == 0) {
            displayStr += "⏰ 今天！";
        } else {
            displayStr += QString("⏳ 倒计时 %1 天").arg(list[i].daysLeft);
        }

        QListWidgetItem* listItem = new QListWidgetItem(displayStr, ui->listWidgetSidebar);

        listItem->setData(Qt::UserRole, list[i].year);
        listItem->setData(Qt::UserRole + 1, list[i].month);
        listItem->setData(Qt::UserRole + 2, list[i].day);
    }
}
//
// 当点击侧边栏的某条日程时，主日历自动反向跳转并聚焦到这一天
void MainWindow::on_listWidgetSidebar_itemClicked(QListWidgetItem *item) {
    if (!item) return;
    
    // 取出刚才隐式存在里面的年月日数据
    int y = item->data(Qt::UserRole).toInt();
    int m = item->data(Qt::UserRole + 1).toInt();
    int d = item->data(Qt::UserRole + 2).toInt();

    cal->updateDate(y, m); // 让日历核心逻辑跳转到这一年这一月
    updateCalendarUI();    // 刷新整个日历大网格

    // 重新推算这一天在 7x6 单元格矩阵中的精确行号和列号
    int firstDay = cal->getFirstDayOfWeek() % 7;
    int targetIndex = firstDay + d - 1; // 目标一维索引
    int row = targetIndex / 7;          // 换算成二维网格行号
    int col = targetIndex % 7;          // 换算成二维网格列号

    ui->tableWidget->setCurrentCell(row, col); // 将当前高亮选中单元格直接定位到那一天
    ui->tableWidget->setFocus();               // 强制让日历表获取焦点，产生视觉选中框
}

// “上一月”按钮点击事件
void MainWindow::on_btnPrev_clicked() {
    int y = cal->getYear();
    int m = cal->getMonth() - 1;
    if (m < 1) { m = 12; y--; } // 跨年递减处理
    if (y < 1901) {
        QMessageBox::information(this, "提示", "已经到达系统支持的最小年份 (1901年)。");
        return;
    }
    cal->updateDate(y, m);
    updateCalendarUI(); // 内部联动触发侧边栏更新
}

// “下一月”按钮点击事件
void MainWindow::on_btnNext_clicked() {
    int y = cal->getYear();
    int m = cal->getMonth() + 1;
    if (m > 12) { m = 1; y++; } // 跨年递增处理
    if (y > 2099) {
        QMessageBox::information(this, "提示", "已经到达系统支持的最大年份 (2099年)。");
        return;
    }
    cal->updateDate(y, m);
    updateCalendarUI();
}

// “跳转年月”自由输入跳转
void MainWindow::on_btnJump_clicked() {
    bool ok;
    // 弹出标准文本框让用户录入形如 "2026 5"
    QString text = QInputDialog::getText(this, "跳转年月", "请输入年份和月份 (如: 2026 5)\n支持范围: 1901 - 2099:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        QStringList parts = text.split(" ", Qt::SkipEmptyParts); // 以空格分割
        if (parts.size() >= 2) {
            int y = parts[0].toInt();
            int m = parts[1].toInt();
            if (y < 1901 || y > 2099) {
                QMessageBox::warning(this, "范围错误", "年份超出支持范围！请输入 1901 到 2099 之间的年份。");
                return;
            }
            if (m >= 1 && m <= 12) {
                cal->updateDate(y, m);
                updateCalendarUI();
            } else {
                QMessageBox::warning(this, "错误", "月份必须在 1 到 12 之间！");
            }
        } else {
            QMessageBox::warning(this, "错误", "格式错误！请使用空格分隔年份和月份。");
        }
    }
}

// 双击某天日历格，拉起备忘录管理器模态框
void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column) {
    QTableWidgetItem* item = ui->tableWidget->item(row, column);
    if (!item) return;
    QString text = item->text();
    if (text.isEmpty()) return; // 空格子（上月残留或下月开头空位）不作响应

    // 1. 提取被点击格子里的公历数字日子
    QString dayStr = text.split("\n")[0]; // 切割取第一行
    dayStr.remove(QRegularExpression("[^0-9]")); // 正则剔除可能自带的 "★" 等干扰符号
    int day = dayStr.toInt();
    if (day < 1 || day > 31) return;

    int y = cal->getYear();
    int m = cal->getMonth();

    // 2. 动态用代码构建一个微型弹出对话框 (QDialog)
    QDialog dialog(this);
    dialog.setWindowTitle(QString("%1年%2月%3日 备忘录管理").arg(y).arg(m).arg(day));
    dialog.resize(350, 400);

    // 3. 为对话框组装布局与控件
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    QListWidget* listWidget = new QListWidget(&dialog); // 备忘录条目展示列表
    mainLayout->addWidget(listWidget);

    // 4. 如果今天有数据，读出来并切割填充到列表里展示
    if (memoMgr->hasMemo(y, m, day)) {
        QString currentMemo = QString::fromStdString(memoMgr->getMemo(y, m, day));
        QStringList memos = currentMemo.split("；", Qt::SkipEmptyParts);
        for(const QString& memoStr : memos) {
            listWidget->addItem(memoStr);
        }
    }

    // 5. 组合底部的输入和按钮控件
    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLineEdit* inputEdit = new QLineEdit(&dialog);
    QPushButton* btnAdd = new QPushButton("添加", &dialog);
    inputLayout->addWidget(inputEdit);
    inputLayout->addWidget(btnAdd);
    mainLayout->addLayout(inputLayout);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    QPushButton* btnDelete = new QPushButton("删除选中", &dialog);
    QPushButton* btnSave = new QPushButton("保存并关闭", &dialog);
    actionLayout->addWidget(btnDelete);
    actionLayout->addWidget(btnSave);
    mainLayout->addLayout(actionLayout);

    // 6. 槽函数绑定 (Lambda 表达式闭包写法，高效轻量)
    // 【添加条目】：点击添加时将输入框文本插入列表组件
    connect(btnAdd, &QPushButton::clicked, [&]() {
        QString newText = inputEdit->text().trimmed();
        if(!newText.isEmpty()) {
            listWidget->addItem(newText);
            inputEdit->clear();
        }
    });

    // 【删除条目】：点击删除选中的条目
    connect(btnDelete, &QPushButton::clicked, [&]() {
        qDeleteAll(listWidget->selectedItems()); // 批量安全释放并移除选中的子项
    });

    // 【确认关闭】：点击保存时使对话框发出 Accept 信号关闭
    connect(btnSave, &QPushButton::clicked, &dialog, &QDialog::accept);

    // 7. 【阻塞式开启模态框】：等待用户操作
    if (dialog.exec() == QDialog::Accepted) {
        // 先彻底清空这一天原本的备忘录旧数据
        memoMgr->deleteMemo(y, m, day);
        
        // 循环把列表中留存的备忘录单项逐个读出，通过 addMemo 拼装保存
        for(int i = 0; i < listWidget->count(); ++i) {
            memoMgr->addMemo(y, m, day, listWidget->item(i)->text().toStdString());
        }
        
        // 数据写入完毕，全局刷新主日历网格（会补上或去掉小星星 ★）
        updateCalendarUI();
    }
}
