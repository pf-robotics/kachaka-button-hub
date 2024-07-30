import { useState, useCallback } from "react";

import { Backdrop } from "./Backdrop";
import {
  useKachakaButtonHub,
  useWiFiSettings,
  useInput,
  useSelect,
} from "./hooks";
import { getHubHost } from "./utils";
import { TopHeader } from "./TopHeader";
import { Icon } from "./Icon";

import { MdError } from "react-icons/md";
import { MdErrorOutline } from "react-icons/md";
import { MdReplay } from "react-icons/md";

const HUB_HOST = getHubHost();

function checkConfusableCharacters(s: string, level: "high" | "low"): string[] {
  const CONFUSABLES: {
    [l: string]: {
      [c: string]: {
        label: string;
        confusable: string[];
      };
    };
  } = {
    high: {
      "0": { label: "数字のゼロ「0」", confusable: ["O", "o"] },
      O: { label: "大文字のオー「O」", confusable: ["0", "o"] },
      o: { label: "小文字のオー「o」", confusable: ["O", "0"] },
      "1": { label: "数字のイチ「1」", confusable: ["l", "I"] },
      l: { label: "小文字のエル「l」", confusable: ["1", "I"] },
      I: { label: "大文字のアイ「I」", confusable: ["1", "l"] },
      "-": { label: "ハイフン「-」", confusable: ["_"] },
      _: { label: "アンダースコア「_」", confusable: ["-"] },
    },
    low: {
      S: { label: "大文字のエス「S」", confusable: ["s", "5"] },
      s: { label: "小文字のエス「s」", confusable: ["S", "5"] },
      "5": { label: "数字の五「5」", confusable: ["S", "s"] },
      Z: { label: "大文字のゼット「Z」", confusable: ["z", "2"] },
      z: { label: "小文字のゼット「z」", confusable: ["Z", "2"] },
      "2": { label: "数字のニ「2」", confusable: ["Z", "z"] },
      B: { label: "大文字のビー「B」", confusable: ["8"] },
      "8": { label: "数字の八「8」", confusable: ["B"] },
      ".": { label: "ピリオド「.」", confusable: [","] },
      ",": { label: "カンマ「,」", confusable: ["."] },
      ";": { label: "セミコロン「;」", confusable: [":"] },
      ":": { label: "コロン「:」", confusable: [";"] },
    },
  };
  const out: string[] = [];
  for (const [c, { label, confusable }] of Object.entries(CONFUSABLES[level])) {
    if (s.includes(c)) {
      const candidates = confusable
        .map((c) => `${CONFUSABLES[level][c].label}`)
        .join("や");
      out.push(`${label}が、${candidates}の間違いではないか注意してください。`);
    }
  }
  return out;
}

function checkJapaneseZenkaku(s: string): string[] {
  const out: string[] = [];
  // Check if ASCII code
  // eslint-disable-next-line no-control-regex
  if (s.match(/[^\x01-\x7E]/)) {
    out.push(
      `全角文字の「${
        s
          .match(/[^\x01-\x7E]/g) // eslint-disable-line no-control-regex
          ?.filter((c, i, self) => self.indexOf(c) === i)
          .join(", ") ?? ""
      }」が含まれています。半角文字の間違いではないか確認してください。`,
    );
  }
  return out;
}

