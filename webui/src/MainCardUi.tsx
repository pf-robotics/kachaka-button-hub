import { useMemo } from "react";

import { ObservedButtonList } from "./ObservedButtonList";
import { RegisteredCommandList } from "./RegisteredCommandList";

import {
  type Button,
  type Command,
  type RobotInfo,
  GetButtonId,
} from "./types";

export function MainCardUi({
  buttons,
  commands,
  buttonIdToNameMap,
  recentPressedButtonId,
  robotInfo,
  onEdit,
  onDelete,
  onSetButtonName,
  onDeleteButtonName,
  setEditingButton,
  useLockAndProceed,
  enableShortcutFeature,
}: {
  buttons: Button[] | undefined;
  commands: { button: Button; command: Command }[] | undefined;
  buttonIdToNameMap: Map<string, string>;
  recentPressedButtonId: string[];
  robotInfo: RobotInfo | undefined;
  onEdit: (button: Button, command: Command) => Promise<void>;
  onDelete: (button: Button) => Promise<void>;
  onSetButtonName: (button: Button, name: string) => Promise<void>;
  onDeleteButtonName: (button: Button) => Promise<void>;
  setEditingButton: (button: Button) => void;
  useLockAndProceed: boolean;
  enableShortcutFeature: boolean;
}) {
  const registeredButtonIds = useMemo(
    () => (commands ?? []).map(({ button }) => GetButtonId(button)),
    [commands],
  );

  return (
    <div style={{ width: "100%", maxWidth: 540, alignSelf: "center" }}>
      <RegisteredCommandList
        commands={commands}
        robotInfo={robotInfo}
        buttonIdToNameMap={buttonIdToNameMap}
        recentPressedButtonId={recentPressedButtonId}
        onEdit={onEdit}
        onDelete={onDelete}
        onSetButtonName={onSetButtonName}
        useLockAndProceed={useLockAndProceed}
        enableShortcutFeature={enableShortcutFeature}
      />
      <div style={{ height: "1em" }} />
      <ObservedButtonList
        buttons={buttons}
        registeredButtonIds={registeredButtonIds}
        onRegister={(button) => setEditingButton(button)}
        onSetButtonName={onSetButtonName}
        onDeleteButtonName={onDeleteButtonName}
      />
    </div>
  );
}
