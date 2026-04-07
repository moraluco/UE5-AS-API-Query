# ASApiQuery（AS API Query）

在 **Unreal Editor** 内提供本机 **HTTP** 服务，用于查询当前会话下 **AngelScript 已注册类型/符号** 等与运行时绑定一致的信息。与 **SoftUEBridge** 无关，不占用 `/bridge`。

## 硬前置

- 目标工程必须已启用 **Angelscript** 插件（Unreal Engine AngelScript）。本插件的 `ASApiQuery.uplugin` 声明了对 `Angelscript` 的依赖；**没有 Angelscript 的工程无法使用本插件**。

## 拷贝到其它工程时带什么

从版本库拷贝时，至少包含：

- `ASApiQuery.uplugin`
- `Source/` 目录（全部）

**不要**依赖 `Intermediate/`、`Binaries/`（若从已编译工程里拖目录，请只保留上述源码与 uplugin，或直接从 Git 取干净树）。

## 安装位置

将本目录完整放到：

`YourProject/Plugins/ASApiQuery/`

## 启用插件（.uproject）

在工程 `.uproject` 的 `Plugins` 数组中增加（若已有同名项则把 `Enabled` 设为 `true`）：

```json
{
  "Name": "ASApiQuery",
  "Enabled": true
}
```

**推荐（幂等、少手改）**：在工程仓库根执行（路径按你的仓库调整）：

```bash
npm i
npm run as-api:enable-plugin -- --uproject "path/to/YourProject.uproject"
```

详见仓库内 `tools/as-api-query/scripts/enable-as-api-query-in-uproject.mjs`。

## 编译

- 对目标工程生成 IDE 工程并编译 **Editor**（与常规 C++ 插件相同）。
- 本仓库内可参考技能 `vs-ue-build-as` 与 [`Doc/Workflow_Automated_Development.md`](../../Doc/Workflow_Automated_Development.md)。

## 运行时与验收

1. 用编辑器打开该工程（非 Commandlet / 非无人值守时才会启动 HTTP 服务）。
2. 工程根目录应出现 **`.as-api-query/instance.json`**（记录 host/port/basePath；该目录通常加入 `.gitignore`）。
3. 使用 CLI 或浏览器访问 health（见下文「命令行客户端」）。

### 环境变量（服务端）

- **`AS_API_QUERY_PORT`**：覆盖默认监听端口（默认见契约文档）。

## 命令行客户端（与插件配套）

HTTP/JSON 契约与 Node CLI 在仓库的 `tools/as-api-query/`，自述见 `tools/as-api-query/README.md`。**权威契约**：`tools/as-api-query/REQUIREMENT.md`。

典型验收（在**工程根**、已启动编辑器并加载工程后）：

```bash
npm run as-api:query -- health
npm run as-api:query -- list_types --filter UAnimInstance
npm run as-api:query -- list_symbols --filter LineTrace
```

## Cursor Skill（随插件分发）

- **技能副本**：`.cursor/skills/as-api-dynamic-query/SKILL.md`（与 AS-learn 仓库根同名 Skill 同步维护）。
- **如何拷到目标工程的 `.cursor/`**：见 **[CursorSkill_分发说明.md](CursorSkill_分发说明.md)**。

## 故障排查

| 现象 | 处理 |
|------|------|
| 无 `instance.json` | 确认用编辑器打开的是该工程；非无人模式；插件已编译并启用 |
| 连接被拒绝 | 以 `instance.json` 中端口为准；检查端口占用或改 `AS_API_QUERY_PORT` 后重启编辑器 |
| AS 未初始化 | 等工程与脚本加载完成后再查；部分错误码见 REQUIREMENT 中 `not_initialized` |
