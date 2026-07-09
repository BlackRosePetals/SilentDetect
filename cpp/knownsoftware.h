#ifndef KNOWNSOFTWARE_H
#define KNOWNSOFTWARE_H

#include <windows.h>
#include <wintrust.h>
#include <softpub.h>
#include <cryptuiapi.h>
#include <string>
#include <vector>
#include <cstring>

#pragma comment(lib, "wintrust.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "cryptui.lib")

// 知名软件静默安装参数数据库
// 匹配优先级：文件名关键词 > PE版本信息

struct KnownSoft {
    const char* pattern;      // 文件名关键词（小写，子串匹配，NULL=仅PE匹配）
    const char* productName;  // PE ProductName 子串匹配（可选，NULL=跳过）
    const char* companyName;  // PE CompanyName 子串匹配（可选，NULL=跳过）
    const char* installerType; // 对应的 InstallerType（查 database.h 获取参数）
};

// 读取 PE 文件头中的 TimeDateStamp（Unix时间戳）
// 返回 0 表示读取失败
inline DWORD ReadPETimestamp(const std::wstring& filePath) {
    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;

    // 读取 DOS header
    IMAGE_DOS_HEADER dosHeader;
    DWORD bytesRead;
    if (!ReadFile(hFile, &dosHeader, sizeof(dosHeader), &bytesRead, NULL)
        || dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        CloseHandle(hFile);
        return 0;
    }

    // 定位到 PE signature
    SetFilePointer(hFile, dosHeader.e_lfanew, NULL, FILE_BEGIN);
    DWORD peSig;
    if (!ReadFile(hFile, &peSig, sizeof(peSig), &bytesRead, NULL)
        || peSig != IMAGE_NT_SIGNATURE) {
        CloseHandle(hFile);
        return 0;
    }

    // 读取 COFF header（含 TimeDateStamp）
    IMAGE_FILE_HEADER fileHeader;
    if (!ReadFile(hFile, &fileHeader, sizeof(fileHeader), &bytesRead, NULL)) {
        CloseHandle(hFile);
        return 0;
    }

    CloseHandle(hFile);
    return fileHeader.TimeDateStamp;
}

// 读取数字签名中的公司名
// 返回小写字符串，例如 "canon inc."
inline std::string ReadSignatureCompany(const std::wstring& filePath) {
    std::string result;
    HCERTSTORE hStore = NULL;
    HCRYPTMSG hMsg = NULL;
    DWORD encoding, contentType, formatType;
    if (!CryptQueryObject(CERT_QUERY_OBJECT_FILE, filePath.c_str(),
                          CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
                          CERT_QUERY_FORMAT_FLAG_BINARY, 0,
                          &encoding, &contentType, &formatType,
                          &hStore, &hMsg, NULL))
        return result;

    DWORD signerInfoSize = 0;
    CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, NULL, &signerInfoSize);
    if (signerInfoSize > 0) {
        std::vector<BYTE> signerBuf(signerInfoSize);
        CMSG_SIGNER_INFO* signerInfo = (CMSG_SIGNER_INFO*)signerBuf.data();
        if (CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, signerInfo, &signerInfoSize)) {
            CERT_INFO ci = {};
            ci.Issuer = signerInfo->Issuer;
            ci.SerialNumber = signerInfo->SerialNumber;
            PCCERT_CONTEXT pCert = CertFindCertificateInStore(hStore,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0,
                CERT_FIND_SUBJECT_CERT, &ci, NULL);
            if (pCert) {
                DWORD len = CertGetNameStringW(pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, NULL, 0);
                if (len > 1) {
                    std::vector<wchar_t> buf(len);
                    CertGetNameStringW(pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, buf.data(), len);
                    for (int i = 0; i < len && buf[i]; i++)
                        result += (char)tolower((unsigned char)buf[i]);
                }
                CertFreeCertificateContext(pCert);
            }
        }
    }

    CertCloseStore(hStore, 0);
    CryptMsgClose(hMsg);
    return result;
}

// 时间戳 → 年份
inline int TimestampToYear(DWORD ts) {
    // 简单转换：1970 + ts/31536000
    if (ts == 0) return 0;
    return 1970 + (int)(ts / 31536000UL);
}

