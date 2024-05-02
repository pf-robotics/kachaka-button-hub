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
        border: "1px solid #f0f0f0",
        backgroundColor: "white",
        padding: "12px",
        margin: "8px",
        borderRadius: "8px",
        boxShadow: "0 0 4px #f0f0f0",
        ...style,
      }}
    >
      {children}
    </div>
  );
}
