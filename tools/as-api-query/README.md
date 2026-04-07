# tools/as-api-query

本目录为 **ASApiQuery** 插件配套的「一键启用 + 命令行查询」工具。

> 目标：把上层 README 里提到的 `npm run as-api:enable-plugin` / `npm run as-api:query` 做到**仓库自洽**，拷贝到其它工程时也能直接用。

## 需要什么

- Node.js 18+（推荐 20+）
- PowerShell / bash 都可（只是运行 `node`）

## 一句话初始化（启用插件到目标工程）

在 **本仓库根目录** 执行（路径按你的工程改）：

```bash
npm run as-api:enable-plugin -- --uproject "D:/Path/To/YourProject.uproject"
```

该命令会确保 `.uproject` 的 `Plugins[]` 中包含：

```json
{ "Name": "ASApiQuery", "Enabled": true }
```

## 查询（需要先打开 UE Editor）

1. 用 **UnrealEditor** 打开你的工程（非 commandlet、非 unattended）。
2. 等工程根出现：`.as-api-query/instance.json`
3. 在工程根执行：

```bash
npm run as-api:query -- health
npm run as-api:query -- list_types --filter UAnimInstance
npm run as-api:query -- list_symbols --filter LineTrace
```

> `--health` / `list_types` / `list_symbols` 最终都会调用实例文件中记录的 `host/port/basePath`。

## 契约

权威契约：[`REQUIREMENT.md`](REQUIREMENT.md)。