// 静默参数变体（根据PE时间戳的年份范围选择）
struct SilentParamVariant {
    int yearFrom;        // 起始年份（含）
    int yearTo;          // 结束年份（含），0=不限
    const char* command; // 静默参数
    const char* label;   // 参数说明
};

// 按常用程度排序，越常用的放前面
static const KnownSoft g_knownSoftware[] = {
    // 格式: {文件名关键词, PE产品名, PE公司名, InstallerType}
    // === 浏览器 ===
    {"chrome",     "Google Chrome",   "Google",   "GoogleInstaller"},
    {"firefox",    "Firefox",         "Mozilla",  "NSIS"},
    {"opera",      "Opera",           "Opera",    "GoogleInstaller"},
    // === 压缩工具 ===
    {"7z",         "7-Zip",           NULL,       "NSIS"},
    {"winrar",     "WinRAR",          NULL,       "WinRAR"},
    {"bandizip",   "Bandizip",        NULL,       "NSIS"},
    // === 播放器 ===
    {"vlc",        "VLC",             "VideoLAN", "NSIS"},
    {"potplayer",  "PotPlayer",       NULL,       "NSIS"},
    {"mpc-hc",     "MPC-HC",          NULL,       "NSIS"},
    // === 办公 ===
    {"wps",        "WPS Office",      "Kingsoft", "NSIS"},
    {"office",     NULL,              "Microsoft","OfficeMsi"},
    {"libreoffice","LibreOffice",     NULL,       "MSI"},
    // === 开发工具 ===
    {"python",     "Python",          "Python",   "MSI"},
    {"vscode",     "Visual Studio Code","Microsoft","InnoSetup"},
    {"code",       "Visual Studio Code","Microsoft","InnoSetup"},
    {"visualstudio","Visual Studio",  "Microsoft", "VSBootstrapper"},
    {"jdk",        "Java SE",         "Oracle",   "InstallShield"},
    {"jre",        "Java",            "Oracle",   "InstallShield"},
    {"java",       "Java",            "Oracle",   "InstallShield"},
    {"git",        "Git",             NULL,       "InnoSetup"},
    {"node",       "Node.js",         "Node.js",  "MSI"},
    // === 运行时 ===
    {"vcredist",   NULL,              "Microsoft", "MSI"},
    {"netframework","Microsoft .NET", "Microsoft", "MSI"},
    {"ndp",        "Microsoft .NET",  "Microsoft", "MSI"},
    {"dotnet",     "Microsoft .NET",  "Microsoft", "MSI"},
    {"directx",    "DirectX",         "Microsoft", "Custom"},
    // === IM / 通讯 ===
    {"wechat",     "微信",            "Tencent",  "NSIS"},
    {"wx",         NULL,              "Tencent",  "NSIS"},
    {"qq",         "QQ",              "Tencent",  "NSIS"},
    {"dingtalk",   "钉钉",            "Alibaba",  "NSIS"},
    {"dingding",   "钉钉",            NULL,       "NSIS"},
    {"feishu",     "飞书",            "ByteDance","NSIS"},
    {"lark",       "飞书",            "ByteDance","NSIS"},
    {"teams",      "Teams",           "Microsoft","InstallShield"},
    {"discord",    "Discord",         "Discord",  "NSIS"},
    // === 安全 ===
    {"360",        NULL,              "Qihoo",    "NSIS"},
    {"huorong",    "火绒",            NULL,       "NSIS"},
    // === 编辑器 ===
    {"notepad",    "Notepad++",       NULL,       "NSIS"},
    {"sublime",    "Sublime Text",    NULL,       "InnoSetup"},
    // === 远程 ===
    {"teamviewer","TeamViewer",       "TeamViewer","NSIS"},
    {"anydesk",    "AnyDesk",         NULL,       "Custom"},
    {"todesk",     "ToDesk",          NULL,       "NSIS"},
    // === 打印机 ===
    {NULL,         "Installer Module","CANON INC.","Custom"},  // Canon 打印机驱动(通过数字签名匹配)

    // === 其他 ===
    {"everything", NULL,              "voidtools", "NSIS"},
    {"obs",        "OBS Studio",      NULL,       "NSIS"},
    // === Adobe 系列（仅PE版本信息匹配，不参与文件名匹配，防止误判） ===
    {NULL,         "Adobe Installer", "Adobe",    "AdobeSetup"},
    {"adobe",      NULL,              "Adobe",    "InstallShield"},
    {"reader",     "Adobe Reader",    "Adobe",    "InstallShield"},

    {NULL, NULL, NULL, NULL} // sentinel
};

