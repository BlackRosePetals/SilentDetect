#r "D:/静默参数查询/SilentParamQuery/bin/Debug/SilentParamQuery.exe"
using SilentParamQuery.Core;
var engine = new DetectorEngine();
string[] files = {
    @"D:\静默参数查询\Audition2020.exe",
    @"D:\静默参数查询\Update.exe",
    @"D:\静默参数查询\swftojpg.exe"
};
foreach (var f in files) {
    var result = engine.Scan(f);
    Console.Write(System.IO.Path.GetFileName(f) + ": ");
    if (result.Success)
        Console.WriteLine(result.InstallerFullName + " (" + result.InstallerType + ") via " + result.DetectedBy);
    else
        Console.WriteLine("未识别 - " + result.ErrorMessage);
}
