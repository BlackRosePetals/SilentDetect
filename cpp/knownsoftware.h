#ifndef KNOWNSOFTWARE_H
#define KNOWNSOFTWARE_H

#include <string>
#include <vector>
#include <cstring>

// 知名软件静默安装参数数据库
// 匹配优先级：文件名关键词 > PE版本信息

struct KnownSoft {
    const char* pattern;      // 文件名关键词（小写，子串匹配）
    const char* productName;  // PE ProductName 子串匹配（可选，NULL=跳过）
    const char* companyName;  // PE CompanyName 子串匹配（可选，NULL=跳过）
    const char* displayName;  // 显示名称
    const char* silentCmd;    // 静默安装命令模板 ({file}替换为文件路径)
    const char* installerType; // 打包类型（用于兼容现有数据库结构）
};

// 按常用程度排序，越常用的放前面
static const KnownSoft g_knownSoftware[] = {
    // === 浏览器 ===
    {"chrome",     "Google Chrome",   "Google",   "Google Chrome",       "\"{file}\" /silent /install",       "GoogleInstaller"},
    {"firefox",    "Firefox",         "Mozilla",  "Mozilla Firefox",     "\"{file}\" /S",                     "NSIS"},
    {"opera",      "Opera",           "Opera",    "Opera Browser",       "\"{file}\" /silent /install",       "GoogleInstaller"},

    // === 压缩工具 ===
    {"7z",         "7-Zip",           NULL,       "7-Zip",               "\"{file}\" /S",                     "NSIS"},
    {"winrar",     "WinRAR",          NULL,       "WinRAR",              "\"{file}\" /s",                     "WinRAR"},
    {"bandizip",   "Bandizip",        NULL,       "Bandizip",            "\"{file}\" /S",                     "NSIS"},

    // === 播放器 ===
    {"vlc",        "VLC",             "VideoLAN", "VLC Media Player",    "\"{file}\" /S",                     "NSIS"},
    {"potplayer",  "PotPlayer",       NULL,       "PotPlayer",           "\"{file}\" /S",                     "NSIS"},
    {"mpc-hc",     "MPC-HC",          NULL,       "MPC-HC",              "\"{file}\" /S",                     "NSIS"},

    // === 办公 ===
    {"wps",        "WPS Office",      "Kingsoft", "WPS Office",          "\"{file}\" /S",                     "NSIS"},
    {"office",     NULL,              "Microsoft","Microsoft Office",    "setup.exe /configure config.xml",   "OfficeMsi"},
    {"libreoffice","LibreOffice",     NULL,       "LibreOffice",         "msiexec /i \"{file}\" /qn",         "MSI"},

    // === 开发工具 ===
    {"python",     "Python",          "Python",   "Python",              "\"{file}\" /quiet InstallAllUsers=1","MSI"},
    {"vscode",     "Visual Studio Code","Microsoft","VS Code",           "\"{file}\" /VERYSILENT /SP-",       "InnoSetup"},
    {"code",       "Visual Studio Code","Microsoft","VS Code",           "\"{file}\" /VERYSILENT /SP-",       "InnoSetup"},
    {"visualstudio","Visual Studio",  "Microsoft", "Visual Studio",      "\"{file}\" --quiet --norestart",    "VSBootstrapper"},
    {"jdk",        "Java SE",         "Oracle",   "Java JDK",            "\"{file}\" /s",                     "InstallShield"},
    {"jre",        "Java",            "Oracle",   "Java Runtime",        "\"{file}\" /s",                     "InstallShield"},
    {"java",       "Java",            "Oracle",   "Java",                "\"{file}\" /s",                     "InstallShield"},
    {"git",        "Git",             NULL,       "Git for Windows",     "\"{file}\" /VERYSILENT /SP- /NORESTART", "InnoSetup"},
    {"node",       "Node.js",         "Node.js",  "Node.js",             "msiexec /i \"{file}\" /qn",         "MSI"},

    // === 运行时 ===
    {"vcredist",   NULL,              "Microsoft", "VC++ Runtime",       "\"{file}\" /quiet /norestart",       "MSI"},
    {"netframework","Microsoft .NET", "Microsoft",".NET Framework",      "\"{file}\" /q /norestart",           "MSI"},
    {"ndp",        "Microsoft .NET",  "Microsoft",".NET Framework",      "\"{file}\" /q /norestart",           "MSI"},
    {"dotnet",     "Microsoft .NET",  "Microsoft",".NET Runtime",        "\"{file}\" /quiet /norestart",       "MSI"},
    {"directx",    "DirectX",         "Microsoft","DirectX Runtime",     "\"{file}\" /silent",                 "Custom"},

    // === IM / 通讯 ===
    {"wechat",     "微信",            "Tencent",  "微信",                "\"{file}\" /S",                     "NSIS"},
    {"wx",         NULL,              "Tencent",  "微信",                "\"{file}\" /S",                     "NSIS"},
    {"qq",         "QQ",              "Tencent",  "QQ",                  "\"{file}\" /S",                     "NSIS"},
    {"dingtalk",   "钉钉",            "Alibaba",  "钉钉",                "\"{file}\" /S",                     "NSIS"},
    {"dingding",   "钉钉",            NULL,       "钉钉",                "\"{file}\" /S",                     "NSIS"},
    {"feishu",     "飞书",            "ByteDance","飞书",                "\"{file}\" /S",                     "NSIS"},
    {"lark",       "飞书",            "ByteDance","飞书",                "\"{file}\" /S",                     "NSIS"},
    {"teams",      "Teams",           "Microsoft","Microsoft Teams",     "\"{file}\" /s",                     "InstallShield"},
    {"discord",    "Discord",         "Discord",  "Discord",             "\"{file}\" /S",                     "NSIS"},

    // === 安全 ===
    {"360",        NULL,              "Qihoo",    "360安全卫士",         "\"{file}\" /S",                     "NSIS"},
    {"huorong",    "火绒",            NULL,       "火绒安全",            "\"{file}\" /S",                     "NSIS"},

    // === 编辑器 ===
    {"notepad",    "Notepad++",       NULL,       "Notepad++",           "\"{file}\" /S",                     "NSIS"},
    {"sublime",    "Sublime Text",    NULL,       "Sublime Text",        "\"{file}\" /VERYSILENT /SP-",       "InnoSetup"},

    // === 远程 ===
    {"teamviewer","TeamViewer",       "TeamViewer","TeamViewer",         "\"{file}\" /S",                     "NSIS"},
    {"anydesk",    "AnyDesk",         NULL,       "AnyDesk",             "\"{file}\" --silent",               "Custom"},
    {"todesk",     "ToDesk",          NULL,       "ToDesk",              "\"{file}\" /S",                     "NSIS"},

    // === 其他 ===
    {"everything", NULL,              "voidtools", "Everything",         "\"{file}\" /S",                     "NSIS"},
    {"obs",        "OBS Studio",      NULL,       "OBS Studio",          "\"{file}\" /S",                     "NSIS"},
    {"adobe",      NULL,              "Adobe",    "Adobe Reader",        "\"{file}\" /sAll /rs /l /msi EULA_ACCEPT=YES", "InstallShield"},
    {"reader",     "Adobe Reader",    "Adobe",    "Adobe Reader",        "\"{file}\" /sAll /rs /l /msi EULA_ACCEPT=YES", "InstallShield"},

    {NULL, NULL, NULL, NULL, NULL, NULL} // sentinel
};

