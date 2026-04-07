import { readFile, writeFile } from 'node:fs/promises';
import { resolve } from 'node:path';

function usage() {
  process.stderr.write(`Usage:
  npm run as-api:enable-plugin -- --uproject "<path/to/YourProject.uproject>"

Edits the .uproject JSON and ensures Plugins[] contains:
  { "Name": "ASApiQuery", "Enabled": true }
`);
}

function parseArgs(argv) {
  let uproject = '';
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    if ((a === '--uproject' || a === '--uproject') && argv[i + 1]) {
      uproject = argv[++i];
      continue;
    }
    if (a === '-h' || a === '--help') {
      return { help: true, uproject: '' };
    }
  }
  return { help: false, uproject };
}

function ensurePluginEntry(plugins) {
  if (!Array.isArray(plugins)) return [{ Name: 'ASApiQuery', Enabled: true }];
  const idx = plugins.findIndex((p) => p && (p.Name === 'ASApiQuery' || p.Name === 'ASApiQuery '));
  if (idx >= 0) {
    plugins[idx].Enabled = true;
    plugins[idx].Name = 'ASApiQuery';
    return plugins;
  }
  return [...plugins, { Name: 'ASApiQuery', Enabled: true }];
}

async function main() {
  const { help, uproject } = parseArgs(process.argv.slice(2));
  if (help || !uproject) {
    usage();
    process.exit(help ? 0 : 2);
  }

  const path = resolve(uproject);
  const raw = await readFile(path, 'utf8');
  const json = JSON.parse(raw);
  json.Plugins = ensurePluginEntry(json.Plugins);
  const out = JSON.stringify(json, null, 2) + '\n';
  await writeFile(path, out, 'utf8');
  process.stdout.write(`OK: enabled ASApiQuery in ${path}\n`);
}

main().catch((e) => {
  process.stderr.write(`${e instanceof Error ? e.message : String(e)}\n`);
  process.exit(1);
});

