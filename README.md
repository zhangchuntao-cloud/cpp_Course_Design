# 备忘录模块与图形用户界面（GUI）开发讲解文稿

我在本项目中主要负责以下两个核心部分：  
1. **备忘录管理系统**（`MemoManager` 类）的完整实现；  
2. **基于 Qt 的图形用户界面**（`MainWindow` 类及 UI 布局）的搭建与交互逻辑。

下面我将结合具体代码，详细说明各模块的设计思路、关键算法以及亮点功能。

---

## 一、备忘录管理系统（`MemoManager`）

### 1.1 设计目标
- 能够为**任意公历日期**（年/月/日）添加、查询、删除备忘录内容。
- 支持**多条备忘录**的存储与展示（以中文分号“；”分隔）。
- 数据持久化：程序启动时自动加载，修改后即时保存，关闭时再次保存。
- 提供简洁的 API 供主窗口调用。

### 1.2 核心数据结构
在 `MemoManager.h` 中：

```cpp
private:
    std::string dataFile;                         // 存储文件名
    std::map<std::string, std::string> memos;    // 键：“YYYY-M-D”，值：备忘录内容
```

- 使用 `std::map` 存储所有备忘录，键为格式化后的日期字符串（如 `"2026-5-23"`），值为多条备忘录拼接的字符串。
- 优点：查找、插入、删除均为 O(log n)，适合日历应用按日期随机访问。

### 1.3 文件持久化格式
在 `MemoManager.cpp` 的 `saveToFile()` 和 `loadFromFile()` 中：

```cpp
// 保存格式：日期|内容\n
outFile << pair.first << "|" << pair.second << "\n";

// 加载时解析
size_t pos = line.find('|');
string dateStr = line.substr(0, pos);
string content = line.substr(pos + 1);
memos[dateStr] = content;
```

- 采用简单的文本格式，每一行代表一条日期记录，使用 `|` 分隔键与值，便于调试和手动编辑。

### 1.4 关键功能实现

#### 添加备忘录（支持追加）
```cpp
void MemoManager::addMemo(int year, int month, int day, const string& content) {
    string key = formatDate(year, month, day);
    if (memos.find(key) != memos.end()) {
        memos[key] += "；" + content;   // 已存在则用分号连接
    } else {
        memos[key] = content;
    }
    saveToFile();  // 实时保存，防止程序异常崩溃导致数据丢失
}
```
- 每次添加后立即调用 `saveToFile()`，体现了对数据安全性的重视。

#### 查询与删除
```cpp
bool hasMemo(...) const;      // 判断某天是否有备忘录（用于日历上画星号）
string getMemo(...) const;    // 获取完整备忘录文本
void deleteMemo(...);         // 删除整天的所有备忘录
```

---

## 二、图形用户界面（`MainWindow`）开发

### 2.1 UI 布局与控件绑定（`mainwindow.ui`）
- 使用 Qt Designer 设计了主窗口，包含：
  - **标题栏**（`labelTitle`）：显示当前年月。
  - **三个按钮**：上个月（`btnPrev`）、下个月（`btnNext`）、跳转年月（`btnJump`）。
  - **日历表格**（`tableWidget`）：6×7 的网格，用于展示一个月中每一天的公历日期、农历以及备忘录星标。
  - **侧边栏**（`listWidgetSidebar`）：显示近期日程倒计时列表。

- 界面布局参数（位置、大小、字体）均在 `.ui` 文件中以 XML 形式定义，确保了界面美观和可维护性。

### 2.2 窗口初始化与视觉美化
在 `MainWindow` 构造函数中，除了初始化日历核心和备忘录管理器，还**动态应用了样式表**来提升用户体验：

```cpp
QString styleSheet = R"(
    QMainWindow { background-color: #F5F7FA; }
    QPushButton { background-color: #FFFFFF; border-radius: 6px; ... }
    QTableWidget { background-color: #FFFFFF; border-radius: 8px; }
    QListWidget::item:hover { background-color: #F5F7FA; border-radius: 4px; }
)";
this->setStyleSheet(styleSheet);
```
- 采用现代扁平化设计，按钮悬停变色、表格圆角、侧边栏条目悬浮高亮。

### 2.3 核心交互逻辑：日历更新与渲染

#### `updateCalendarUI()` 方法
- 根据当前 `CalendarCore` 提供的年份、月份、当月天数、1 号星期几，将日期填入 `tableWidget`。
- **关键亮点**：每个单元格显示公历日期 + 农历（通过 `LunarCore::getLunarString` 获得），并且如果 `memoMgr->hasMemo()` 返回 true，则在日期前加一个“★”符号。
```cpp
QString cellText = QString::number(day) + "\n" + QString::fromStdString(lunar);
if (hasMemo) {
    cellText = "★ " + cellText;
}
```
- 这样用户无需进入详情，即可一眼看出哪些日子有备忘录。

