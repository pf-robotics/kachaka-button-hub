import { MdQuestionMark } from "react-icons/md";
import { MdBluetoothSearching } from "react-icons/md";

import { Button, IsBraveridgeButton } from "./types";
import { ButtonVariantIndicator } from "./ButtonVariantIndicator";
import BraveridgeButton from "./BraveridgeButton.svg";
import M5Stack from "./M5Stack.svg";
import M5StackBtnA from "./M5StackBtnA.svg";
import M5StackBtnB from "./M5StackBtnB.svg";
import M5StackBtnC from "./M5StackBtnC.svg";

function _ButtonImage({
  button,
  style,
}: {
  button: Button;
  style?: React.CSSProperties;
}) {
  const sizedStyle = { width: 64, height: 64, ...style };
  if ("apple_i_beacon" in button) {
    if (IsBraveridgeButton(button)) {
      return <img src={BraveridgeButton} style={sizedStyle} />;
    } else {
      return <MdBluetoothSearching style={sizedStyle} />;
    }
  } else if ("m5_button" in button) {
    switch (button.m5_button.id) {
      case 1:
        return <img src={M5StackBtnA} style={sizedStyle} />;
      case 2:
        return <img src={M5StackBtnB} style={sizedStyle} />;
      case 3:
        return <img src={M5StackBtnC} style={sizedStyle} />;
    }
    return <img src={M5Stack} style={sizedStyle} />;
  }
  return <MdQuestionMark style={sizedStyle} />;
}

export function ButtonImage({
  button,
  style,
}: {
  button: Button;
  style?: React.CSSProperties;
}) {
  return (
    <span style={{ position: "relative" }}>
      <_ButtonImage button={button} style={style} />
      <ButtonVariantIndicator
        button={button}
        style={{ position: "absolute", bottom: 0, right: 0 }}
      />
    </span>
  );
}
