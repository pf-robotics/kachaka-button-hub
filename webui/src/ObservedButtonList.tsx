import { useState, useEffect } from "react";

import { Button, GetButtonId } from "./types";
import { ObservedButton } from "./ObservedButton";

function isBeaconButton(button: Button) {
  return "apple_i_beacon" in button; 
}

function isM5Button(button: Button) {
  return "m5_button" in button;
}

function compareButtonSortKeys(lhs: Button, rhs: Button) {
  if ((isBeaconButton(lhs) && isBeaconButton(rhs)) || (isM5Button(lhs) && isM5Button(rhs))) {
    const lhsNameComplemented = lhs.name ?? "";
    const rhsNameComplemented = rhs.name ?? "";
    if (lhsNameComplemented === rhsNameComplemented) {
      return 0;
    }
    if (lhsNameComplemented === "") {
      return 1;
    }
    if (rhsNameComplemented === "") {
      return -1;
    }
    const colloator = new Intl.Collator("ja-JP", { numeric: true, sensitivity: "base"});
    return colloator.compare(lhsNameComplemented, rhsNameComplemented);
  }
  return isBeaconButton(lhs) ? -1 : 1;
}

export function ObservedButtonList({
  buttons,
  registeredButtonIds,
  onRegister,
  onSetButtonName,
  onDeleteButtonName,
}: {
  buttons: Button[] | undefined;
  registeredButtonIds: string[];
  onRegister: (button: Button) => void;
  onSetButtonName: (button: Button, name: string) => Promise<void>;
  onDeleteButtonName: (button: Button) => Promise<void>;
}) {
  const [_, setReloadCount] = useState(0);

  useEffect(() => {
    const handle = setInterval(() => setReloadCount((prev) => prev + 1), 1000);
    return () => clearInterval(handle);
  }, []);

  if (buttons === undefined) {
    return (
      <>
        <h2>見つかったボタン</h2>
        <p>(Loading...)</p>
      </>
    );
  }
  return (
    <>
      <h2>見つかったボタン</h2>
      <div>
        {buttons.filter((button) => !("m5_button" in button)).length === 0 ? (
          <p style={{ textAlign: "center" }}>
            （ボタンを押すと、ここに表示されます）
          </p>
        ) : null}
        {buttons.sort(compareButtonSortKeys).map((button) => {
          const id = GetButtonId(button);
          const registerred = registeredButtonIds.includes(id);
          if (registerred) {
            return null;
          }
          return (
            <ObservedButton
              key={id}
              button={button}
              registerred={registerred}
              onRegister={onRegister}
              onSetButtonName={onSetButtonName}
              onDeleteButtonName={onDeleteButtonName}
            />
          );
        })}
      </div>
    </>
  );
}
