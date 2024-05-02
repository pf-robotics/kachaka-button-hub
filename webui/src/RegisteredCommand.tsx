import { useState, useEffect, useRef, useCallback, useMemo } from "react";

import { MdDelete } from "react-icons/md";
import { MdEdit } from "react-icons/md";

import { Button, Command, RobotInfo, GetButtonName } from "./types";
import { ButtonImage } from "./ButtonImage";
import { ButtonNameEditor } from "./ButtonNameEditor";
import { CommandEditor } from "./CommandEditor";
import { CommandText } from "./CommandText";
import { Icon } from "./Icon";
import { Modal } from "./Modal";

const HIGHLIGHT_COLOR = "#ff0";
const HIGHLIGHT_DURATION = "5s";

export function RegisteredCommand({
  name,
  button,
  command,
  robotInfo,
  recentlyPressed,
  onEdit,
  onDelete,
  onSetButtonName,
  useLockAndProceed,
}: {
  name: string | undefined;
  button: Button;
  command: Command;
  robotInfo: RobotInfo | undefined;
  recentlyPressed: boolean;
  onEdit: (button: Button, command: Command) => void;
  onDelete: (button: Button) => void;
  onSetButtonName: (button: Button, name: string) => Promise<void>;
  useLockAndProceed: boolean;
}) {
  const [openEditor, setOpenEditor] = useState(false);

  const ref = useRef<HTMLDivElement>(null);
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

  const handleEdit = useCallback(
    (newCommand: Command) => onEdit(button, newCommand),
    [button, onEdit],
  );
  const handleDelete = useCallback(() => onDelete(button), [button, onDelete]);
  const printName = useMemo(
    () => name ?? GetButtonName(button),
    [name, button],
  );

  return (
    <div style={{ display: "flex", flexDirection: "column", gap: 12 }}>
      <div
        ref={ref}
        style={{
          fontWeight: "bold",
          transition: recentlyPressed
            ? "none"
            : `background-color ${HIGHLIGHT_DURATION}`,
          backgroundColor: recentlyPressed ? HIGHLIGHT_COLOR : "inherit",
          display: "flex",
          alignItems: "center",
        }}
      >
        {printName}
      </div>
      <div
        style={{
          display: "flex",
          flexDirection: "row",
          gap: 16,
          alignItems: "center",
        }}
      >
        <ButtonImage button={button} />
        <div
          style={{ flex: 1, display: "flex", flexDirection: "column", gap: 16 }}
        >
          <CommandText command={command} robotInfo={robotInfo} />
          <div
            style={{
              display: "flex",
              flexDirection: "row",
              gap: 12,
            }}
          >
            <button onClick={() => setOpenEditor((prev) => !prev)}>
              <Icon color="inherit" margin="right" children={<MdEdit />} />
              編集
            </button>
            <span style={{ flex: 1 }} />
            <button className="icon" onClick={handleDelete}>
              <Icon children={<MdDelete />} />
            </button>
          </div>
        </div>
      </div>
      <Modal open={openEditor} onClose={() => setOpenEditor(false)}>
        <div style={{ margin: 8, marginRight: 32 }}>
          <ButtonNameEditor
            button={button}
            name={name}
            bold
            onSetButtonName={onSetButtonName}
          />
        </div>
        <CommandEditor
          command={command}
          robotInfo={robotInfo}
          onSubmit={handleEdit}
          useLockAndProceed={useLockAndProceed}
          closeDialog={() => setOpenEditor(false)}
        />
      </Modal>
    </div>
  );
}
