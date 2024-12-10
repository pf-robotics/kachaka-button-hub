export function Collapse({
  open,
  children,
}: {
  open: boolean;
  children: React.ReactNode;
}) {
  return (
    <div
      style={{
        display: "grid",
        gridTemplateRows: open ? "1fr" : "0fr",
        transition: "grid-template-rows 0.2s",
      }}
    >
      <div style={{ overflowY: "hidden" }}>{children}</div>
    </div>
  );
}
