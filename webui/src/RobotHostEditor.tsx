import { useCallback } from "react";

import { useInput } from "./hooks";

export function RobotHostEditor({
  hubHost,
  robotHost,
}: {
  hubHost: string;
  robotHost: string | undefined;
}) {
  const setRobotHost = useCallback(
    (newRobotHost: string) =>
      fetch(`http://${hubHost}/config/robot_host`, {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ robot_host: newRobotHost.trim() }),
      }),
    [hubHost],
  );
  const robotHostInput = useInput(robotHost ?? "");

  const handleSetRobotHost = useCallback(() => {
    setRobotHost(robotHostInput.value);
    window.location.hash = "#reboot-apply";
  }, [robotHostInput, setRobotHost]);

  return (
    <>
      <div>
        現在の設定: <code>{robotHost || "(未設定)"}</code>
      </div>
      <div>
        例(シリアル番号): <code>BKP12345Z</code>
      </div>
      <div>
        例(IPアドレス): <code>123.45.67.8</code>
      </div>
      {/* The hidden input field is a hack to avoid auto-focus on the first input field */}
      <div style={{ maxWidth: 0, maxHeight: 0, overflow: "hidden" }}>
        <input type="checkbox" />
      </div>
      <input
        {...robotHostInput}
        placeholder="シリアル番号・IPアドレス"
      />
      <button
        onClick={handleSetRobotHost}
        disabled={
          robotHostInput.value === robotHost || robotHostInput.value === ""
        }
        style={{ margin: 4 }}
      >
        変更する
      </button>
    </>
  );
}
