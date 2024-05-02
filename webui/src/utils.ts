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
