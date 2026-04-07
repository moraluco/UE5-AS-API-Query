# ASApiQuery HTTP/JSON 契约（简版）

## 目的

在 Unreal Editor 内提供本机 HTTP 服务，查询 **当前会话** AngelScript 已注册的类型/符号（与运行时绑定一致）。

## 实例发现

服务启动后会在 **工程根** 写入：

`.as-api-query/instance.json`

示例：

```json
{
  "schemaVersion": 1,
  "host": "127.0.0.1",
  "port": 18080,
  "basePath": "/as-api"
}
```

## Endpoints

以 `base = http://{host}:{port}{basePath}` 为前缀：

- `GET {base}/health`
  - 返回：`{ schemaVersion: 1, ok: true, data: { name, version } }`

- `POST {base}/v1/query`
  - body：`{ schemaVersion: 1, action: string, filter?: string }`
  - actions：
    - `list_types`：返回 `data.types[]`（`name`）
    - `list_symbols`：返回 `data.symbols[]`（`name`，如 `System::PrintString()`）
  - 当 Angelscript 未初始化时：`ok=false` / `error.code = not_initialized`（HTTP 200）

## 环境变量

服务端：

- `AS_API_QUERY_PORT`：覆盖默认监听端口（默认 18080）

