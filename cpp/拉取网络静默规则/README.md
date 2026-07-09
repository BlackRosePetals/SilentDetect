# 拉取网络静默规则

每月运行一次，从微软 winget-pkgs 仓库提取 InstallerType → 静默参数映射。

## 使用

```bash
# 自动克隆 winget-pkgs（首次约 50MB），生成 database_winget.h
python fetch_winget.py

# 或指定已有仓库路径
python fetch_winget.py /path/to/winget-pkgs
```

## 输出

`../database_winget.h` — C 头文件，包含社区验证的 InstallerType → 静默参数映射表。

## 依赖

```bash
pip install pyyaml
```

## GitHub Action

每月自动执行，编译并发布新版 exe。
