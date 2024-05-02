import { Button } from "./types";

export function ButtonVariantIndicator({
  button,
  style,
}: {
  button: Button;
  style?: React.CSSProperties;
}) {
  if (
    "apple_i_beacon" in button &&
    button.apple_i_beacon.uuid === "00010203-0405-0607-0809-0a0b0c0d0e0f"
  ) {
    if (button.apple_i_beacon.major === 12289) {
      return (
        <span
          className="chip"
          style={{ backgroundColor: "var(--sky-blue3)", ...style }}
        >
          ２回押し
        </span>
      );
    }
    if (button.apple_i_beacon.major === 4097) {
      return (
        <span
          className="chip"
          style={{ backgroundColor: "var(--modern-orange3)", ...style }}
        >
          長押し
        </span>
      );
    }
  }
  return null;
}
