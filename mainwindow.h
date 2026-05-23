#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem> // 新增引入
#include "CalendarCore.h"
#include "LunarCore.h"
#include "MemoManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnPrev_clicked();
    void on_btnNext_clicked();
    void on_btnJump_clicked();
    void on_tableWidget_cellDoubleClicked(int row, int column);
    // 新增：点击侧边栏日程时触发的槽函数
    void on_listWidgetSidebar_itemClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    CalendarCore* cal;
    MemoManager* memoMgr;

    void updateCalendarUI();
    void updateSidebarUI(); // 新增：更新右侧倒计时侧边栏
};

#endif