import { ReactNode } from "react";

export function Backdrop({
  children,
  alpha,
}: {
  children: ReactNode;
  alpha?: number;
}) {
  return (
    <div
      style={{
        position: "fixed",
        top: 0,
        left: 0,
        width: "100vw",
        height: "100dvh",
        color: "white",
        background: `rgba(0, 0, 0, ${alpha ?? 0.7})`,
        border: "none",
        display: "flex",
        justifyContent: "center",
        alignItems: "center",
        flexDirection: "column",
        zIndex: 10,
      }}
    >
      {children}
    </div>
  );
}