// === 断代参数表（按PE时间戳年份自动选择参数） ===
// Adobe Setup: 三代安装器
static const SilentParamVariant g_eraAdobe[] = {
    {0,    2012, "\"{file}\" --mode=silent",    "CS3-CS6旧版(≤2012)"},
    {2013, 2019, "\"{file}\" --silent",         "Creative Cloud早期(2013-2019)"},
    {2020, 0,    "\"{file}\" --silent=1",       "Creative Cloud新版(2020+)"},
    {0,    0,    NULL, NULL}  // sentinel
};

// Office: 断代
static const SilentParamVariant g_eraOffice[] = {
    {0,    2009, "msiexec /i \"{file}\" /qn",         "Office 2003-2010(MSI)"},
    {2010, 2017, "\"{file}\" /configure config.xml",  "Office 2010-2016(Click-to-Run)"},
    {2018, 0,    "\"{file}\" /configure config.xml",  "Office 2019+(ODT)"},
    {0,    0,    NULL, NULL}
};

// 根据年份选择静默参数
inline const SilentParamVariant* SelectByEra(const SilentParamVariant* variants, int year) {
    const SilentParamVariant* best = nullptr;
    for (const SilentParamVariant* v = variants; v->command; v++) {
        int from = v->yearFrom, to = v->yearTo;
        if (to == 0) to = 9999;
        if (year >= from && year <= to) return v;  // 精确命中
    }
    // 无匹配返回第一个（兜底）
    return &variants[0];
}

// 根据 installerType 获取断代表
inline const SilentParamVariant* GetEraTable(const char* installerType) {
    if (_stricmp(installerType, "AdobeSetup") == 0) return g_eraAdobe;
    if (_stricmp(installerType, "OfficeMsi") == 0) return g_eraOffice;
    return nullptr;
}

// 辅助：宽字串转小写ASCII
static std::string ToLower(const std::wstring& ws) {
    std::string s;
    for (wchar_t c : ws) s += (char)tolower((unsigned char)c);
    return s;
}

