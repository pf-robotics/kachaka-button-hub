import { MdAdd } from "react-icons/md";
import { MdCheck } from "react-icons/md";

import { Box } from "./Box";
import { Button } from "./types";
import { ButtonImage } from "./ButtonImage";
import { ButtonNameEditor } from "./ButtonNameEditor";
import { Distance } from "./Distance";
import { DurationText } from "./DurationText";
import { Icon } from "./Icon";

export function ObservedButton({
  button,
  registerred,
  onRegister,
  onSetButtonName,
  onDeleteButtonName,
}: {
  button: Button;
  registerred: boolean;
  onRegister: (button: Button) => void;
  onSetButtonName: (button: Button, name: string) => Promise<void>;
  onDeleteButtonName: (button: Button) => Promise<void>;
}) {
  return (
    <Box
      style={{
        display: "flex",
        flexDirection: "column",
        gap: 16,
      }}
    >
      <ButtonNameEditor
        button={button}
        name={button.name}
        bold
        onSetButtonName={onSetButtonName}
        onDeleteButtonName={onDeleteButtonName}
      />
      <div
        style={{
          display: "flex",
          flexDirection: "row",
          gap: 16,
          alignItems: "center",
        }}
      >
        <ButtonImage button={button} style={{ opacity: 0.5 }} />
        <div
          style={{ flex: 1, display: "flex", flexDirection: "column", gap: 16 }}
        >
          <div style={{ display: "flex", alignItems: "center" }}>
            {button.timestamp ? (
              <span>
                <DurationText
                  durationMsec={Date.now() - button.timestamp.getTime()}
                />
                前
              </span>
            ) : null}
            <span style={{ flex: 1 }} />
            {button.estimated_distance !== undefined ? (
              <Distance distance={button.estimated_distance} />
            ) : null}
          </div>
          <span style={{ display: "flex" }}>
            {registerred ? (
              <>
                <Icon
                  margin="right"
                  children={<MdCheck style={{ color: "#4a4" }} />}
                />
                登録済
              </>
            ) : (
              <button onClick={() => onRegister(button)}>
                <Icon color="inherit" margin="right" children={<MdAdd />} />
                登録する
              </button>
            )}
          </span>
        </div>
      </div>
    </Box>
  );
}
