export function getHubHost() {
  const ENV_API_HOST = import.meta.env.VITE_API_HOST;
  return ENV_API_HOST ?? window.document.location.hostname;
}

function isObject(value: unknown): value is Record<string, unknown> {
  return typeof value === "object" && value !== null;
}

export function isEqual(a: unknown, b: unknown): boolean {
  if (!isObject(a) || !isObject(b)) {
    return a === b;
  }

  const aKeys = Object.keys(a).sort();
  const bKeys = Object.keys(b).sort();

  if (aKeys.length !== bKeys.length) {
    return false;
  }

  for (let i = 0; i < aKeys.length; i++) {
    const key = aKeys[i];
    if (
      !Object.prototype.hasOwnProperty.call(b, key) ||
      !isEqual(a[key], b[key])
    ) {
      return false;
    }
  }

  return true;
}

export function isValidVersion(version: string | undefined): boolean {
  if (version === undefined) {
    return false;
  }
  return /^\d+\.\d+\.\d+$/.test(version);
}

export function getIntVersion(version: string): number {
  return version
    .split(".")
    .map((x) => parseInt(x, 10))
    .reduce((prev, cur) => prev * 1000 + cur, 0);
}
