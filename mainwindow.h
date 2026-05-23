#ifndef MAINWINDOW_H    // 【头文件保护】检查是否未定义 MAINWINDOW_H 宏
#define MAINWINDOW_H    // 如果未定义，则定义它，防止该头文件被重复包含导致编译冲突

#include <QMainWindow>       // 引入 Qt 系统的标准主窗口类，MainWindow 将继承它
#include <QListWidgetItem>   // 新增引入：用于处理右侧侧边栏日程列表中的单个小组件项

// 引入自定义的三个核心业务类头文件
#include "CalendarCore.h"    // 日历核心逻辑类（负责公历算法、日期切换等）
#include "LunarCore.h"       // 农历核心逻辑类（负责阴历、节气、节日等转换）
#include "MemoManager.h"     // 备忘录管理类（负责备忘录的增删改查与文件读写）

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; } // 前置声明 UI 命名空间中的 MainWindow 类，用于绑定由 Qt Designer 绘制的界面
QT_END_NAMESPACE

// MainWindow 类继承自 QMainWindow
class MainWindow : public QMainWindow {
    Q_OBJECT // 【Qt核心宏】必须放在类定义的第一行！只有加了它，类才支持 Qt 的信号与槽（Signals & Slots）机制

public:
    // 构造函数：parent 参数允许指定父窗口，默认为 nullptr（表示它是一个独立顶层窗口）
    MainWindow(QWidget *parent = nullptr);
    
    // 析构函数：用于在窗口关闭、销毁时释放内存资源
    ~MainWindow();

private slots: // 【私有槽函数】用于接收并响应界面上各种控件触发的信号（点击、双击等）
    
    void on_btnPrev_clicked(); // 当点击“上一月”（或上一年）按钮时自动触发的槽函数
    
    void on_btnNext_clicked(); // 当点击“下一月”（或下一年）按钮时自动触发的槽函数
    
    void on_btnJump_clicked(); // 当点击“跳转”按钮（输入指定年月跳转）时自动触发的槽函数
    
    /**
     * @brief 双击日历表格中的某个单元格时触发
     * @param row 双击的行号
     * @param column 双击的列号
     * @note 通常在此处弹窗让用户“添加”或“编辑”当前选中日期的备忘录
     */
    void on_tableWidget_cellDoubleClicked(int row, int column); 

    /**
     * @brief 新增槽函数：当点击右侧侧边栏日程列表（QListWidget）中的某一项时触发
     * @param item 被点击的日程项指针
     */
    void on_listWidgetSidebar_itemClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;     // 指向 Qt UI 界面类的指针，通过 ui-> 访问界面上的按钮、表格、标签等所有控件
    
    CalendarCore* cal;      // 指向日历核心逻辑对象的指针，用于处理日历数据计算
    
    MemoManager* memoMgr;   // 指向备忘录管理对象的指针，用于管理和持久化备忘录数据

    /**
     * @brief 内部辅助函数：刷新整个万年历的日历主界面UI
     * @note 当切换月份、跳转日期或备忘录更新时，调用此函数把最新的日子和星号标记重新填入 tableWidget
     */
    void updateCalendarUI();

    /**
     * @brief 新增辅助函数：更新右侧倒计时侧边栏UI
     * @note 用于在右侧侧边栏刷新显示近期的重要日程、节日或者倒计时列表
     */
    void updateSidebarUI(); 
};

#endif // MAINWINDOW_H // 对应第一行的 #ifndef，结束头文件保护