// 匹配知名软件
// 返回匹配到的 KnownSoft* 或 nullptr
inline const KnownSoft* KS_FindByPattern(const std::wstring& filePath) {
    // 提取文件名并转小写
    size_t lastSlash = filePath.find_last_of(L"\\/");
    std::wstring fileName = (lastSlash != std::wstring::npos)
        ? filePath.substr(lastSlash + 1) : filePath;
    // 也去掉扩展名
    size_t dot = fileName.rfind(L'.');
    std::wstring baseName = (dot != std::wstring::npos)
        ? fileName.substr(0, dot) : fileName;

    std::string lowerName, lowerBase, lowerFull;
    for (wchar_t c : fileName) { lowerFull += (char)tolower((unsigned char)c); }
    for (wchar_t c : baseName) { lowerBase += (char)tolower((unsigned char)c); }
    lowerName = lowerBase; // alias

    const KnownSoft* best = nullptr;
    int bestLen = 0;

    for (const KnownSoft* ks = g_knownSoftware; ks->pattern; ks++) {
        std::string pat(ks->pattern);
        // 精确匹配优先，子串匹配次之
        if (lowerBase == pat) return ks; // 文件名精确匹配，直接返回

        if (lowerFull.find(pat) != std::string::npos || lowerBase.find(pat) != std::string::npos) {
            if ((int)pat.size() > bestLen) {
                bestLen = (int)pat.size();
                best = ks;
            }
        }
    }
    return best;
}

#endif // KNOWNSOFTWARE_H
