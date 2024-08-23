export function StackedWarning({
  items,
}: { items: Array<{ key: string; elem: JSX.Element }> }) {
  return (
    <div
      style={{
        position: "fixed",
        bottom: 62 + 16,
        right: 8,
        fontSize: "small",
        maxWidth: "70vw",
        display: "flex",
        flexDirection: "column",
        gap: 8,
      }}
    >
      {items.map(({ key, elem }) => (
        <div
          key={key}
          style={{
            backgroundColor: "var(--ginger-yellow1)",
            color: "var(--status-danger)",
            padding: 16,
            boxShadow: "0 0 8px 0 rgba(0, 0, 0, 0.5)",
            borderRadius: 8,
          }}
        >
          {elem}
        </div>
      ))}
    </div>
  );
}
