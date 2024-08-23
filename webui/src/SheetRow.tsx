import { useRef, useEffect, useState, useCallback } from "react";

import { Button, CommandType, Command, RobotInfo } from "./types";
import { useCommandEditor } from "./useCommandEditor";

export function SheetRow({
  groupSpan,
  groupLabel,
  itemLabel,
  button,
  command,
  robotInfo,
  recentlyPressed,
  enableShortcutFeature,
  onEdit,
  onDelete,
}: {
  groupSpan: number;
  groupLabel?: JSX.Element;
  itemLabel?: string;
  button: Button;
  command: Command;
  robotInfo: RobotInfo | undefined;
  recentlyPressed: boolean;
  enableShortcutFeature: boolean;
  onEdit: (button: Button, command: Command) => void;
  onDelete: (button: Button) => void;
}) {
  const [selectedCommandType, setSelectedCommandType] = useState<CommandType>(
    command.type,
  );
  const {
    newCommand,
    moveShelf,
    returnShelf,
    undockShelf,
    moveToLocation,
    returnHome,
    shortcut,
    speak,
    cancelCommand,
    proceed,
    httpGet,
    httpPost,
    cancelAllInput,
    ttsOnSuccessInput,
    deferrableInput,
    lockDurationSecInput,
    modified,
    disableOptions,
    disableShortcut,
  } = useCommandEditor(command, selectedCommandType, robotInfo);
  const handleEdit = useCallback(
    () => onEdit(button, newCommand),
    [button, newCommand, onEdit],
  );
  const handleDelete = useCallback(() => onDelete(button), [button, onDelete]);
  const bgStyle = {
    backgroundColor: modified ? "var(--coral-pink0)" : undefined,
  };
  const optProps: React.HTMLProps<HTMLTableCellElement> = {
    style: { visibility: disableOptions ? "hidden" : "visible", ...bgStyle },
  };
  const ShowIf = useCallback(
    ({ type, children }: { type: CommandType; children: React.ReactNode }) => (
      <div
        style={{
          height: selectedCommandType !== type ? 0 : "auto", // preserve width
          overflow: "hidden",
        }}
      >
        {children}
      </div>
    ),
    [selectedCommandType],
  );

  const HIGHLIGHT_COLOR = "#ff0";
  const HIGHLIGHT_DURATION = "5s";

  const ref = useRef<HTMLTableRowElement>(null);
  useEffect(() => {
    if (recentlyPressed) {
      const handle = setTimeout(() => {
        ref.current?.style.setProperty("background-color", "inherit");
        ref.current?.style.setProperty(
          "transition",
          `background-color ${HIGHLIGHT_DURATION}`,
        );
      }, 10);
      return () => clearTimeout(handle);
    }
  }, [recentlyPressed]);

  return (
    <tr
      ref={ref}
      style={{
        transition: recentlyPressed
          ? "none"
          : `background-color ${HIGHLIGHT_DURATION}`,
        backgroundColor: recentlyPressed ? HIGHLIGHT_COLOR : "inherit",
      }}
    >
      {groupSpan === 0 ? null : <td rowSpan={groupSpan}>{groupLabel}</td>}
      <td style={bgStyle}>{itemLabel}</td>
      <td style={bgStyle}>
        <select
          value={selectedCommandType}
          onChange={(e) =>
            setSelectedCommandType(Number(e.target.value) as CommandType)
          }
        >
          <option value={CommandType.MOVE_SHELF}>家具を移動</option>
          <option value={CommandType.RETURN_SHELF}>家具を片付ける</option>
          <option value={CommandType.UNDOCK_SHELF}>
            持っている家具をその場に置く
          </option>
          <option value={CommandType.MOVE_TO_LOCATION}>移動</option>
          <option value={CommandType.RETURN_HOME}>充電ドックに戻る</option>
          {enableShortcutFeature && (
            <option value={CommandType.SHORTCUT} disabled={disableShortcut}>
              ショートカットを実行
            </option>
          )}
          <option value={CommandType.SPEAK}>発話</option>
          <option value={CommandType.CANCEL_COMMAND}>コマンドキャンセル</option>
          <option value={CommandType.PROCEED}>待機状態を解除</option>
          <option value={CommandType.HTTP_GET}>HTTP GET</option>
          <option value={CommandType.HTTP_POST}>HTTP POST</option>
        </select>
      </td>
      <td style={{ textAlign: "start", ...bgStyle }}>
        <ShowIf type={CommandType.MOVE_SHELF}>{moveShelf}</ShowIf>
        <ShowIf type={CommandType.RETURN_SHELF}>{returnShelf}</ShowIf>
        <ShowIf type={CommandType.UNDOCK_SHELF}>{undockShelf}</ShowIf>
        <ShowIf type={CommandType.MOVE_TO_LOCATION}>{moveToLocation}</ShowIf>
        <ShowIf type={CommandType.RETURN_HOME}>{returnHome}</ShowIf>
        <ShowIf type={CommandType.SHORTCUT}>{shortcut}</ShowIf>
        <ShowIf type={CommandType.SPEAK}>{speak}</ShowIf>
        <ShowIf type={CommandType.CANCEL_COMMAND}>{cancelCommand}</ShowIf>
        <ShowIf type={CommandType.PROCEED}>{proceed}</ShowIf>
        <ShowIf type={CommandType.HTTP_GET}>{httpGet}</ShowIf>
        <ShowIf type={CommandType.HTTP_POST}>{httpPost}</ShowIf>
      </td>
      <td {...optProps}>{cancelAllInput}</td>
      <td {...optProps}>{deferrableInput}</td>
      <td {...optProps}>{ttsOnSuccessInput}</td>
      <td {...optProps}>{lockDurationSecInput} 秒</td>
      <td style={bgStyle}>
        <button
          type="button"
          disabled={!modified}
          onClick={handleEdit}
          style={{
            color: modified ? "white" : undefined,
            backgroundColor: modified ? "red" : undefined,
          }}
        >
          保存
        </button>{" "}
        <button type="button" onClick={handleDelete}>
          削除
        </button>
      </td>
    </tr>
  );
}