export function AppWiFi() {
  const [_ssid, pass, setWiFiSettings] = useWiFiSettings(HUB_HOST);
  const { wifiApList } = useKachakaButtonHub(HUB_HOST);
  const [mode, setMode] = useState<
    "input" | "scanning" | "writing" | "done" | "error"
  >("input");

  const [selectedSsid, ssidSelectProps] = useSelect(
    (Array.isArray(wifiApList) ? wifiApList : [])
      ?.map(({ ssid }) => ssid)
      .filter((ssid, i, self) => self.indexOf(ssid) === i) ?? [],
    "",
    "(Wi-Fiを選択してください)",
  );
  const passInput = useInput(pass ?? "");
  const [showPass, setShowPass] = useState(false);

  const handleSubmit = useCallback(() => {
    setMode("writing");
    setWiFiSettings(selectedSsid, passInput.value)
      .then(() => setMode("done"))
      .catch(() => setMode("error"));
  }, [selectedSsid, passInput, setWiFiSettings]);

  const handleRescan = useCallback(() => {
    setMode("scanning");
    fetch(`http://${HUB_HOST}/wifi_scan`)
      .then(() => setMode("input"))
      .catch(() => setMode("error"));
  }, []);

  return (
    <>
      <TopHeader>カチャカボタンHub</TopHeader>
      <div className="content">
        <h2>Wi-Fi 設定</h2>
        {["input", "scanning"].includes(mode) ? (
          <>
            <p>
              Hubが接続するWi-Fiの情報を入力してください。カチャカ本体が接続しているものと同じWi-Fiである必要があります。
            </p>
            <p>
              ※
              Hubは2.4GHzのWi-Fiのみ対応しています。カチャカ本体は2.4GHzと5GHzの両方に対応しています。
            </p>
            <div
              style={{
                display: "flex",
                alignItems: "center",
                gap: 8,
                marginBottom: 8,
              }}
            >
              {Array.isArray(wifiApList) && wifiApList.length > 0 ? (
                <select {...ssidSelectProps} />
              ) : (
                <select value="Loading" disabled>
                  <option value="Loading">
                    {wifiApList === "scanning"
                      ? "スキャン中..."
                      : "スキャンが必要です"}
                  </option>
                </select>
              )}
              <button
                className="icon"
                onClick={handleRescan}
                disabled={wifiApList === "scanning"}
              >
                <Icon children={<MdReplay />} />
                再スキャン
              </button>
            </div>
            <div
              style={{
                marginBottom: 4,
                position: "relative",
                width: "fit-content",
              }}
            >
              <input
                {...passInput}
                placeholder="Password"
                type={showPass ? "text" : "password"}
                style={{ paddingRight: 90 }}
              />
              <button
                onClick={() => setShowPass(!showPass)}
                style={{
                  position: "absolute",
                  right: 10,
                  top: "50%",
                  transform: "translateY(-50%)",
                  padding: "2px 8px",
                }}
              >
                {showPass ? "非表示" : "表示"}
              </button>
            </div>
            <div
              style={{
                display: "grid",
                gridTemplateRows: showPass ? "1fr" : "0fr",
                transition: "grid-template-rows 0.2s",
              }}
            >
              <div style={{ overflowY: "hidden" }}>
                {checkJapaneseZenkaku(passInput.value).map((msg, i) => (
                  <p key={i} style={{ color: "red", display: "flex", gap: 4 }}>
                    <MdError />
                    <span style={{ flex: 1 }}>{msg}</span>
                  </p>
                ))}
                {(
                  [
                    { level: "high", color: "orange" },
                    { level: "low", color: "grey" },
                  ] as { level: "high" | "low"; color: string }[]
                ).map(({ level, color }) =>
                  checkConfusableCharacters(passInput.value, level).map(
                    (msg, i) => (
                      <p
                        key={i}
                        style={{ color: color, display: "flex", gap: 4 }}
                      >
                        <MdErrorOutline />
                        <span style={{ flex: 1 }}>{msg}</span>
                      </p>
                    ),
                  ),
                )}
              </div>
            </div>
            <div style={{ marginTop: 4 }}>
              <button
                onClick={handleSubmit}
                disabled={selectedSsid === "" || passInput.value === ""}
              >
                書き込む
              </button>
            </div>
          </>
        ) : mode === "writing" ? (
          <>
            <p>書き込み中...</p>
            <div>
              <select {...ssidSelectProps} disabled />
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
      </div>
      {mode === "scanning" && (
        <Backdrop alpha={0.4}>
          <p>Wi-Fiスキャン中...</p>
        </Backdrop>
      )}
    </>
  );
}
