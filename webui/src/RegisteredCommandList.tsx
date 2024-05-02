import { Button, Command, RobotInfo, GetButtonId } from "./types";
import { Box } from "./Box";
import { RegisteredCommand } from "./RegisteredCommand";

export function RegisteredCommandList({
  commands,
  robotInfo,
  buttonIdToNameMap,
  recentPressedButtonId,
  onEdit,
  onDelete,
  onSetButtonName,
  useLockAndProceed,
}: {
  commands: { button: Button; command: Command }[] | undefined;
  robotInfo: RobotInfo | undefined;
  buttonIdToNameMap: Map<string, string>;
  recentPressedButtonId: string[];
  onEdit: (button: Button, command: Command) => void;
  onDelete: (button: Button) => void;
  onSetButtonName: (button: Button, name: string) => Promise<void>;
  useLockAndProceed: boolean;
}) {
  if (commands === undefined) {
    return (
      <>
        <h2>登録済みボタン</h2>
        <p>(Loading...)</p>
      </>
    );
  }
  return (
    <>
      <h2>登録済みボタン</h2>
      <div>
        {commands.length === 0 && (
          <p style={{ textAlign: "center" }}>（下のリストから登録できます）</p>
        )}
        {commands.map(({ button, command }) => (
          <Box key={GetButtonId(button)}>
            <RegisteredCommand
              name={buttonIdToNameMap.get(GetButtonId(button))}
              button={button}
              command={command}
              robotInfo={robotInfo}
              recentlyPressed={recentPressedButtonId.includes(
                GetButtonId(button),
              )}
              onEdit={onEdit}
              onDelete={onDelete}
              onSetButtonName={onSetButtonName}
              useLockAndProceed={useLockAndProceed}
            />
          </Box>
        ))}
      </div>
    </>
  );
}
