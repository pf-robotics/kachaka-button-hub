import { useCallback } from "react";

import { RobotHostEditor } from "./RobotHostEditor";
import { RobotInfo, Settings } from "./types";
import { getHubHttpApiEndpoint } from "./utils";

import { MdCheckCircle } from "react-icons/md";

export function CheckMark() {
  return (
    <MdCheckCircle
      style={{
        color: "var(--status-success)",
        fontSize: "1.2em",
        marginRight: 8,
      }}
    />
  );
}

export function InitialUi({
  robotInfo,
  settings,
}: {
  robotInfo: RobotInfo | undefined;
  settings: Settings | undefined;
}) {
  const handleReboot = useCallback(() => {
    fetch(getHubHttpApiEndpoint("/reboot")).then(() => {
      window.location.hash = "#reboot";
    });
  }, []);

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
        <button type="button" onClick={handleReboot}>
          Hubの再起動
        </button>
        を試してください。
      </p>
      <ul>
        <li>
          バージョン ...{" "}
          {robotInfo?.robot_version ? (
            <>
              <CheckMark />
              {robotInfo?.robot_version}
            </>
          ) : (
            <progress />
          )}
        </li>
        <li>
          家具一覧 ...{" "}
          {robotInfo?.shelves?.length ? (
            <>
              <CheckMark />
              {robotInfo?.shelves.length}件
            </>
          ) : (
            <progress />
          )}
        </li>
        <li>
          目的地一覧 ...{" "}
          {robotInfo?.locations?.length ? (
            <>
              <CheckMark />
              {robotInfo?.locations.length}件
            </>
          ) : (
            <progress />
          )}
        </li>
        <li>
          ショートカット一覧 ...{" "}
          {robotInfo?.shortcuts?.length ? (
            <>
              <CheckMark />
              {robotInfo?.shortcuts.length}件
            </>
          ) : (
            <progress />
          )}
        </li>
      </ul>
      <h2>カチャカのシリアル番号・IPアドレス</h2>
      <RobotHostEditor robotHost={settings?.robot_host ?? ""} />
    </div>
  );
}
