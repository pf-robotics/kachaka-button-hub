import { useState, useCallback } from "react";

import { RobotInfo, CommandType, Command } from "./types";
import { useCommandEditor } from "./useCommandEditor";

function Checkbox({
  checked,
  children,
  onSelect,
}: {
  checked: boolean;
  children: React.ReactNode;
  onSelect: () => void;
}) {
  return (
    <label style={{ fontWeight: checked ? "bold" : undefined }}>
      <input
        type="radio"
        checked={checked}
        onChange={(ev: React.ChangeEvent<HTMLInputElement>) => {
          if (ev.target.checked) {
            onSelect();
          }
        }}
        style={{ marginRight: 8 }}
      />
      {children}
    </label>
  );
}

function Item({
  commandType,
  command,
  onSelect,
  children,
}: {
  commandType: CommandType;
  command: Command;
  onSelect: () => void;
  children: React.ReactNode;
}) {
  const selected = command.type === commandType;
  return (
    <div>
      <Checkbox checked={selected} onSelect={onSelect}>
        {children}
      </Checkbox>
    </div>
  );
}

export function CommandEditor({
  command,
  robotInfo,
  onSubmit,
  useLockAndProceed,
  closeDialog,
}: {
  command: Command;
  robotInfo: RobotInfo | undefined;
  onSubmit: (command: Command) => void;
  useLockAndProceed: boolean;
  closeDialog: () => void;
}) {
  const [selectedCommandType, setSelectedCommandType] = useState(command.type);
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
  const handleSubmit = useCallback(() => {
    onSubmit(newCommand);
    closeDialog();
  }, [newCommand, onSubmit, closeDialog]);
  return (
    <>
      <div
        style={{
          overflow: "auto",
          display: "flex",
          flexDirection: "column",
          gap: 12,
        }}
      >
        <h3>コマンド</h3>
        <Item
          commandType={CommandType.MOVE_SHELF}
          command={newCommand}
          onSelect={() => setSelectedCommandType(CommandType.MOVE_SHELF)}
        >
          {moveShelf}
        </Item>
        <Item
          commandType={CommandType.RETURN_SHELF}
          command={newCommand}
          onSelect={() => setSelectedCommandType(CommandType.RETURN_SHELF)}
        >
          {returnShelf}
        </Item>
        <Item
          commandType={CommandType.MOVE_TO_LOCATION}
          command={newCommand}
          onSelect={() => setSelectedCommandType(CommandType.MOVE_TO_LOCATION)}
        >
          {moveToLocation}
        </Item>
        <Item
          commandType={CommandType.RETURN_HOME}
          command={newCommand}
          onSelect={() => setSelectedCommandType(CommandType.RETURN_HOME)}
        >
          {returnHome}
        </Item>
        <Item
          commandType={CommandType.SPEAK}
          command={newCommand}
          onSelect={() => setSelectedCommandType(CommandType.SPEAK)}
        >
          {speak}
        </Item>
        <Item
          commandType={CommandType.CANCEL_COMMAND}
          command={newCommand}
          onSelect={() => setSelectedCommandType(CommandType.CANCEL_COMMAND)}
        >
          {cancelCommand}
        </Item>
        {useLockAndProceed && (
          <Item
            commandType={CommandType.PROCEED}
            command={newCommand}
            onSelect={() => setSelectedCommandType(CommandType.PROCEED)}
          >
            {proceed}
          </Item>
        )}
        <h3 className={disableOptions ? "disabled" : undefined}>オプション</h3>
        <div className={disableOptions ? "disabled" : undefined}>
          <label>{cancelAllInput} 実行中のコマンドをキャンセルする</label>
        </div>
        <div className={disableOptions ? "disabled" : undefined}>
          <label>{deferrableInput} 後回しにしてよい</label>
        </div>
        <div className={disableOptions ? "disabled" : undefined}>
          <label>コマンド成功後に {ttsOnSuccessInput} と発話</label>
        </div>
        {useLockAndProceed && (
          <div className={disableOptions ? "disabled" : undefined}>
            <label>到着後に {lockDurationSecInput} 秒間待機する</label>
          </div>
        )}
      </div>
      <div style={{ margin: 8, display: "flex", justifyContent: "center" }}>
        <button className="primary" disabled={!modified} onClick={handleSubmit}>
          保存
        </button>
      </div>
    </>
  );
}
