import { useCallback } from "react";

import { RobotHostEditor } from "./RobotHostEditor";
import { RobotInfo, Settings } from "./types";

export function InitialUi({
  hubHost,
  robotInfo,
  settings,
}: {
  hubHost: string;
  robotInfo: RobotInfo | undefined;
  settings: Settings | undefined;
}) {
  const handleReboot = useCallback(() => {
    fetch(`http://${hubHost}/reboot`).then(
      () => (window.location.hash = "#reboot"),
    );
  }, [hubHost]);

  return (
    <div
      style={{
        width: "100%",
        maxWidth: 540,
        marginBottom: 32,
        marginTop: 64,
        alignSelf: "center",
      }}
    >
      <h2>カチャカAPIに接続中...</h2>
      <p>
        ▶
        初回設定時は、カチャカのシリアル番号かIPアドレスを設定してください（スマホアプリの「アプリ情報」から確認できます）。
      </p>
      <p>
        ▶
        カチャカの設定で、カチャカAPIを有効にしてください（スマホアプリから可能です）
      </p>
      <p>
        ▶ 接続が不安定な場合は、
        <a href="#" onClick={handleReboot}>
          Hubの再起動
        </a>
        を試してください。
      </p>
      <ul>
        <li>バージョン ... {robotInfo?.robot_version ?? "(取得中...)"}</li>
        <li>
          家具一覧 ...{" "}
          {robotInfo?.shelves?.length
            ? `${robotInfo?.shelves.length}件`
            : "(取得中...)"}
        </li>
        <li>
          目的地一覧 ...{" "}
          {robotInfo?.locations?.length
            ? `${robotInfo?.locations.length}件`
            : "(取得中...)"}
        </li>
        <li>
          ショートカット一覧 ...{" "}
          {robotInfo?.shortcuts?.length
            ? `${robotInfo?.shortcuts.length}件`
              : "(取得中...)"}
        </li>
      </ul>
      <h2>カチャカのシリアル番号・IPアドレス</h2>
      <RobotHostEditor
        hubHost={hubHost}
        robotHost={settings?.robot_host ?? ""}
      />
    </div>
  );
}
