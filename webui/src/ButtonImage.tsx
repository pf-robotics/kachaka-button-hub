import { MdQuestionMark } from "react-icons/md";
import { MdBluetoothSearching } from "react-icons/md";
import { BsSquare } from "react-icons/bs";
import { Bs1Square } from "react-icons/bs";
import { Bs2Square } from "react-icons/bs";
import { Bs3Square } from "react-icons/bs";
import { Bs4Square } from "react-icons/bs";
import { Bs5Square } from "react-icons/bs";
import { Bs6Square } from "react-icons/bs";
import { Bs7Square } from "react-icons/bs";
import { Bs8Square } from "react-icons/bs";
import { Bs9Square } from "react-icons/bs";

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
  } else if ("gpio_button" in button) {
    switch (button.gpio_button.id) {
      case 1:
        return <Bs1Square style={sizedStyle} />;
      case 2:
        return <Bs2Square style={sizedStyle} />;
      case 3:
        return <Bs3Square style={sizedStyle} />;
      case 4:
        return <Bs4Square style={sizedStyle} />;
      case 5:
        return <Bs5Square style={sizedStyle} />;
      case 6:
        return <Bs6Square style={sizedStyle} />;
      case 7:
        return <Bs7Square style={sizedStyle} />;
      case 8:
        return <Bs8Square style={sizedStyle} />;
      case 9:
        return <Bs9Square style={sizedStyle} />;
    }
    return <BsSquare style={sizedStyle} />;
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
