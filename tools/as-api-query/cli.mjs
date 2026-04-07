import process from 'node:process';
import { readFile } from 'node:fs/promises';
import { resolve } from 'node:path';

import { findProjectRootWithUproject, getInstanceJsonPath, fileExists } from './scripts/find-project-root.mjs';

function usage(stderr) {
  stderr.write(`Usage:
  npm run as-api:query -- health
  npm run as-api:query -- list_types --filter <substring>
  npm run as-api:query -- list_symbols --filter <substring>
  npm run as-api:query -- query '<json>'

Options:
  --project-root <dir>   Override project root (default: search upwards for *.uproject)
  --instance <path>      Override instance.json path
  --filter <string>      Filter for list_types/list_symbols
`);
}

function parseArgs(argv) {
  let cmd = '';
  let filter = '';
  let projectRoot = '';
  let instance = '';
  let rawQuery = '';

  const rest = [];
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    if (a === '--filter' && argv[i + 1]) {
      filter = argv[++i];
      continue;
    }
    if (a === '--project-root' && argv[i + 1]) {
      projectRoot = argv[++i];
      continue;
    }
    if (a === '--instance' && argv[i + 1]) {
      instance = argv[++i];
      continue;
    }
    rest.push(a);
  }

  cmd = rest[0] ?? '';
  if (cmd === 'query') rawQuery = rest[1] ?? '';
  return { cmd, filter, projectRoot, instance, rawQuery };
}

async function readInstanceJson(instancePath) {
  const raw = await readFile(instancePath, 'utf8');
  const j = JSON.parse(raw);
  if (!j.host || !j.port || !j.basePath) {
    throw new Error(`Invalid instance.json (missing host/port/basePath): ${instancePath}`);
  }
  return j;
}

async function main() {
  const argv = process.argv.slice(2);
  if (argv.length === 0 || argv.includes('-h') || argv.includes('--help')) {
    usage(process.stderr);
    process.exit(0);
  }

  const { cmd, filter, projectRoot, instance, rawQuery } = parseArgs(argv);
  if (!cmd) {
    usage(process.stderr);
    process.exit(2);
  }

  const root =
    projectRoot ? resolve(projectRoot) : (await findProjectRootWithUproject(process.cwd())).root;
  const instancePath = instance ? resolve(instance) : getInstanceJsonPath(root);
  if (!fileExists(instancePath)) {
    throw new Error(
      `instance.json not found: ${instancePath}\nOpen UnrealEditor with the project (non-unattended) and wait for .as-api-query/instance.json.`,
    );
  }

  const inst = await readInstanceJson(instancePath);
  const base = `http://${inst.host}:${inst.port}${inst.basePath}`;

  if (cmd === 'health') {
    const r = await fetch(`${base}/health`);
    const j = await r.json();
    process.stdout.write(`${JSON.stringify(j, null, 2)}\n`);
    return;
  }

  if (cmd === 'list_types' || cmd === 'list_symbols') {
    const body = { schemaVersion: 1, action: cmd, filter: filter ?? '' };
    const r = await fetch(`${base}/v1/query`, {
      method: 'POST',
      headers: { 'content-type': 'application/json' },
      body: JSON.stringify(body),
    });
    const j = await r.json();
    process.stdout.write(`${JSON.stringify(j, null, 2)}\n`);
    return;
  }

  if (cmd === 'query') {
    if (!rawQuery) throw new Error('Missing raw JSON for query');
    const parsed = JSON.parse(rawQuery);
    const r = await fetch(`${base}/v1/query`, {
      method: 'POST',
      headers: { 'content-type': 'application/json' },
      body: JSON.stringify(parsed),
    });
    const j = await r.json();
    process.stdout.write(`${JSON.stringify(j, null, 2)}\n`);
    return;
  }

  usage(process.stderr);
  process.exit(2);
}

main().catch((e) => {
  process.stderr.write(`${e instanceof Error ? e.message : String(e)}\n`);
  process.exit(1);
});

