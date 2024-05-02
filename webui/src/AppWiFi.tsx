import { useState, useCallback } from "react";

import { useWiFiSettings, useInput } from "./hooks";
import { getHubHost } from "./utils";

const HUB_HOST = getHubHost();

export function AppWiFi() {
  const [ssid, pass, setWiFiSettings] = useWiFiSettings(HUB_HOST);
  const [mode, setMode] = useState<"input" | "writing" | "done" | "error">(
    "input",
  );

  const ssidInput = useInput(ssid ?? "");
  const passInput = useInput(pass ?? "");

  const handleSubmit = useCallback(() => {
    setMode("writing");
    setWiFiSettings(ssidInput.value, passInput.value)
      .then(() => setMode("done"))
      .catch(() => setMode("error"));
  }, [ssidInput, passInput, setWiFiSettings]);

  return (
    <>
      <h1>カチャカボタンHub</h1>
      <h2>Wi-Fi 設定</h2>
      {mode === "input" ? (
        <>
          <p>
            Hubが接続するWi-Fiの情報を入力してください。カチャカが接続しているものと同じWi-Fiである必要があります。
          </p>
          <p>
            ※
            Hubは2.4GHzのWi-Fiのみ対応しています。カチャカは2.4GHzと5GHzの両方に対応しています。
          </p>
          <div style={{ marginBottom: 8 }}>
            <input {...ssidInput} placeholder="SSID" />
          </div>
          <div style={{ marginBottom: 8 }}>
            <input {...passInput} placeholder="Password" />
          </div>
          <div>
            <button
              onClick={handleSubmit}
              disabled={ssidInput.value === "" || passInput.value === ""}
            >
              書き込む
            </button>
          </div>
        </>
      ) : mode === "writing" ? (
        <>
          <p>書き込み中...</p>
          <div>
            <input {...ssidInput} disabled />
          </div>
          <div>
            <input {...passInput} disabled />
          </div>
        </>
      ) : mode === "done" ? (
        <>
          <p>設定が完了しました。</p>
          <p>
            スマートフォンのWi-Fiの設定を元に戻し、Hubの画面に表示されているQRコードを読み取ってください。
          </p>
        </>
      ) : (
        <>
          <p>エラー：もう一度お試しください。</p>
          <a href="/#wifi">再試行</a>
        </>
      )}
    </>
  );
}
