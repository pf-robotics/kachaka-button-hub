import { useEffect, useRef, useCallback, ReactNode } from "react";

import { MdOutlineClose } from "react-icons/md";

import { Icon } from "./Icon";

export function Modal({
  open,
  children,
  onClose,
}: {
  open: boolean;
  children: ReactNode;
  onClose?: () => void;
}) {
  const ref = useRef<HTMLDialogElement>(null);

  useEffect(() => {
    if (ref.current) {
      if (open) {
        ref.current.showModal();
      } else {
        ref.current.close();
      }
    }
  }, [open]);

  const handleClick = useCallback(
    (event: React.MouseEvent<HTMLDialogElement>) => {
      if (!ref.current || event.target !== ref.current) {
        return;
      }
      const rect = ref.current.getBoundingClientRect();
      const isInDialog =
        rect.top <= event.clientY &&
        event.clientY <= rect.top + rect.height &&
        rect.left <= event.clientX &&
        event.clientX <= rect.left + rect.width;
      if (!isInDialog) {
        onClose?.();
      }
    },
    [onClose],
  );

  return (
    <dialog
      ref={ref}
      style={{
        display: open ? "flex" : "none",
        overflow: "hidden",
        flexDirection: "column",
      }}
      onClick={handleClick}
    >
      {children}
      {onClose && (
        <button
          className="icon"
          onClick={onClose}
          style={{ position: "absolute", right: 8, top: 8 }}
        >
          <Icon children={<MdOutlineClose />} />
        </button>
      )}
    </dialog>
  );
}
