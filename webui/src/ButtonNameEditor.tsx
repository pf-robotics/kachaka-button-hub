import { useState, useCallback, useMemo } from "react";

import { MdEdit } from "react-icons/md";
import { MdDeleteOutline } from "react-icons/md";
import { debounce } from "ts-debounce";

import { Button, GetButtonName } from "./types";
import { Icon } from "./Icon";
import { Modal } from "./Modal";
import { useInput } from "./hooks";

export function ButtonNameEditor({
  button,
  name,
  bold,
  onSetButtonName,
  onDeleteButtonName,
}: {
  button: Button;
  name: string | undefined;
  bold?: boolean;
  onSetButtonName: (button: Button, name: string) => Promise<void>;
  onDeleteButtonName?: (button: Button) => Promise<void>;
}) {
  const printName = useMemo(
    () => name ?? GetButtonName(button),
    [name, button],
  );
  const [openDialog, setOpenDialog] = useState(false);
  const input = useInput(printName);

  const debouncedOnSetButtonName = useMemo(
    () => debounce(onSetButtonName, 200),
    [onSetButtonName],
  );
  const handleUpdateName = useCallback(
    () =>
      debouncedOnSetButtonName(button, input.value).then(() =>
        setOpenDialog(false),
      ),
    [button, input.value, debouncedOnSetButtonName],
  );
  const handleDelete = useCallback(
    () => onDeleteButtonName?.(button).then(() => setOpenDialog(false)),
    [button, onDeleteButtonName],
  );

  return (
    <>
      <div
        style={{
          fontWeight: bold ? "bold" : undefined,
          display: "flex",
          alignItems: "center",
        }}
      >
        {printName}
        <button
          className="icon"
          onClick={() => setOpenDialog((prev) => !prev)}
          style={{ marginLeft: 8 }}
        >
          <MdEdit style={{ color: "#aaa" }} />
        </button>
      </div>
      <Modal open={openDialog} onClose={() => setOpenDialog(false)}>
        <div style={{ display: "flex", flexDirection: "column", gap: 12 }}>
          <div>ボタン名の変更</div>
          <input {...input} />
          <div style={{ display: "flex", justifyContent: "end" }}>
            {button.name && onDeleteButtonName ? (
              <>
                <button className="icon" onClick={handleDelete}>
                  <Icon children={<MdDeleteOutline />} />
                </button>
                <span style={{ flex: 1 }} />
              </>
            ) : null}
            <button
              onClick={handleUpdateName}
              disabled={input.value === "" || input.value === name}
            >
              更新
            </button>
          </div>
        </div>
      </Modal>
    </>
  );
}
