function getHubHost() {
  const ENV_API_HOST = import.meta.env.VITE_API_HOST;
  return ENV_API_HOST ?? window.document.location.hostname;
}

function isApiHostEnvIsProvided() {
  return import.meta.env.VITE_API_HOST !== undefined;
}

function getHubPort(): number | undefined {
  const port = Number.parseInt(window.document.location.port, 10);
  if (Number.isNaN(port)) {
    return undefined;
  }
  return port;
}

export function getHubHttpApiEndpoint(
  path: string | undefined,
  search?: Record<string, string>,
) {
  const hubHost = getHubHost();
  const hubPort = getHubPort();
  const url = new URL(`http://${hubHost}`);
  if (isApiHostEnvIsProvided()) {
    url.port = ""; // for `npm run dev` to work
  } else if (hubPort !== undefined) {
    url.port = hubPort.toString();
  }
  if (path !== undefined) {
    url.pathname = path;
  }
  if (search) {
    for (const [key, value] of Object.entries(search)) {
      url.searchParams.append(key, value);
    }
  }
  return url.href;
}

export function getHubWebSocketEndpoint() {
  const hubHost = getHubHost();
  const hubPort = getHubPort();
  const url = new URL(`ws://${hubHost}`);
  if (isApiHostEnvIsProvided()) {
    url.port = ""; // for `npm run dev` to work
  } else if (hubPort !== undefined) {
    url.port = hubPort.toString(); // for port forwarding
  }
  url.pathname = "/ws";
  return url.href;
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
    .map((x) => Number.parseInt(x, 10))
    .reduce((prev, cur) => prev * 1000 + cur, 0);
}
