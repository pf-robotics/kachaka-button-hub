import { useMemo } from "react";

import { ObservedButtonList } from "./ObservedButtonList";
import { RegisteredCommandList } from "./RegisteredCommandList";

import { Button, Command, RobotInfo, GetButtonId } from "./types";

export function MainCardUi({
  buttons,
  commands,
  buttonIdToNameMap,
  robotInfo,
  onEdit,
  onDelete,
  onSetButtonName,
  onDeleteButtonName,
  setEditingButton,
  showAllIBeacons,
  useLockAndProceed,
}: {
  buttons: Button[] | undefined;
  commands: { button: Button; command: Command }[] | undefined;
  buttonIdToNameMap: Map<string, string>;
  robotInfo: RobotInfo | undefined;
  onEdit: (button: Button, command: Command) => Promise<void>;
  onDelete: (button: Button) => Promise<void>;
  onSetButtonName: (button: Button, name: string) => Promise<void>;
  onDeleteButtonName: (button: Button) => Promise<void>;
  setEditingButton: (button: Button) => void;
  showAllIBeacons: boolean;
  useLockAndProceed: boolean;
}) {
  const registeredButtonIds = useMemo(
    () => (commands ?? []).map(({ button }) => GetButtonId(button)),
    [commands],
  );

  const recentPressedButtonId = useMemo(
    () =>
      buttons
        ?.filter(({ timestamp }) =>
          timestamp === undefined
            ? false
            : Date.now() - timestamp.getTime() < 1000,
        )
        .map((button) => GetButtonId(button)) ?? [],
    [buttons],
  );

  return (
    <div
      style={{
        width: "100%",
        maxWidth: 540,
        marginBottom: 32,
        alignSelf: "center",
      }}
    >
      <RegisteredCommandList
        commands={commands}
        robotInfo={robotInfo}
        buttonIdToNameMap={buttonIdToNameMap}
        recentPressedButtonId={recentPressedButtonId}
        onEdit={onEdit}
        onDelete={onDelete}
        onSetButtonName={onSetButtonName}
        useLockAndProceed={useLockAndProceed}
      />
      <div style={{ height: "1em" }} />
      <ObservedButtonList
        buttons={buttons}
        registeredButtonIds={registeredButtonIds}
        showAllIBeacons={showAllIBeacons}
        onRegister={(button) => setEditingButton(button)}
        onSetButtonName={onSetButtonName}
        onDeleteButtonName={onDeleteButtonName}
      />
    </div>
  );
}
