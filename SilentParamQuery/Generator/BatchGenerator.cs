using System.IO;
using System.Text;
using SilentParamQuery.Core;

namespace SilentParamQuery.Generator
{
    public class BatchOptions
    {
        public bool IncludeLog { get; set; }
        public bool IncludeErrorHandling { get; set; }
        public bool IncludeUac { get; set; }
        public bool IncludeComments { get; set; }
    }

    public static class BatchGenerator
    {
        public static string Generate(ScanResult result, BatchOptions options)
        {
            var sb = new StringBuilder();

            if (options.IncludeComments)
            {
                sb.AppendLine("@REM ============================================================");
                sb.AppendLine($"@REM 静默安装脚本 - 由 SilentParamQuery 生成");
                sb.AppendLine($"@REM 文件: {result.FileName}");
                sb.AppendLine($"@REM 安装器类型: {result.InstallerFullName}");
                sb.AppendLine($"@REM 检测方式: {result.DetectedBy}");
                sb.AppendLine($"@REM 置信度: {result.GetConfidenceStars()}");
                sb.AppendLine("@REM ============================================================");
                sb.AppendLine();
            }

            // ANSI编码声明
            sb.AppendLine("@echo off");
            sb.AppendLine("chcp 936 >nul 2>&1");
            sb.AppendLine();

            if (options.IncludeUac)
            {
                sb.AppendLine("@REM 请求管理员权限");
                sb.AppendLine("NET SESSION >nul 2>&1");
                sb.AppendLine("IF %ERRORLEVEL% NEQ 0 (");
                sb.AppendLine("    echo 正在请求管理员权限...");
                sb.AppendLine("    powershell -Command \"Start-Process '%~f0' -Verb RunAs\"");
                sb.AppendLine("    exit /b");
                sb.AppendLine(")");
                sb.AppendLine();
            }

            if (options.IncludeComments)
            {
                sb.AppendLine("@REM 设置安装文件路径");
            }
            sb.AppendLine($"set \"INSTALLER=%~dp0{result.FileName}\"");
            sb.AppendLine();

            if (options.IncludeErrorHandling)
            {
                sb.AppendLine("@REM 检查安装文件是否存在");
                sb.AppendLine("if not exist \"%INSTALLER%\" (");
                sb.AppendLine("    echo [错误] 未找到安装文件: %INSTALLER%");
                sb.AppendLine("    pause");
                sb.AppendLine("    exit /b 1");
                sb.AppendLine(")");
                sb.AppendLine();
                sb.AppendLine("echo ============================================================");
                sb.AppendLine($"echo  正在静默安装: {result.FileName}");
                sb.AppendLine($"echo  安装器类型: {result.InstallerFullName}");
                sb.AppendLine("echo ============================================================");
                sb.AppendLine();
            }

            // 生成安装命令
            string installCmd = result.SilentCommand.Replace("{file}", "%INSTALLER%");

            if (options.IncludeLog)
            {
                string logPath = "%~dp0install_log.txt";
                if (result.InstallerType == "MSI" || result.InstallerType == "WiXMSI")
                {
                    installCmd = $"msiexec /i \"%INSTALLER%\" /qn /norestart /l*v \"{logPath}\"";
                }
                else if (result.InstallerType == "InnoSetup")
                {
                    installCmd = $"\"%INSTALLER%\" /VERYSILENT /SP- /LOG=\"{logPath}\"";
                }
                else if (result.InstallerType == "InstallShield" || result.InstallerType == "InstallScript")
                {
                    installCmd = $"\"%INSTALLER%\" /s /v\"/qn /l*v {logPath}\"";
                }
                else if (result.InstallerType == "BitRockInstallBuilder")
                {
                    installCmd = $"\"%INSTALLER%\" --mode unattended --debuglevel 4";
                }
            }

            sb.AppendLine($"echo 正在安装...");
            sb.AppendLine(installCmd);
            sb.AppendLine();

            if (options.IncludeErrorHandling)
            {
                sb.AppendLine("@REM 检查安装结果");
                sb.AppendLine("if %ERRORLEVEL% EQU 0 (");
                sb.AppendLine("    echo [成功] 安装完成");
                sb.AppendLine(") else (");
                sb.AppendLine("    echo [失败] 安装出错，错误码: %ERRORLEVEL%");
                sb.AppendLine("    pause");
                sb.AppendLine("    exit /b %ERRORLEVEL%");
                sb.AppendLine(")");
                sb.AppendLine();
            }

            if (options.IncludeLog)
            {
                sb.AppendLine($"echo 安装日志已保存至: install_log.txt");
            }

            sb.AppendLine("echo 安装完成。");
            sb.AppendLine("exit /b 0");

            return sb.ToString();
        }

        public static void SaveToFile(string content, string filePath)
        {
            // 使用 ANSI (GB2312) 编码写入，确保中文批处理兼容
            Encoding ansi = Encoding.GetEncoding(936);
            File.WriteAllText(filePath, content, ansi);
        }
    }
}
