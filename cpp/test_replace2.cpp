#include <windows.h>
#include <string>
#include <vector>
#include <cstdio>

static std::vector<unsigned char> ReadFileBytes(const std::wstring& filePath) {
    FILE* f = _wfopen(filePath.c_str(), L"rb");
    if (!f) return {};
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<unsigned char> data(sz);
    fread(data.data(), 1, sz, f);
    fclose(f);
    return data;
}

static bool HasUtf8Bom(const std::wstring& filePath) {
    FILE* f = _wfopen(filePath.c_str(), L"rb");
    if (!f) return false;
    unsigned char bom[3] = {};
    size_t read = fread(bom, 1, 3, f);
    fclose(f);
    return (read >= 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF);
}

static size_t FindCaseInsensitive(const std::string& haystack, const std::string& needle, size_t start = 0) {
    if (needle.empty()) return start;
    std::string h = haystack;
    for (auto& c : h) c = tolower((unsigned char)c);
    std::string n = needle;
    for (auto& c : n) c = tolower((unsigned char)c);
    return h.find(n, start);
}

static std::string GetReplacement(bool isUtf8) {
    const wchar_t* wide = L"echo 暂停已消除";
    if (isUtf8) {
        int len = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
        std::string s(len - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide, -1, &s[0], len, nullptr, nullptr);
        return s;
    } else {
        int len = WideCharToMultiByte(936, 0, wide, -1, nullptr, 0, nullptr, nullptr);
        std::string s(len - 1, '\0');
        WideCharToMultiByte(936, 0, wide, -1, &s[0], len, nullptr, nullptr);
        return s;
    }
}

int main() {
    wchar_t path[MAX_PATH];
    wcscpy(path, L"D:\静默参数查询\临时文件夹\测试pause.bat");
    
    auto data = ReadFileBytes(path);
    if (data.empty()) {
        printf("Cannot read file\n");
        return 1;
    }
    
    bool isUtf8 = HasUtf8Bom(path);
    size_t bodyStart = isUtf8 ? 3 : 0;
    std::string body(data.begin() + bodyStart, data.end());
    
    printf("Original content:\n%s\n", body.c_str());
    
    std::string replacement = GetReplacement(isUtf8);
    printf("\nReplacement: %s\n", replacement.c_str());
    
    size_t pos = 0;
    while ((pos = FindCaseInsensitive(body, "pause", pos)) != std::string::npos) {
        body.replace(pos, 5, replacement);
        pos += replacement.size();
    }
    
    printf("\nResult:\n%s\n", body.c_str());
    
    // Write result
    wchar_t newPath[MAX_PATH];
    wcscpy(newPath, L"D:\静默参数查询\临时文件夹\测试pause静默.bat");
    FILE* f = _wfopen(newPath, L"wb");
    if (f) {
        fwrite(body.c_str(), 1, body.size(), f);
        fclose(f);
        printf("\nWritten to: 测试pause静默.bat\n");
    }
    
    return 0;
}
