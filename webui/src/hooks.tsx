import { ChangeEvent, useCallback, useEffect, useMemo, useState } from "react";
import useWebSocket, { ReadyState } from "react-use-websocket";

import {
  Button,
  ButtonJson,
  Command,
  ConvertButtonJsonToButton,
  HubInfo,
  Location,
  RobotInfo,
  Settings,
  Shelf,
} from "./types";

export function useWiFiSettings(
  hubHost: string,
): [
  string | undefined,
  string | undefined,
  (ssid: string, pass: string) => Promise<void>,
] {
  const [ssid, setSsid] = useState<string>();
  const [pass, setPass] = useState<string>();
  useEffect(() => {
    fetch(`http://${hubHost}/config/wifi`)
      .then((res) => res.json())
      .then((data) => {
        setSsid(data.ssid);
        setPass(data.pass);
      });
  }, [hubHost]);
  const setWiFiSettings = useCallback(
    (newSsid: string, newPass: string) =>
      fetch(`http://${hubHost}/config/wifi`, {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ ssid: newSsid.trim(), pass: newPass.trim() }),
      }).then(() => {
        setSsid(newSsid);
        setPass(newPass);
      }),
    [hubHost],
  );
  return [ssid, pass, setWiFiSettings];
}

export function useInput(
  defaultValue: string,
  onChange?: (value: string) => void,
) {
  const [value, setValue] = useState(defaultValue);
  const onChangeImpl = (e: ChangeEvent<HTMLInputElement>) => {
    setValue(e.target.value);
    onChange?.(e.target.value);
  };
  return { value, onChange: onChangeImpl, type: "text" };
}

export function useNumberInput(
  defaultValue: number,
  onChange?: (value: number) => void,
  options?: { min?: number; max?: number },
) {
  const [value, setValue] = useState(defaultValue);
  const onChangeImpl = (e: ChangeEvent<HTMLInputElement>) => {
    setValue(Number(e.target.value));
    onChange?.(Number(e.target.value));
  };
  return { value, onChange: onChangeImpl, type: "number", ...options };
}

export function useRangeInput(
  value: number | undefined,
  min: number,
  max: number,
  step: number,
  onChange?: (value: number) => void,
) {
  const onChangeImpl = useCallback(
    (e: ChangeEvent<HTMLInputElement>) => {
      const newValue = Number(e.target.value);
      if (value !== newValue && value !== undefined) {
        onChange?.(newValue);
      }
    },
    [value, onChange],
  );
  return {
    value: value ?? min,
    onChange: onChangeImpl,
    type: "range",
    min,
    max,
    step,
    disabled: value === undefined,
  };
}

export function useCheckbox(
  defaultValue: boolean,
  onChange?: (value: boolean) => void,
) {
  const [checked, setChecked] = useState(defaultValue);
  const onChangeImpl = (e: ChangeEvent<HTMLInputElement>) => {
    setChecked(e.target.checked);
    onChange?.(e.target.checked);
  };
  return { checked, onChange: onChangeImpl, type: "checkbox" };
}

interface ShelfSelectOption {
  value: string;
  label: string;
}

export function useShelfSelect(
  shelves: Shelf[],
  defaultValue: string,
  extraOptions: ShelfSelectOption[] = [],
): [string | undefined, JSX.Element] {
  const [selectedShelfId, setSelectedShelfId] = useState<string | undefined>(
    defaultValue,
  );
  const options = useMemo(
    () =>
      shelves.map((shelf) => (
        <option key={shelf.id} value={shelf.id}>
          {shelf.name}
        </option>
      )),
    [shelves],
  );
  const isKnownId = [
    ...shelves.map(({ id }) => id),
    ...extraOptions.map(({ value }) => value),
  ].includes(defaultValue);
  const select = useMemo(
    () => (
      <select
        value={selectedShelfId ?? ""}
        onChange={(e) => setSelectedShelfId(e.target.value)}
        style={{ maxWidth: "30vw" }}
      >
        {!isKnownId && (
          <option value={defaultValue} disabled>
            (不明な家具)
          </option>
        )}
        {extraOptions.map((option) => (
          <option key={option.value} value={option.value}>
            {option.label}
          </option>
        ))}
        {options}
      </select>
    ),
    [extraOptions, options, selectedShelfId, defaultValue, isKnownId],
  );
  return [selectedShelfId, select];
}

