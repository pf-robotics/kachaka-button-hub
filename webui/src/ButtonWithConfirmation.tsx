import { useState } from "react";

import { Modal } from "./Modal";

export function ButtonWithConfirmation({
  children,
  confirmText,
  confirmButtonText,
  style,
  disabled,
  onClick,
}: {
  children: React.ReactNode;
  confirmText: React.ReactNode;
  confirmButtonText?: string;
  style?: React.CSSProperties;
  disabled?: boolean;
  onClick: () => void;
}) {
  const [open, setOpen] = useState(false);

  return (
    <div>
      <button
        type="button"
        onClick={() => setOpen(true)}
        style={style}
        disabled={disabled}
      >
        {children}
      </button>
      <Modal open={open} onClose={() => setOpen(false)}>
        <h3>確認</h3>
        <p>{confirmText}</p>
        <button type="button" onClick={onClick}>
          {confirmButtonText}
        </button>
      </Modal>
    </div>
  );
}
