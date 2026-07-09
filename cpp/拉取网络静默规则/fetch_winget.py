#!/usr/bin/env python3
"""
从 winget-pkgs 仓库提取 InstallerType → 静默参数映射，生成 database_winget.h
用法: python fetch_winget.py [winget-pkgs目录路径]
      如果不指定路径，自动浅克隆 winget-pkgs 仓库
"""

import os
import sys
import subprocess
import yaml
from collections import defaultdict
from pathlib import Path

# InstallerType 映射：winget名称 → 我们的内部名称
TYPE_MAP = {
    "inno": "InnoSetup",
    "nullsoft": "NSIS",
    "nsis": "NSIS",
    "msi": "MSI",
    "wix": "WiXMSI",
    "burn": "WiXBurn",
    "exe": None,       # exe可能是各种类型，不处理
    "portable": None,  # 便携版不需静默参数
    "zip": None,
    "msix": "MSIX",
    "appx": "APPX",
}

OUTPUT_FILE = "database_winget.h"

def find_manifest_files(repo_path):
    """扫描仓库目录找 installer.yaml 文件"""
    manifests = []
    root = Path(repo_path) / "manifests"
    if not root.exists():
        print(f"错误: 找不到 manifests 目录 ({root})")
        return manifests
    for yaml_file in root.rglob("*.installer.yaml"):
        manifests.append(yaml_file)
    print(f"找到 {len(manifests)} 个 installer.yaml 文件")
    return manifests

def parse_manifest(filepath):
    """解析单个 winget manifest，提取 InstallerType + Switches"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            data = yaml.safe_load(f)
    except Exception as e:
        return None

    if not data or "InstallerType" not in data:
        return None

    itype = data.get("InstallerType", "").lower()
    if itype not in TYPE_MAP or TYPE_MAP[itype] is None:
        return None

    internal_type = TYPE_MAP[itype]
    result = {
        "type": internal_type,
        "silent": None,
        "silent_progress": None,
        "product": data.get("PackageIdentifier", "Unknown"),
        "version": data.get("PackageVersion", ""),
    }

    # 提取 InstallerSwitches（可能在根级别或 Installers 数组内）
    switches = data.get("InstallerSwitches", {})
    if not switches:
        installers = data.get("Installers", [])
        if installers:
            switches = installers[0].get("InstallerSwitches", {})

    result["silent"] = switches.get("Silent", None)
    result["silent_progress"] = switches.get("SilentWithProgress", None)
    result["install_location"] = switches.get("InstallLocation", None)
    result["custom"] = switches.get("Custom", None)

    return result if result["silent"] or result["silent_progress"] else None

def collect_by_type(manifests):
    """按 InstallerType 汇总所有静默参数，去重计数"""
    type_params = defaultdict(lambda: defaultdict(int))

    for mf in manifests:
        data = parse_manifest(mf)
        if not data:
            continue

        itype = data["type"]
        for key in ["silent", "silent_progress", "custom", "install_location"]:
            val = data.get(key)
            if val:
                type_params[itype][val] += 1

    return type_params

def generate_header(type_params):
    """生成 database_winget.h"""
    lines = []
    lines.append("// ============================================================")
    lines.append("// database_winget.h — 自动生成于 winget-pkgs 仓库")
    lines.append("// 每月更新一次，请勿手动编辑")
    lines.append("// ============================================================")
    lines.append("")
    lines.append("#ifndef DATABASE_WINGET_H")
    lines.append("#define DATABASE_WINGET_H")
    lines.append("")
    lines.append("// winget 社区验证的 InstallerType → 静默参数映射")
    lines.append("// 格式: {内部类型, 首选参数, 备选参数, 进度参数}")
    lines.append("")

    # 统计
    total_types = len(type_params)
    lines.append(f"// 共 {total_types} 种安装器类型")
    lines.append("")
    lines.append("struct WingetTypeParam {")
    lines.append("    const char* type;")
    lines.append("    const char* primary;       // 最常用参数（出现次数最多）")
    lines.append("    const char* fallback;      // 备选参数（出现次数第二多）")
    lines.append("    const char* withProgress;  // 带进度条的参数")
    lines.append("};")
    lines.append("")
    lines.append("static const WingetTypeParam g_wingetParams[] = {")

    for itype in sorted(type_params.keys()):
        params = type_params[itype]
        # 按出现次数排序
        sorted_params = sorted(params.items(), key=lambda x: x[1], reverse=True)
        primary = sorted_params[0][0] if len(sorted_params) > 0 else None
        fallback = sorted_params[1][0] if len(sorted_params) > 1 else None

        # 找 SilentWithProgress
        progress = [v for v in params.keys() if v != primary and v != fallback]
        with_progress = progress[0] if progress else None

        count = sum(params.values())

        if primary:
            escaped = primary.replace('"', '\\"')
            fallback_esc = fallback.replace('"', '\\"') if fallback else ""
            prog_esc = with_progress.replace('"', '\\"') if with_progress else ""
            lines.append(f'    {{"{itype}", "{escaped}", "{fallback_esc}", "{prog_esc}"}},'
                        f'  // {count}个软件使用')

    lines.append("    {NULL, NULL, NULL, NULL}  // sentinel")
    lines.append("};")
    lines.append("")
    lines.append("// 按 InstallerType 查找 winget 推荐的静默参数")
    lines.append("inline const WingetTypeParam* Winget_FindByType(const char* type) {")
    lines.append("    for (const WingetTypeParam* p = g_wingetParams; p->type; p++) {")
    lines.append("        if (_stricmp(p->type, type) == 0) return p;")
    lines.append("    }")
    lines.append("    return nullptr;")
    lines.append("}")
    lines.append("")
    lines.append("#endif // DATABASE_WINGET_H")
    lines.append("")

    return "\n".join(lines)

def main():
    repo_path = sys.argv[1] if len(sys.argv) > 1 else None

    if not repo_path:
        # 尝试浅克隆 winget-pkgs（仅 manifests 目录）
        print("浅克隆 winget-pkgs 仓库（仅 manifests，约50MB）...")
        repo_path = "_winget_pkgs"
        if not os.path.exists(repo_path):
            cmd = [
                "git", "clone", "--depth", "1",
                "--filter=blob:none", "--sparse",
                "https://github.com/microsoft/winget-pkgs.git",
                repo_path
            ]
            subprocess.run(cmd, check=True)
            subprocess.run(["git", "-C", repo_path, "sparse-checkout", "set", "manifests"],
                          check=True)
        else:
            print(f"使用已有仓库: {repo_path}")
            # 更新
            subprocess.run(["git", "-C", repo_path, "pull", "--depth", "1"],
                          check=False)

    manifests = find_manifest_files(repo_path)
    if not manifests:
        print("未找到 manifest 文件，退出")
        return

    print(f"开始解析...")
    type_params = collect_by_type(manifests)

    print(f"\n=== InstallerType 统计 ===")
    for itype in sorted(type_params.keys()):
        params = type_params[itype]
        total = sum(params.values())
        top = sorted(params.items(), key=lambda x: x[1], reverse=True)
        print(f"  {itype}: {total}次, 常用参数: {top[:3]}")

    header = generate_header(type_params)
    output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", OUTPUT_FILE)
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(header)
    print(f"\n已生成: {output_path}")

if __name__ == "__main__":
    main()
