---
name: as-api-dynamic-query
description: Dynamic AngelScript API lookup via the ASApiQuery editor HTTP service and npm script as-api:query (project-local, not SoftUEBridge). Use when the user needs symbols bound in the current editor session or when verifying API exposure; fallback to angelscript-api-query (Doc/Hazelight) when the editor is not running or CLI fails. For porting ASApiQuery to another project, see Plugins/ASApiQuery/README.md.
---

# AS 动态 API 查询（Skill + CLI）

> **分发副本**：本文件与仓库根 `.cursor/skills/as-api-dynamic-query/SKILL.md` 同步维护；随插件一并拷贝。安装到 Cursor 的方式见同目录上一级的 [`CursorSkill_分发说明.md`](../../CursorSkill_分发说明.md)。

## 何时使用

- 需要 **当前工程 + 当前编辑器** 下 AngelScript **已注册类型** 等与运行时绑定一致的信息。
- 与 **SoftUEBridge** 无关；不要通过 `/bridge` 查 AS API。

## 新工程 / 移植

1. 先读 **`Plugins/ASApiQuery/README.md`**（前置条件、拷贝范围、`.uproject`、编译、验收；可选 `npm run as-api:enable-plugin -- <path/to/Project.uproject>`）。
2. 再读 **`tools/as-api-query/README.md`**（npm 片段、环境变量）。
3. 契约仍以 **`tools/as-api-query/REQUIREMENT.md`** 为准。

## 流程（已集成时）

1. **默认优先动态查询**：先用 ASApiQuery 查询“当前会话已注册符号”。只有动态查询不可用才回落静态文档（`angelscript-api-query`）。
2. **启动 UE 编辑器** 并打开目标工程（非 commandlet、非 unattended）；服务地址以工程根 **`.as-api-query/instance.json`** 为准（端口可由 `AS_API_QUERY_PORT` 覆盖）。
3. 在工程根执行：
   - `npm run as-api:query -- health`
   - `npm run as-api:query -- list_types --filter UAnimInstance`
   - `npm run as-api:query -- list_symbols --filter LineTrace`
4. **失败回落**：无实例文件/连接拒绝/AS 未初始化时，回落 **`angelscript-api-query`**（Kit 本地 AS_API → Hazelight 站点）。

## 契约与细节

- 单一事实来源：`tools/as-api-query/REQUIREMENT.md`。
- CLI：`tools/as-api-query/cli.mjs`；测试：`tools/as-api-query/*.test.mjs`（`npm test`）。

## 与 write-angelscript

- 编写 `.as` 时：**优先** 用本技能得到 **当前绑定**；文档站用于补充说明。
