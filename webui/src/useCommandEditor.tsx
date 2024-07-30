import { useMemo } from "react";

import { RobotInfo, CommandType, Command } from "./types";
import {
  useCheckbox,
  useInput,
  useLocationSelect,
  useNumberInput,
  useShelfSelect,
  useShortcutSelect,
  useTextArea,
} from "./hooks";
import { isEqual } from "./utils";

export function useCommandEditor(
  command: Command,
  selectedCommandType: CommandType,
  robotInfo: RobotInfo | undefined,
) {
  const [moveShelfLocationId, moveShelfLocationSelect] = useLocationSelect(
    robotInfo?.locations ?? [],
    command.type === CommandType.MOVE_SHELF
      ? command.move_shelf.location_id
      : robotInfo?.locations?.[0]?.id ?? "",
    /* excludeCharger= */ true,
  );
  const [moveShelfShelfId, moveShelfShelfSelect] = useShelfSelect(
    robotInfo?.shelves ?? [],
    command.type === CommandType.MOVE_SHELF
      ? command.move_shelf.shelf_id
      : robotInfo?.shelves?.[0]?.id ?? "",
  );
  const [returnShelfShelfId, returnShelfShelfSelect] = useShelfSelect(
    robotInfo?.shelves ?? [],
    command.type === CommandType.RETURN_SHELF
      ? command.return_shelf.shelf_id
      : robotInfo?.shelves?.[0]?.id ?? "",
    [{ value: "", label: "持っている家具" }],
  );
  const [shortcutId, shortcutIdSelect] = useShortcutSelect(
    robotInfo?.shortcuts ?? [],
    command.type === CommandType.SHORTCUT
      ? command.shortcut.shortcut_id
      : robotInfo?.shortcuts?.[0]?.id ?? "",
  );
  const [moveToLocationLocationId, moveToLocationLocationSelect] =
    useLocationSelect(
      robotInfo?.locations ?? [],
      command.type === CommandType.MOVE_TO_LOCATION
        ? command.move_to_location.location_id
        : robotInfo?.locations?.[0]?.id ?? "",
      /* excludeCharger= */ true,
    );
  const speakInput = useInput(
    command.type === CommandType.SPEAK ? command.speak.text : "",
  );
  const httpGetUrlInput = useInput(
    command.type === CommandType.HTTP_GET ? command.http_get.url : "",
  );
  const httpPostUrlInput = useInput(
    command.type === CommandType.HTTP_POST ? command.http_post.url : "",
  );
  const httpPostBodyInput = useTextArea(
    command.type === CommandType.HTTP_POST ? command.http_post.body : "",
  );
  const cancelAllInput = useCheckbox(command.cancel_all);
  const ttsOnSuccessInput = useInput(command.tts_on_success ?? "");
  const deferrableInput = useCheckbox(command.deferrable ?? false);
  const lockDurationSecInput = useNumberInput(
    command.lock_duration_sec ?? 0,
    undefined,
    { min: 0 },
  );

  const newCommand = useMemo((): Command => {
    let out = { ...command }; // This is required to keep order of properties
    switch (selectedCommandType) {
      case CommandType.MOVE_SHELF:
        out = {
          ...out,
          type: CommandType.MOVE_SHELF,
          move_shelf: {
            shelf_id: moveShelfShelfId ?? "",
            location_id: moveShelfLocationId ?? "",
          },
        };
        break;
      case CommandType.RETURN_SHELF:
        out = {
          ...out,
          type: CommandType.RETURN_SHELF,
          return_shelf: {
            shelf_id: returnShelfShelfId ?? "",
          },
        };
        break;
      case CommandType.UNDOCK_SHELF:
        out = {
          ...out,
          type: CommandType.UNDOCK_SHELF,
        };
        break;
      case CommandType.MOVE_TO_LOCATION:
        out = {
          ...out,
          type: CommandType.MOVE_TO_LOCATION,
          move_to_location: {
            location_id: moveToLocationLocationId ?? "",
          },
        };
        break;
      case CommandType.RETURN_HOME:
        out = {
          ...out,
          type: CommandType.RETURN_HOME,
        };
        break;
      case CommandType.SHORTCUT:
        out = {
          ...out,
          type: CommandType.SHORTCUT,
          shortcut: {
            shortcut_id: shortcutId ?? "",
          },
        };
        break;
      case CommandType.SPEAK:
        out = {
          ...out,
          type: CommandType.SPEAK,
          speak: {
            text: speakInput.value,
          },
        };
        break;
      case CommandType.PROCEED:
        out = {
          ...out,
          type: CommandType.PROCEED,
        };
        break;
      case CommandType.CANCEL_COMMAND:
        out = {
          ...out,
          type: CommandType.CANCEL_COMMAND,
        };
        break;
      case CommandType.HTTP_GET:
        out = {
          ...out,
          type: CommandType.HTTP_GET,
          http_get: {
            url: httpGetUrlInput.value,
          },
        };
        break;
      case CommandType.HTTP_POST:
        out = {
          ...out,
          type: CommandType.HTTP_POST,
          http_post: {
            url: httpPostUrlInput.value,
            body: httpPostBodyInput.value,
          },
        };
        break;
      default:
        console.error("Unknown command type", selectedCommandType);
    }
    out.cancel_all = cancelAllInput.checked;
    if (ttsOnSuccessInput.value === "") {
      delete out.tts_on_success;
    } else {
      out.tts_on_success = ttsOnSuccessInput.value;
    }
    out.deferrable = deferrableInput.checked;
    if (lockDurationSecInput.value === 0) {
      delete out.lock_duration_sec;
    } else {
      out.lock_duration_sec = lockDurationSecInput.value;
    }
    return out;
  }, [
    command,
    selectedCommandType,
    moveShelfShelfId,
    moveShelfLocationId,
    returnShelfShelfId,
    moveToLocationLocationId,
    shortcutId,
    speakInput.value,
    httpGetUrlInput.value,
    httpPostUrlInput.value,
    httpPostBodyInput.value,
    cancelAllInput.checked,
    ttsOnSuccessInput.value,
    deferrableInput.checked,
    lockDurationSecInput.value,
  ]);

  const disableOptions = [
    CommandType.PROCEED,
    CommandType.CANCEL_COMMAND,
  ].includes(selectedCommandType);

  return {
    newCommand,
    moveShelf: (
      <>
        {moveShelfShelfSelect} を {moveShelfLocationSelect} に移動
      </>
    ),
    returnShelf: <>{returnShelfShelfSelect} を片付ける</>,
    undockShelf: <>持っている家具をその場に置く</>,
    moveToLocation: <>{moveToLocationLocationSelect} に移動</>,
    returnHome: <>充電ドックに戻る</>,
    shortcut: <>ショートカット {shortcutIdSelect} を実行</>,
    speak: (
      <>
        <input
          style={{ maxWidth: "50vw" }}
          {...speakInput}
          placeholder="フレーズ"
        />{" "}
        と発話
      </>
    ),
    cancelCommand: <>実行中のコマンドをキャンセル</>,
    proceed: <>待機状態を解除</>,
    httpGet: (
      <>
        <input
          style={{ maxWidth: "50vw" }}
          {...httpGetUrlInput}
          placeholder="URL"
        />{" "}
        に GET リクエストを送信
      </>
    ),
    httpPost: (
      <>
        <input
          style={{ maxWidth: "50vw" }}
          {...httpPostUrlInput}
          placeholder="URL"
        />{" "}
        に POST リクエストを送信
        <br />
        <textarea
          style={{ width: "calc(100% - 16px)" }}
          {...httpPostBodyInput}
          placeholder="Body"
        />
      </>
    ),
    cancelAllInput: <input {...cancelAllInput} disabled={disableOptions} />,
    ttsOnSuccessInput: (
      <input {...ttsOnSuccessInput} disabled={disableOptions} />
    ),
    deferrableInput: <input {...deferrableInput} disabled={disableOptions} />,
    lockDurationSecInput: (
      <input
        style={{ maxWidth: "3em" }}
        {...lockDurationSecInput}
        disabled={disableOptions}
      />
    ),
    modified: !isEqual(command, newCommand),
    disableOptions,
    disableShortcut: [undefined, 0].includes(robotInfo?.shortcuts?.length),
  };
}