export function useLocationSelect(
  locations: Location[],
  defaultValue: string,
): [string | undefined, JSX.Element] {
  const [selectedLocationId, setSelectedLocationId] = useState<
    string | undefined
  >(defaultValue);
  const options = useMemo(
    () =>
      locations.map((location) => (
        <option key={location.id} value={location.id}>
          {location.name}
        </option>
      )),
    [locations],
  );
  const isKnownId = (locations ?? [])
    .map((location) => location.id)
    .includes(defaultValue);
  const select = useMemo(
    () => (
      <select
        value={selectedLocationId ?? ""}
        onChange={(e) => setSelectedLocationId(e.target.value)}
        style={{ maxWidth: "30vw" }}
      >
        {!isKnownId && (
          <option value={defaultValue} disabled>
            (不明な目的地)
          </option>
        )}
        {options}
      </select>
    ),
    [options, selectedLocationId, defaultValue, isKnownId],
  );
  return [selectedLocationId, select];
}

interface HubInfoMessage extends HubInfo {
  type: "hub_info";
}

interface RobotInfoMessage extends RobotInfo {
  type: "robot_info";
}

interface SettingsMessage {
  type: "settings";
  settings: Settings;
}

interface ObservedButtonMessage {
  type: "observed_buttons";
  buttons: ButtonJson[];
  timestamp_now: number;
}

interface CommandsMessage {
  type: "commands";
  commands: Array<{
    button: Button;
    command: Command;
  }>;
  timestamp_now: number;
}

interface WifiRssiMessage {
  type: "wifi_rssi";
  wifi_rssi: number;
}

type WsMessage =
  | HubInfoMessage
  | RobotInfoMessage
  | SettingsMessage
  | ObservedButtonMessage
  | CommandsMessage
  | WifiRssiMessage;

let handle: number | undefined = undefined;

export function useKachakaButtonHub(hubHost: string) {
  const [online, setOnline] = useState(false);
  const [hubInfo, setHubInfo] = useState<HubInfo>();
  const [robotInfo, setRobotInfo] = useState<RobotInfo>();
  const [settings, setSettings] = useState<Settings>();
  const [buttons, setButtons] = useState<Button[]>();
  const [commands, setCommands] =
    useState<{ button: Button; command: Command }[]>();
  const [wifiRssi, setWifiRssi] = useState<number>();

  const onMessage = useCallback((message: WebSocketEventMap["message"]) => {
    if (message?.data === undefined) {
      return;
    }
    const parsedMessage: WsMessage = JSON.parse(message.data ?? "null");
    const now = Date.now();
    if (parsedMessage.type === "hub_info") {
      setHubInfo(parsedMessage);
    }
    if (parsedMessage.type === "robot_info") {
      setRobotInfo(parsedMessage);
    }
    if (parsedMessage.type === "settings") {
      setSettings(parsedMessage.settings);
    }
    if (parsedMessage.type === "observed_buttons") {
      setButtons(
        parsedMessage.buttons.map((button: ButtonJson) =>
          ConvertButtonJsonToButton(
            now / 1000 - parsedMessage.timestamp_now,
            button,
          ),
        ),
      );
    }
    if (parsedMessage.type === "commands") {
      setCommands(
        parsedMessage.commands.map(({ button, command }) => ({
          button,
          command,
        })),
      );
    }
    if (parsedMessage.type === "wifi_rssi") {
      setWifiRssi(parsedMessage.wifi_rssi);
    }

    if (handle) {
      clearTimeout(handle);
    }
    handle = setTimeout(() => setOnline(false), 5000);
    setOnline(true);
  }, []);

  const { readyState } = useWebSocket(`ws://${hubHost}/ws`, {
    shouldReconnect: () => true,
    onMessage,
  });

  useEffect(() => {
    if (readyState !== ReadyState.OPEN) {
      setButtons(undefined);
      setCommands(undefined);
    }
  }, [readyState]);

  return {
    online,
    hubInfo,
    robotInfo,
    settings,
    buttons,
    commands,
    wifiRssi,
  };
}

export function useLocalStorageState<T>(
  key: string,
  defaultValue: T,
): [T, (value: T) => void] {
  const [value, setValue] = useState<T>(() => {
    const item = window.localStorage.getItem(key);
    return item !== null ? JSON.parse(item) : defaultValue;
  });
  const setValueAndLocalStorage = useCallback(
    (newValue: T) => {
      setValue(newValue);
      window.localStorage.setItem(key, JSON.stringify(newValue));
    },
    [key],
  );
  return [value, setValueAndLocalStorage];
}
