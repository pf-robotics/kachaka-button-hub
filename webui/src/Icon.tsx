import { ReactNode, CSSProperties } from "react";

export function Icon({
  children,
  color,
  margin,
  style,
}: {
  children: ReactNode;
  color?: string;
  margin?: "right" | "left";
  style?: CSSProperties;
}) {
  return (
    <span
      style={{
        color: color ?? "#777",
        lineHeight: 0,
        fontSize: "1.5em",
        marginRight: margin === "right" ? 4 : undefined,
        marginLeft: margin === "left" ? 4 : undefined,
        ...style,
      }}
    >
      {children}
    </span>
  );
}
