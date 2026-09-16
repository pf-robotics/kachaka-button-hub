import {
  Button,
  IsBraveridgeButton,
  IsBraveridgePlusButton,
  kBraveridgeDoublePressMajor,
  kBraveridgeLongPressMajor,
  kBraveridgePlusLongPressMajorBit,
} from "./types";

function getVariant(button: Button): { label: string; color: string } | null {
  if (IsBraveridgeButton(button) && "apple_i_beacon" in button) {
    if (button.apple_i_beacon.major === kBraveridgeDoublePressMajor) {
      return { label: "２回押し", color: "var(--sky-blue3)" };
    }
    if (button.apple_i_beacon.major === kBraveridgeLongPressMajor) {
      return { label: "長押し", color: "var(--modern-orange3)" };
    }
  }
  if (IsBraveridgePlusButton(button) && "apple_i_beacon" in button) {
    if (
      (button.apple_i_beacon.major & kBraveridgePlusLongPressMajorBit) !==
      0
    ) {
      return { label: "長押し", color: "var(--modern-orange3)" };
    }
  }
  return null;
}

export function ButtonVariantIndicator({
  button,
  style,
}: {
  button: Button;
  style?: React.CSSProperties;
}) {
  const variant = getVariant(button);
  if (variant === null) {
    return null;
  }
  return (
    <span className="chip" style={{ backgroundColor: variant.color, ...style }}>
      {variant.label}
    </span>
  );
}
