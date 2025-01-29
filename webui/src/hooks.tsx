import { ChangeEvent, useCallback, useEffect, useMemo, useState } from "react";
import useWebSocket, { ReadyState } from "react-use-websocket";
import { getHubHttpApiEndpoint, getHubWebSocketEndpoint } from "./utils";

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
  Shortcut,
} from "./types";

type ManualSettings = {
  ip_address: string;
  subnet_mask: string;
  gateway: string;
  dns_server_1: string;
  dns_server_2: string;
};

export function useWiFiSettings(): [
  string | undefined,
  string | undefined,
  (ssid: string, pass: string, manual?: ManualSettings) => Promise<void>,
] {
  const [ssid, setSsid] = useState<string>();
  const [pass, setPass] = useState<string>();
  useEffect(() => {
    fetch(getHubHttpApiEndpoint("/config/wifi"))
      .then((res) => res.json())
      .then((data) => {
        setSsid(data.ssid);
        setPass(data.pass);
      });
  }, []);
  const setWiFiSettings = useCallback(
    (newSsid: string, newPass: string, manual?: ManualSettings) => {
      const request = { ssid: newSsid.trim(), pass: newPass.trim() };
      if (manual) {
        Object.assign(request, manual);
      }
      return fetch(getHubHttpApiEndpoint("/config/wifi"), {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(request),
      }).then(() => {
        setSsid(newSsid);
        setPass(newPass);
      });
    },
    [],
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

export function useTextArea(
  defaultValue: string,
  onChange?: (value: string) => void,
) {
  const [value, setValue] = useState(defaultValue);
  const onChangeImpl = (e: ChangeEvent<HTMLTextAreaElement>) => {
    setValue(e.target.value);
    onChange?.(e.target.value);
  };
  return { value, onChange: onChangeImpl };
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

export function useSelect(
  options: string[],
  defaultValue: string,
  placeholderLabel?: string,
): [string, JSX.Element] {
  const [selectedValue, setSelectedValue] = useState(defaultValue);
  const optionElems = useMemo(
    () =>
      options.map((option) => (
        <option key={option} value={option}>
          {option}
        </option>
      )),
    [options],
  );
  const selectProps = useMemo(
    () => ({
      value: selectedValue,
      onChange: (e: ChangeEvent<HTMLSelectElement>) =>
        setSelectedValue(e.target.value),
      children: options.includes(selectedValue)
        ? optionElems
        : [
            <option key="unknown" value={selectedValue} disabled>
              {placeholderLabel ?? "(選択してください)"}
            </option>,
            ...optionElems,
          ],
    }),
    [options, placeholderLabel, selectedValue, optionElems],
  );
  const select = useMemo(() => <select {...selectProps} />, [selectProps]);
  return [selectedValue, select];
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

export function useShortcutSelect(
  shortcuts: Shortcut[],
  defaultValue: string,
): [string | undefined, JSX.Element] {
  const [selectedShortcutId, setSelectedShortcutId] = useState<
    string | undefined
  >(defaultValue);
  const options = useMemo(
    () =>
      shortcuts.map((shortcut) => (
        <option key={shortcut.id} value={shortcut.id}>
          {shortcut.name}
        </option>
      )),
    [shortcuts],
  );
  const isKnownId = (shortcuts ?? [])
    .map((shortcut) => shortcut.id)
    .includes(defaultValue);
  const select = useMemo(
    () => (
      <select
        value={selectedShortcutId ?? ""}
        onChange={(e) => setSelectedShortcutId(e.target.value)}
        style={{ maxWidth: "30vw" }}
      >
        {!isKnownId && (
          <option value={defaultValue} disabled>
            (不明なショートカット)
          </option>
        )}
        {options}
      </select>
    ),
    [options, selectedShortcutId, defaultValue, isKnownId],
  );
  return [
    selectedShortcutId,
    shortcuts.length > 0 ? (
      select
    ) : (
      <select key="noshortcut" disabled>
        <option>(なし)</option>
      </select>
    ),
  ];
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

interface WifiAp {
  ssid: string;
  bssid: string;
  channel: number;
  encryption_type: number;
}

interface WifiApListMessage {
  type: "wifi_ap_list";
  scanning: boolean;
  wifi_ap_list: WifiAp[];
}

type WsMessage =
  | HubInfoMessage
  | RobotInfoMessage
  | SettingsMessage
  | ObservedButtonMessage
  | CommandsMessage
  | WifiRssiMessage
  | WifiApListMessage;

let handle: number | undefined = undefined;

export function useKachakaButtonHub() {
  const [networkState, setNetworkState] = useState<
    "offline" | "online" | "unstable"
  >("offline");
  const [hubInfo, setHubInfo] = useState<HubInfo>();
  const [robotInfo, setRobotInfo] = useState<RobotInfo>();
  const [settings, setSettings] = useState<Settings>();
  const [buttons, setButtons] = useState<Button[]>();
  const [commands, setCommands] =
    useState<{ button: Button; command: Command }[]>();
  const [wifiRssi, setWifiRssi] = useState<number>();
  const [wifiApList, setWifiApList] = useState<"scanning" | WifiAp[]>();

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
    if (parsedMessage.type === "wifi_ap_list") {
      setWifiApList(
        parsedMessage.scanning ? "scanning" : parsedMessage.wifi_ap_list,
      );
    }

    if (handle) {
      clearTimeout(handle);
    }
    handle = setTimeout(() => {
      setNetworkState("unstable");
      handle = setTimeout(() => setNetworkState("offline"), 10 * 1000);
    }, 5 * 1000);
    setNetworkState("online");
  }, []);

  const { readyState } = useWebSocket(getHubWebSocketEndpoint(), {
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
    networkState,
    hubInfo,
    robotInfo,
    settings,
    buttons,
    commands,
    wifiRssi,
    wifiApList,
  };
}

export function useFilteredCommandsAndButtons(
  commands: { button: Button; command: Command }[] | undefined,
  buttons: Button[] | undefined,
  gpioButtonIsEnabled: boolean,
): [{ button: Button; command: Command }[] | undefined, Button[] | undefined] {
  const filteredCommands = useMemo(
    () =>
      commands?.filter(
        ({ button }) => gpioButtonIsEnabled || !("gpio_button" in button),
      ),
    [commands, gpioButtonIsEnabled],
  );
  const filteredButtons = useMemo(
    () =>
      buttons?.filter(
        (button) => gpioButtonIsEnabled || !("gpio_button" in button),
      ),
    [buttons, gpioButtonIsEnabled],
  );
  return [filteredCommands, filteredButtons];
}
