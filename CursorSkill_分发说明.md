# Cursor Skill 分发说明（as-api-dynamic-query）

本插件目录内附带 **与仓库同步的 Cursor Skill** 副本，便于随插件拷贝到其它工程时，**不必单独找 AS-learn 仓库**即可安装 Agent 技能。

## 副本位置

| 路径 | 说明 |
|------|------|
| `Plugins/ASApiQuery/.cursor/skills/as-api-dynamic-query/SKILL.md` | 技能正文（与 AS-learn 根目录 `.cursor/skills/as-api-dynamic-query/SKILL.md` 一致维护） |

## 安装到目标工程（任选）

**方式 A — 只装本技能**

1. 在**游戏工程仓库根**（与 `.uproject` 同级）确保存在 `.cursor/skills/`。
2. 将 `as-api-dynamic-query` **整个文件夹**复制到：
   - `YourProject/.cursor/skills/as-api-dynamic-query/`
3. 确认该目录内有 `SKILL.md`（即本文件所在插件内的 `SKILL.md`）。

**方式 B — 合并整个 `.cursor` 片段**

若你希望保留插件自带的相对路径结构，可将 `Plugins/ASApiQuery/.cursor/` 下内容**合并**进工程根 `.cursor/`（注意与已有 `skills` 目录冲突时只合并子目录 `as-api-dynamic-query`）。

## 与主仓库的关系

- **权威来源**：AS-learn 仓库中 **`.cursor/skills/as-api-dynamic-query/SKILL.md`**。
- 更新插件时，请同步更新本副本；或在 PR/发布说明中注明「Skill 是否有变更」。

## 依赖（本 Skill 未单独包含的部分）

- **CLI 与契约**：仍需将仓库中的 **`tools/as-api-query/`** 一并纳入目标工程，并配置 `package.json` 中 `as-api:query` 等脚本，详见 **`tools/as-api-query/README.md`**。
- **回落技能 `angelscript-api-query`**：若目标工程没有 `Doc/API_Docs` 与 Hazelight 流程，失败时 Agent 可改为手动查线上文档；该 Skill 可选单独安装。

## 插件自述

HTTP 服务、编译与验收见同目录 **[README.md](README.md)**。
