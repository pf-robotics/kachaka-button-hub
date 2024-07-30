import { ReactNode } from "react";

export function Box({
  children,
  style,
}: {
  children: ReactNode;
  style?: React.CSSProperties;
}) {
  return (
    <div
      style={{
        border: "2px solid var(--kachaka-gray-1)",
        backgroundColor: "white",
        padding: "12px",
        margin: "8px",
        borderRadius: "8px",
        ...style,
      }}
    >
      {children}
    </div>
  );
}
