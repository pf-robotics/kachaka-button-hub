export interface Shelf {
  id: string;
  name: string;
}

export interface Location {
  id: string;
  name: string;
  type: string;
}

export interface Shortcut {
  id: string;
  name: string;
}

export interface HubInfo {
  hub_version: string;
  ota_available: boolean;
  ota_label: string;
  client_count: number;
}

export interface RobotInfo {
  robot_version?: string;
  shelves?: Shelf[];
  locations?: Location[];
  shortcuts?: Shortcut[];
}

export interface Settings {
  wifi_ssid: string;
  robot_host: string;
  ntp_server: string;
  beep_volume: number;
  screen_brightness: number;
  auto_ota_is_enabled: boolean;
  auto_refetch_on_ui_load: boolean;
  gpio_button_is_enabled: boolean;
}

export interface AppleIBeacon {
  address: string;
  uuid: string;
  major: number;
  minor: number;
}

export interface M5Button {
  id: number;
}

export interface GpioButton {
  id: number;
}

export interface TimestampByDate {
  timestamp?: Date;
}

export interface TimestampBySeconds {
  timestamp?: number;
}

export type ButtonBase = (
  | { apple_i_beacon: AppleIBeacon }
  | { m5_button: M5Button }
  | { gpio_button: GpioButton }
) & {
  name?: string;
  estimated_distance?: number;
};

export type ButtonJson = ButtonBase & TimestampBySeconds;

export type Button = ButtonBase & TimestampByDate;

export function ConvertButtonJsonToButton(
  offsetSeconds: number,
  button: ButtonJson,
): Button {
  if (button.timestamp !== undefined) {
    return {
      ...button,
      timestamp: new Date((button.timestamp + offsetSeconds) * 1000),
    };
  }
  return { ...button } as Button;
}

export function IsBraveridgeButton(button: Button | ButtonJson) {
  return (
    "apple_i_beacon" in button &&
    button.apple_i_beacon.uuid === "00010203-0405-0607-0809-0a0b0c0d0e0f"
  );
}

export function GetButtonId(button: Button | ButtonJson) {
  if ("m5_button" in button) {
    return `m5_button:${button.m5_button.id}`;
  }
  if ("apple_i_beacon" in button) {
    if (IsBraveridgeButton(button)) {
      if (button.apple_i_beacon.major === 1) {
        return `braveridge:${button.apple_i_beacon.address}:1`;
      }
      if (button.apple_i_beacon.major === 12289) {
        return `braveridge:${button.apple_i_beacon.address}:2`;
      }
      if (button.apple_i_beacon.major === 4097) {
        return `braveridge:${button.apple_i_beacon.address}:L`;
      }
    }
    return `apple_i_beacon:${button.apple_i_beacon.address}_${button.apple_i_beacon.uuid}_${button.apple_i_beacon.major}_${button.apple_i_beacon.minor}`;
  }
  if ("gpio_button" in button) {
    return `gpio_button:${button.gpio_button.id}`;
  }
  throw new Error("Unknown button type");
}

export function GetButtonName(button: Button | ButtonJson) {
  if ("m5_button" in button) {
    return `Hubボタン${button.m5_button.id}`;
  }
  if ("apple_i_beacon" in button) {
    if (IsBraveridgeButton(button)) {
      return `ボタン ${button.apple_i_beacon.address}`;
    }
    return `ビーコン ${button.apple_i_beacon.address}`;
  }
  return "ボタン";
}

export enum CommandType {
  MOVE_SHELF = 1,
  RETURN_SHELF = 2,
  FOLLOW_PERSON = 3,
  MAKE_NEW_MAP = 4,
  UNDOCK_SHELF = 5,
  MOVE_TO_LOCATION = 7,
  RETURN_HOME = 8,
  DOCK_SHELF = 9,
  EXEC_TASK = 10,
  SET_UP_WIFI = 11,
  SPEAK = 12,
  MOVE_TO_POSE = 13,
  REGISTER_SHELF = 14,
  LOCK = 15,
  MOVE_FORWARD = 16,
  ROTATE_IN_PLACE = 17,
  PROCEED = 1000,
  CANCEL_COMMAND = 1001,
  SHORTCUT = 1002,
  HTTP_GET = 2000,
  HTTP_POST = 2001,
}

export interface CommandBase {
  cancel_all: boolean;
  tts_on_success?: string;
  deferrable?: boolean;
  lock_duration_sec?: number;
}

export type MoveShelfCommand = {
  type: CommandType.MOVE_SHELF;
  move_shelf: {
    shelf_id: string;
    location_id: string;
  };
} & CommandBase;

export type ReturnShelfCommand = {
  type: CommandType.RETURN_SHELF;
  return_shelf: {
    shelf_id: string;
  };
} & CommandBase;

export type UndockShelfCommand = {
  type: CommandType.UNDOCK_SHELF;
} & CommandBase;

export type MoveToLocationCommand = {
  type: CommandType.MOVE_TO_LOCATION;
  move_to_location: {
    location_id: string;
  };
} & CommandBase;

export type ReturnHomeCommand = {
  type: CommandType.RETURN_HOME;
} & CommandBase;

export type ShortcutCommand = {
  type: CommandType.SHORTCUT;
  shortcut: {
    shortcut_id: string;
  };
} & CommandBase;

export type SpeakCommand = {
  type: CommandType.SPEAK;
  speak: {
    text: string;
  };
} & CommandBase;

export type ProceedCommand = {
  type: CommandType.PROCEED;
} & CommandBase;

export type CancelCommandCommand = {
  type: CommandType.CANCEL_COMMAND;
} & CommandBase;

export type HttpGetCommand = {
  type: CommandType.HTTP_GET;
  http_get: {
    url: string;
  };
} & CommandBase;

export type HttpPostCommand = {
  type: CommandType.HTTP_POST;
  http_post: {
    url: string;
    body: string;
  };
} & CommandBase;

export type Command =
  | MoveShelfCommand
  | ReturnShelfCommand
  | UndockShelfCommand
  | MoveToLocationCommand
  | ReturnHomeCommand
  | ShortcutCommand
  | SpeakCommand
  | ProceedCommand
  | CancelCommandCommand
  | HttpGetCommand
  | HttpPostCommand;
