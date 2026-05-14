#ifndef BATGEN_H
#define BATGEN_H

#include <string>
#include "scanner.h"

struct BatchOptions {
    bool includeLog;
    bool includeErrorHandling;
    bool includeUac;
    bool includeComments;

    BatchOptions() : includeLog(true), includeErrorHandling(true), includeUac(true), includeComments(true) {}
};

std::string GenerateBatch(const ScanResult& result, const BatchOptions& options);
bool SaveBatchFile(const std::string& content, const std::wstring& filePath);

#endif // BATGEN_H