// 匹配知名软件
// 匹配优先级：数字签名 > PE版本信息 > 文件名 > 文件夹名
struct KSMatch {
    const KnownSoft* ks;
    double confidence;
    KSMatch() : ks(nullptr), confidence(0) {}
    KSMatch(const KnownSoft* k, double c) : ks(k), confidence(c) {}
};
inline KSMatch KS_FindByPattern(const std::wstring& filePath) {
    size_t lastSlash = filePath.find_last_of(L"\\/");
    std::wstring fileName = (lastSlash != std::wstring::npos)
        ? filePath.substr(lastSlash + 1) : filePath;
    size_t dot = fileName.rfind(L'.');
    std::wstring baseName = (dot != std::wstring::npos) ? fileName.substr(0, dot) : fileName;
    std::string lowerFull = ToLower(fileName);
    std::string lowerBase = ToLower(baseName);

    std::string lowerPath;
    if (lastSlash != std::wstring::npos) {
        std::wstring dirPart = filePath.substr(0, lastSlash);
        size_t prevSlash = dirPart.find_last_of(L"\\/");
        lowerPath = (prevSlash != std::wstring::npos)
            ? ToLower(dirPart.substr(prevSlash + 1)) + "|" + ToLower(dirPart)
            : ToLower(dirPart);
    }

    // 一、数字签名匹配（0.75）
    std::string sigCompany = ReadSignatureCompany(filePath);
    if (!sigCompany.empty()) {
        for (const KnownSoft* ks = g_knownSoftware;
             !(ks->pattern == NULL && ks->productName == NULL && ks->companyName == NULL && ks->installerType == NULL); ks++) {
            if (ks->companyName) {
                std::string cn(ks->companyName);
                for (auto& c : cn) c = (char)tolower((unsigned char)c);
                if (sigCompany.find(cn) != std::string::npos)
                    return KSMatch(ks, 0.75);
            }
        }
    }

    // 二、PE版本信息匹配（0.75）
    DWORD dummy;
    DWORD verSize = GetFileVersionInfoSizeW(filePath.c_str(), &dummy);
    if (verSize > 0) {
        std::vector<BYTE> verData(verSize);
        if (GetFileVersionInfoW(filePath.c_str(), 0, verSize, verData.data())) {
            struct LangAndCodePage { WORD lang, codePage; } *translations;
            UINT transLen = 0;
            if (VerQueryValueW(verData.data(), L"\\VarFileInfo\\Translation",
                               (void**)&translations, &transLen) && transLen > 0) {
                wchar_t subBlock[256];
                auto getVerStr = [&](const wchar_t* field) -> std::string {
                    swprintf(subBlock, 256, L"\\StringFileInfo\\%04x%04x\\%ls",
                             translations[0].lang, translations[0].codePage, field);
                    wchar_t* val = nullptr; UINT valLen = 0;
                    if (VerQueryValueW(verData.data(), subBlock, (void**)&val, &valLen) && valLen > 0) {
                        std::string s;
                        for (UINT i = 0; i < valLen && val[i]; i++)
                            s += (char)tolower((unsigned char)val[i]);
                        return s;
                    }
                    return "";
                };
                std::string prodName = getVerStr(L"ProductName");
                std::string compName = getVerStr(L"CompanyName");
                int bestScore = 0;
                const KnownSoft* bestPE = nullptr;
                for (const KnownSoft* ks = g_knownSoftware;
                     !(ks->pattern == NULL && ks->productName == NULL && ks->companyName == NULL && ks->installerType == NULL); ks++) {
                    int score = 0;
                    if (ks->productName && !prodName.empty()) {
                        std::string pn(ks->productName);
                        for (auto& c : pn) c = (char)tolower((unsigned char)c);
                        if (prodName.find(pn) != std::string::npos) score = 2;
                    }
                    if (ks->companyName && !compName.empty()) {
                        std::string cn(ks->companyName);
                        for (auto& c : cn) c = (char)tolower((unsigned char)c);
                        if (compName.find(cn) != std::string::npos && score < 1) score = 1;
                    }
                    if (score > bestScore) { bestScore = score; bestPE = ks; }
                }
                if (bestPE) return KSMatch(bestPE, 0.75);
            }
        }
    }

    // 三、文件名匹配（0.65）
    const KnownSoft* bestFile = nullptr; int bestLen = 0;
    for (const KnownSoft* ks = g_knownSoftware;
         !(ks->pattern == NULL && ks->productName == NULL && ks->companyName == NULL && ks->installerType == NULL); ks++) {
        if (!ks->pattern) continue;
        std::string pat(ks->pattern);
        if (lowerBase == pat) return KSMatch(ks, 0.65);
        if (lowerFull.find(pat) != std::string::npos || lowerBase.find(pat) != std::string::npos) {
            if ((int)pat.size() > bestLen) { bestLen = (int)pat.size(); bestFile = ks; }
        }
    }
    if (bestFile) return KSMatch(bestFile, 0.65);

    // 四、文件夹名匹配（0.55）
    if (!lowerPath.empty()) {
        for (const KnownSoft* ks = g_knownSoftware;
             !(ks->pattern == NULL && ks->productName == NULL && ks->companyName == NULL && ks->installerType == NULL); ks++) {
            if (!ks->pattern) continue; // 修复：跳过无pattern的条目，但不停止循环
            std::string pat(ks->pattern);
            for (auto& c : pat) c = (char)tolower((unsigned char)c);
            if (lowerPath.find(pat) != std::string::npos)
                return KSMatch(ks, 0.55);
        }
    }

    return KSMatch();
}

#endif // KNOWNSOFTWARE_H