#### 翻页与跳转
- 按钮 `btnPrev` / `btnNext` 调用 `CalendarCore::updateDate()`，然后刷新 UI。
- 跳转功能（`btnJump`）使用 `QInputDialog` 获取用户输入，进行有效性检查（年份 1901-2099，月份 1-12），之后更新日历。

### 2.4 备忘录管理对话框（亮点交互）

当用户**双击日历表格的某个单元格**时，触发 `on_tableWidget_cellDoubleClicked` 槽函数：

```cpp
void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column) {
    // 1. 从单元格文本中解析出公历日期数字
    // 2. 弹出 QDialog 对话框，内含：
    //    - QListWidget：显示该日期已有的多条备忘录
    //    - QLineEdit + 添加按钮：用于新增备忘录条目
    //    - 删除选中按钮、保存并关闭按钮
    // 3. 用户点击保存后，清空原备忘录，将 QListWidget 中的所有条目重新添加
}
```

- 这一设计支持**同一日期多条备忘录**（如“上午开会；下午交报告”），用户可独立增删每一条。
- 对话框内部使用 `QListWidget` 和临时变量，最后一次性覆盖写入 `MemoManager`，保证了数据一致性。

### 2.5 侧边栏倒计时与聚合算法（最大亮点）

#### `updateSidebarUI()` 方法
- **功能**：从当前日期开始，扫描未来两年内（跨年）所有有备忘录的日期，计算距离今天的天数，按倒计时从小到大排序，取前 8 条显示在右侧 `QListWidget` 中。

```cpp
std::vector<UpcomingSchedule> list;
QDate today = QDate::currentDate();

for (int y = startYear; y <= startYear + 1; ++y) {
    for (int m = 1; m <= 12; ++m) {
        // 遍历该月的每一天
        if (memoMgr->hasMemo(y, m, d)) {
            int daysLeft = today.daysTo(QDate(y, m, d));
            if (daysLeft >= 0) {   // 只显示今天及未来的日程
                // 拆分多条备忘录，每条单独作为一个条目加入 list
            }
        }
    }
}

std::sort(list.begin(), list.end(), 
    [](const UpcomingSchedule& a, const UpcomingSchedule& b) {
        return a.daysLeft < b.daysLeft;
    });
```

- 每条侧边栏条目显示格式：
  ```
  [5/23] 交作业
  ⏳ 倒计时 3 天
  ```
  若 `daysLeft == 0` 则显示“⏰ 今天！”。

- **性能优化**：只扫描当前年份 + 下一年，避免全量遍历（用户通常不会关注两年后的日程）。

#### 点击侧边栏跳转到对应日期
- `on_listWidgetSidebar_itemClicked` 从 `QListWidgetItem` 的自定义数据中取出年/月/日，调用 `cal->updateDate()` 刷新日历，然后高亮对应的单元格：
```cpp
int targetIndex = firstDay + d - 1;
int row = targetIndex / 7;
int col = targetIndex % 7;
ui->tableWidget->setCurrentCell(row, col);
```
- 实现了从日程列表**快速定位到日历视图**的用户体验闭环。

### 2.6 信号与槽的显式连接
为了防止 Qt 的自动连接机制失效，在构造函数中显式连接了侧边栏的点击事件：
```cpp
connect(ui->listWidgetSidebar, &QListWidget::itemClicked, 
        this, &MainWindow::on_listWidgetSidebar_itemClicked);
```
- 同时其余按钮的点击事件利用 UI 文件中的 `on_控件名_clicked` 命名规则自动连接。

---

## 三、总结与自评

我负责的部分共计完成以下工作：

| 模块 | 代码行数 | 核心贡献 |
|------|----------|----------|
| `MemoManager.h/cpp` | ~100 行 | 完整的备忘录持久化、增删改查、多条目支持 |
| `mainwindow.ui` | ~150 行 | 布局设计，侧边栏与表格的位置、样式设置 |
| `mainwindow.cpp`（除去翻页等已有代码） | ~200 行 | 日历渲染、侧边栏动态聚合算法、双击编辑对话框、跳转高亮 |
| 样式表与界面美化 | 集成在构造函数 | 提升视觉体验，符合现代应用标准 |

**创新亮点总结**：
1. **多备忘录存储**：用分号拼接，支持同一日期多条独立事务。
2. **智能倒计时侧边栏**：动态扫描、排序、截断显示，并支持点击跳转。
3. **所见即所得的备忘录星标**：日历上的 ★ 标记使用户无需进入详情即可获知有无备忘。
4. **数据实时保存**：每次修改都写回文件，避免数据丢失。

通过本次项目，我深入实践了 Qt 的**模型-视图**框架、自定义对话框、样式表应用，以及 C++ 的 STL 容器（`map`、`vector`）与文件流操作。与徐超文同学的底层算法引擎紧密配合，最终交付了一个功能完整、界面美观的万年历备忘录系统。

谢谢大家！欢迎提问。
