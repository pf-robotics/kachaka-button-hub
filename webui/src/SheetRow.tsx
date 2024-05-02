import { useState, useCallback } from "react";

import { Button, CommandType, Command, RobotInfo } from "./types";
import { useCommandEditor } from "./useCommandEditor";

export function SheetRow({
  groupSpan,
  groupLabel,
  itemLabel,
  button,
  command,
  robotInfo,
  onEdit,
  onDelete,
}: {
  groupSpan: number;
  groupLabel?: JSX.Element;
  itemLabel?: string;
  button: Button;
  command: Command;
  robotInfo: RobotInfo | undefined;
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
    moveToLocation,
    returnHome,
    speak,
    cancelCommand,
    proceed,
    cancelAllInput,
    ttsOnSuccessInput,
    deferrableInput,
    lockDurationSecInput,
    modified,
    disableOptions,
  } = useCommandEditor(command, selectedCommandType, robotInfo);
  const handleEdit = useCallback(
    () => onEdit(button, newCommand),
    [button, newCommand, onEdit],
  );
  const handleDelete = useCallback(() => onDelete(button), [button, onDelete]);
  const optProps: React.HTMLProps<HTMLTableCellElement> = {
    style: { visibility: disableOptions ? "hidden" : "visible" },
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

  return (
    <tr>
      {groupSpan === 0 ? null : <td rowSpan={groupSpan}>{groupLabel}</td>}
      <td>{itemLabel}</td>
      <td>
        <select
          value={selectedCommandType}
          onChange={(e) =>
            setSelectedCommandType(Number(e.target.value) as CommandType)
          }
        >
          <option value={CommandType.MOVE_SHELF}>家具を移動</option>
          <option value={CommandType.RETURN_SHELF}>家具を片付ける</option>
          <option value={CommandType.MOVE_TO_LOCATION}>移動</option>
          <option value={CommandType.RETURN_HOME}>充電ドックに戻る</option>
          <option value={CommandType.SPEAK}>発話</option>
          <option value={CommandType.CANCEL_COMMAND}>コマンドキャンセル</option>
          <option value={CommandType.PROCEED}>待機状態を解除</option>
        </select>
      </td>
      <td style={{ textAlign: "start" }}>
        <ShowIf type={CommandType.MOVE_SHELF}>{moveShelf}</ShowIf>
        <ShowIf type={CommandType.RETURN_SHELF}>{returnShelf}</ShowIf>
        <ShowIf type={CommandType.MOVE_TO_LOCATION}>{moveToLocation}</ShowIf>
        <ShowIf type={CommandType.RETURN_HOME}>{returnHome}</ShowIf>
        <ShowIf type={CommandType.SPEAK}>{speak}</ShowIf>
        <ShowIf type={CommandType.CANCEL_COMMAND}>{cancelCommand}</ShowIf>
        <ShowIf type={CommandType.PROCEED}>{proceed}</ShowIf>
      </td>
      <td {...optProps}>{cancelAllInput}</td>
      <td {...optProps}>{deferrableInput}</td>
      <td {...optProps}>{ttsOnSuccessInput}</td>
      <td {...optProps}>{lockDurationSecInput} 秒</td>
      <td>
        <button
          disabled={!modified}
          onClick={handleEdit}
          style={{
            color: modified ? "white" : undefined,
            backgroundColor: modified ? "red" : undefined,
          }}
        >
          保存
        </button>{" "}
        <button onClick={handleDelete}>削除</button>
      </td>
    </tr>
  );
}
