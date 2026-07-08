#ifndef BATCHRUN_H
#define BATCHRUN_H

#include <windows.h>
#include <string>
#include <vector>

enum class RunFileType {
    ExeOnly,    // 只运行 exe, msi, appx, msix, msixbundle
    AllRunnable // 运行所有可执行文件，包括 vbs, bat, cmd, reg
};

struct RunItem {
    std::wstring filePath;
    std::wstring fileName;
    std::wstring ext;
    int priority; // 0=最先, 1=普通, 2=最后
};

// 获取当前exe所在目录
std::wstring BR_GetExeDirectory();

// 扫描并排序文件
std::vector<RunItem> BR_ScanAndSort(const std::wstring& dir, RunFileType type);

// 顺序执行所有文件
// 返回成功执行的数量
int BR_RunAll(HWND hWnd, const std::wstring& dir, RunFileType type);

// 用指定的列表顺序执行（批量顺序调整后使用）
int BR_RunAllWithList(HWND hWnd, const std::vector<RunItem>& items);

// 处理bat/cmd文件：替换pause，返回临时文件路径
// 如果不需要替换（原文件无pause），返回空字符串
std::wstring BR_PrepareBatCmd(const std::wstring& filePath);

// 清理临时文件
void BR_CleanupTemp();

// 压缩包处理功能
// 检测7z是否可用，返回7z.exe路径，不可用返回空
std::wstring BR_Find7z();

// 扫描目录下的7z文件
std::vector<std::wstring> BR_Scan7zFiles(const std::wstring& dir);

// 处理单个7z文件：解压、判断位数、创建快捷方式
// 返回是否成功
bool BR_Process7zFile(HWND hWnd, const std::wstring& zipPath, const std::wstring& sevenZPath);

// 处理所有7z文件，返回成功处理的数量
int BR_ProcessAll7z(HWND hWnd, const std::wstring& dir);

#endif // BATCHRUN_H
