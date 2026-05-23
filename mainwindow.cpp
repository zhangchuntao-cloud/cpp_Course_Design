#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QInputDialog>
#include <QDate>      // 【核心引入】：用于计算日期差
#include <ctime>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <vector>
#include <algorithm>

// 定义一个结构体，方便对近期的日程进行排序
struct UpcomingSchedule {
    int year, month, day;
    QString memoText;
    int daysLeft;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    time_t t = time(nullptr);
    tm* now = localtime(&t);
    int currentYear = now->tm_year + 1900;
    int currentMonth = now->tm_mon + 1;

    cal = new CalendarCore(currentYear, currentMonth);
    memoMgr = new MemoManager("memos_data.txt");

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setFocusPolicy(Qt::NoFocus);
    ui->tableWidget->setShowGrid(true);

    // ==========================================
    // UI 视觉美化引擎升级 (融入侧边栏样式)
    // ==========================================
    QString styleSheet = R"(
        QMainWindow { background-color: #F5F7FA; }
        QLabel#labelTitle { font-size: 24px; font-weight: bold; color: #2C3E50; padding: 10px; }
        QLabel#labelSidebarTitle { font-size: 16px; font-weight: bold; color: #34495E; padding: 5px; }
        QPushButton { background-color: #FFFFFF; border: 1px solid #DCDFE6; border-radius: 6px; color: #606266; font-size: 14px; padding: 6px 15px; font-weight: bold; }
        QPushButton:hover { background-color: #ECF5FF; color: #409EFF; border: 1px solid #c6e2ff; }
        QTableWidget { background-color: #FFFFFF; color: #303133; border: 1px solid #EBEEF5; border-radius: 8px; gridline-color: #F2F6FC; font-size: 15px; }
        QHeaderView::section { background-color: #F8F9FA; color: #909399; font-weight: bold; font-size: 16px; border: none; border-bottom: 2px solid #EBEEF5; padding: 8px; }
        QTableCornerButton::section { background-color: #F8F9FA; border: none; }

        /* 侧边栏列表美化 */
        QListWidget {
            background-color: #FFFFFF;
            border: 1px solid #EBEEF5;
            border-radius: 8px;
            padding: 5px;
            color: #2C3E50;
        }
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid #F2F6FC;
        }
        QListWidget::item:hover {
            background-color: #F5F7FA;
            border-radius: 4px;
            color: #409EFF;
        }
    )";
    this->setStyleSheet(styleSheet);

    // 连接侧边栏的点击事件（利用了编译器的自动连接，或者在此手动连一次安全防漏）
    connect(ui->listWidgetSidebar, &QListWidget::itemClicked, this, &MainWindow::on_listWidgetSidebar_itemClicked);

    updateCalendarUI();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete cal;
    delete memoMgr;
}

void MainWindow::updateCalendarUI()
{
    QString title = QString::number(cal->getYear()) + " 年 " + QString::number(cal->getMonth()) + " 月";
    ui->labelTitle->setText(title);
    ui->tableWidget->clearContents();

    int firstDay = cal->getFirstDayOfWeek() % 7;
    int days = cal->getDaysInMonth();
    int row = 0;
    int col = firstDay;

    for (int day = 1; day <= days; ++day) {
        std::string lunar = LunarCore::getLunarString(cal->getYear(), cal->getMonth(), day);
        bool hasMemo = memoMgr->hasMemo(cal->getYear(), cal->getMonth(), day);

        QString cellText = QString::number(day) + "\n" + QString::fromStdString(lunar);
        if (hasMemo) {
            cellText = "★ " + cellText;
        }

        QTableWidgetItem* item = new QTableWidgetItem(cellText);
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(row, col, item);

        col++;
        if (col > 6) { col = 0; row++; }
    }

    updateSidebarUI();
}

// ==========================================
// 亮点核心：倒计时与侧边栏动态聚合算法
// ==========================================
void MainWindow::updateSidebarUI()
{
    ui->listWidgetSidebar->clear();
    QDate today = QDate::currentDate();
    std::vector<UpcomingSchedule> list;

    int startYear = today.year();
    for (int y = startYear; y <= startYear + 1; ++y) {
        for (int m = 1; m <= 12; ++m) {
            if (y < 1901 || y > 2099) continue;

            int maxDay = 31;
            if (m == 4 || m == 6 || m == 9 || m == 11) maxDay = 30;
            else if (m == 2) {
                maxDay = ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) ? 29 : 28;
            }

            for (int d = 1; d <= maxDay; ++d) {
                if (memoMgr->hasMemo(y, m, d)) {
                    QDate memoDate(y, m, d);
                    int daysLeft = today.daysTo(memoDate);

                    if (daysLeft >= 0) {
                        QString rawMemo = QString::fromStdString(memoMgr->getMemo(y, m, d));
                        QStringList memos = rawMemo.split("；", Qt::SkipEmptyParts);

                        for (const QString& singleMemo : memos) {
                            UpcomingSchedule item = { y, m, d, singleMemo, daysLeft };
                            list.push_back(item);
                        }
                    }
                }
            }
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

void MainWindow::on_listWidgetSidebar_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int y = item->data(Qt::UserRole).toInt();
    int m = item->data(Qt::UserRole + 1).toInt();
    int d = item->data(Qt::UserRole + 2).toInt();

    cal->updateDate(y, m);
    updateCalendarUI();

    int firstDay = cal->getFirstDayOfWeek() % 7;
    int targetIndex = firstDay + d - 1;
    int row = targetIndex / 7;
    int col = targetIndex % 7;
    ui->tableWidget->setCurrentCell(row, col);
    ui->tableWidget->setFocus();
}

// 翻页与跳转保持原样，它们会自动触发 updateCalendarUI() 从而更新侧边栏
void MainWindow::on_btnPrev_clicked()
{
    int y = cal->getYear();
    int m = cal->getMonth() - 1;
    if (m < 1) { m = 12; y--; }
    if (y < 1901) { QMessageBox::information(this, "提示", "已经到达系统支持的最小年份 (1901年)。"); return; }
    cal->updateDate(y, m);
    updateCalendarUI();
}

void MainWindow::on_btnNext_clicked()
{
    int y = cal->getYear();
    int m = cal->getMonth() + 1;
    if (m > 12) { m = 1; y++; }
    if (y > 2099) { QMessageBox::information(this, "提示", "已经到达系统支持的最大年份 (2099年)。"); return; }
    cal->updateDate(y, m);
    updateCalendarUI();
}

void MainWindow::on_btnJump_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "跳转年月", "请输入年份和月份 (如: 2026 5)\n支持范围: 1901 - 2099:", QLineEdit::Normal, "", &ok);

    if (ok && !text.isEmpty()) {
        QStringList parts = text.split(" ", Qt::SkipEmptyParts);
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
        } else { QMessageBox::warning(this, "错误", "格式错误！请使用空格分隔年份和月份。"); }
    }
}
void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    QTableWidgetItem* item = ui->tableWidget->item(row, column);
    if (!item) return;

    QString text = item->text();
    if (text.isEmpty()) return;

    QString dayStr = text.split("\n")[0];
    dayStr.remove(QRegularExpression("[^0-9]"));
    int day = dayStr.toInt();
    if (day < 1 || day > 31) return;

    int y = cal->getYear();
    int m = cal->getMonth();

    QDialog dialog(this);
    dialog.setWindowTitle(QString("%1年%2月%3日 备忘录管理").arg(y).arg(m).arg(day));
    dialog.resize(350, 400);

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    QListWidget* listWidget = new QListWidget(&dialog);
    mainLayout->addWidget(listWidget);

    if (memoMgr->hasMemo(y, m, day)) {
        QString currentMemo = QString::fromStdString(memoMgr->getMemo(y, m, day));
        QStringList memos = currentMemo.split("；", Qt::SkipEmptyParts);
        for(const QString& memoStr : memos) { listWidget->addItem(memoStr); }
    }

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

    connect(btnAdd, &QPushButton::clicked, [&]() {
        QString newText = inputEdit->text().trimmed();
        if(!newText.isEmpty()) { listWidget->addItem(newText); inputEdit->clear(); }
    });
    connect(btnDelete, &QPushButton::clicked, [&]() { qDeleteAll(listWidget->selectedItems()); });
    connect(btnSave, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        memoMgr->deleteMemo(y, m, day);
        for(int i = 0; i < listWidget->count(); ++i) {
            memoMgr->addMemo(y, m, day, listWidget->item(i)->text().toStdString());
        }
        updateCalendarUI();
    }
}