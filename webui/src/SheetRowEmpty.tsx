import { useRef, useEffect } from "react";

export function SheetRowEmpty({
  groupSpan,
  groupLabel,
  itemLabel,
  recentlyPressed,
  onAdd,
}: {
  groupSpan: number;
  groupLabel?: JSX.Element;
  itemLabel?: string;
  recentlyPressed: boolean;
  onAdd: () => Promise<void>;
}) {
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
      <td>{itemLabel}</td>
      <td />
      <td />
      <td />
      <td />
      <td />
      <td />
      <td>
        <button type="button" onClick={onAdd}>
          追加
        </button>
      </td>
    </tr>
  );
}
