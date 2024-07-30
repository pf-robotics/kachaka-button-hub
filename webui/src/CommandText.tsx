import { useCallback } from "react";

import { Command, CommandType, RobotInfo } from "./types";

export function CommandText({
  command,
  robotInfo,
}: {
  command: Command;
  robotInfo: RobotInfo | undefined;
}) {
  const lookupShelfName = useCallback(
    (shelfId: string) => {
      if (shelfId === "") {
        return "持っている家具";
      }
      const shelf = robotInfo?.shelves?.find((s) => s.id === shelfId);
      return shelf?.name ?? "(不明な家具)";
    },
    [robotInfo],
  );
  const lookupLocationName = useCallback(
    (locationId: string) => {
      const location = robotInfo?.locations?.find((l) => l.id === locationId);
      return location?.name ?? "(不明な目的地)";
    },
    [robotInfo],
  );
  const lookupShortcutName = useCallback(
    (shortcutId: string) => {
      const shortcut = robotInfo?.shortcuts?.find((l) => l.id === shortcutId);
      return shortcut?.name ?? "(不明なショートカット)";
    },
    [robotInfo],
  );

  if (command.type === CommandType.MOVE_SHELF) {
    return (
      `${lookupShelfName(command.move_shelf.shelf_id)}を${lookupLocationName(
        command.move_shelf.location_id,
      )}に移動` +
      (command.tts_on_success ? `し、「${command.tts_on_success}」と発話` : "")
    );
  }
  if (command.type === CommandType.RETURN_SHELF) {
    return (
      `${lookupShelfName(command.return_shelf.shelf_id)}を片付け` +
      (command.tts_on_success ? `、「${command.tts_on_success}」と発話` : "る")
    );
  }
  if (command.type === CommandType.UNDOCK_SHELF) {
    return (
      `持っている家具をその場に置` +
      (command.tts_on_success ? `き、「${command.tts_on_success}」と発話` : "く")
    );
  }
  if (command.type === CommandType.MOVE_TO_LOCATION) {
    return (
      `${lookupLocationName(command.move_to_location.location_id)}に移動` +
      (command.tts_on_success ? `し、「${command.tts_on_success}」と発話` : "")
    );
  }
  if (command.type === CommandType.RETURN_HOME) {
    return (
      `充電ドックに戻` +
      (command.tts_on_success
        ? `り、「${command.tts_on_success}」と発話`
        : "る")
    );
  }
  if (command.type === CommandType.SHORTCUT) {
    return (
      `ショートカット ${lookupShortcutName(command.shortcut.shortcut_id)} を実行`
    );
  }
  if (command.type === CommandType.SPEAK) {
    return (
      `「${command.speak.text}」と発話` +
      (command.tts_on_success ? `し、「${command.tts_on_success}」と発話` : "")
    );
  }
  if (command.type === CommandType.PROCEED) {
    return `待機状態を解除`;
  }
  if (command.type === CommandType.CANCEL_COMMAND) {
    return `実行中のコマンドをキャンセル`;
  }
  if (command.type === CommandType.HTTP_GET) {
    return `GETリクエストを送信`;
  }
  if (command.type === CommandType.HTTP_POST) {
    return `POSTリクエストを送信`;
  }
}
