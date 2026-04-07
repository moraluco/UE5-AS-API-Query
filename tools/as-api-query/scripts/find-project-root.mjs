import { existsSync } from 'node:fs';
import { readdir } from 'node:fs/promises';
import { dirname, join, resolve } from 'node:path';

/**
 * Walk up from startDir to find a directory containing a *.uproject.
 * @param {string} startDir
 * @returns {{ root: string, uprojectPath: string }}
 */
export async function findProjectRootWithUproject(startDir) {
  let dir = resolve(startDir);
  while (true) {
    try {
      const entries = await readdir(dir);
      const uproject = entries.find((e) => e.toLowerCase().endsWith('.uproject'));
      if (uproject) {
        return { root: dir, uprojectPath: join(dir, uproject) };
      }
    } catch {
      // ignore
    }
    const parent = dirname(dir);
    if (!parent || parent === dir) break;
    dir = parent;
  }
  throw new Error(`Could not find any .uproject above: ${startDir}`);
}

/**
 * Find instance.json given a project root.
 * @param {string} projectRoot
 */
export function getInstanceJsonPath(projectRoot) {
  return join(projectRoot, '.as-api-query', 'instance.json');
}

export function fileExists(p) {
  return existsSync(p);
}

