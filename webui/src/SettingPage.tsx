import { useState, useEffect } from "react";

import { RobotHostEditor } from "./RobotHostEditor";
import { CheckboxConfigEditor } from "./CheckboxConfigEditor";
import { SliderConfigEditor } from "./SliderConfigEditor";
import { ButtonWithConfirmation } from "./ButtonWithConfirmation";
import { Settings } from "./types";
import { useCheckbox } from "./hooks";

export function SettingPage({
  hubHost,
  hubVersion,
  otaAvailable,
  otaLabel,
  settings,
  robotHost,
  enableSheetUi,
  setEnableSheetUi,
  showAllIBeacons,
  setShowAllIBeacons,
}: {
  hubHost: string;
  hubVersion: string | undefined;
  otaAvailable: boolean | undefined;
  otaLabel: string | undefined;
  settings: Settings | undefined;
  robotHost: string | undefined;
  enableSheetUi: boolean;
  setEnableSheetUi: (value: boolean) => void;
  showAllIBeacons: boolean;
  setShowAllIBeacons: (value: boolean) => void;
}) {
  const enableSheetUiInput = useCheckbox(enableSheetUi, setEnableSheetUi);
  const showAllIBeaconsInput = useCheckbox(showAllIBeacons, setShowAllIBeacons);

  const [manualOtaStarted, setManualOtaStarted] = useState(false);

  const handleManualOta = () => {
    setManualOtaStarted(true);
    fetch(`http://${hubHost}/ota/trigger_auto`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({}),
    });
  };
  const [desiredHubVersion, setDesiredHubVersion] = useState<string>();
  const [otaIsRequired, setOtaIsRequired] = useState<boolean>();
  useEffect(() => {
    fetch(`http://${hubHost}/ota/desired_hub_version`)
      .then((res) => res.json())
      .then((res) => {
        setDesiredHubVersion(res.desired_hub_version);
        setOtaIsRequired(res.ota_is_required);
      });
  }, [hubHost]);

  return (
    <>
      <h2>設定</h2>
      <div style={{ position: "relative", overflow: "auto" }}>
        <h3>カチャカのシリアル番号・IPアドレス</h3>
        <RobotHostEditor hubHost={hubHost} robotHost={robotHost} />

        <h3>音量</h3>
        <SliderConfigEditor
          hubHost={hubHost}
          path="/config/beep_volume"
          fieldKey="beep_volume"
          value={settings?.beep_volume}
          min={0}
          max={10}
          step={1}
        />

        <h3>画面の輝度</h3>
        <SliderConfigEditor
          hubHost={hubHost}
          path="/config/screen_brightness"
          fieldKey="screen_brightness"
          value={settings?.screen_brightness}
          min={1}
          max={255}
          step={1}
        />

        <h3>詳細設定</h3>
        <div style={{ display: "flex", gap: 8, alignItems: "center" }}>
          <label>
            <input {...enableSheetUiInput} />
            シートUIを有効にする
          </label>
        </div>
        <div style={{ display: "flex", gap: 8, alignItems: "center" }}>
          <label>
            <input {...showAllIBeaconsInput} />
            全てのiBeaconを表示
          </label>
        </div>
        <CheckboxConfigEditor
          hubHost={hubHost}
          path="/config/auto_refetch_on_ui_load"
          fieldKey="auto_refetch_on_ui_load"
          value={settings?.auto_refetch_on_ui_load}
          label="設定画面を開くたびに家具・目的地の情報をカチャカから再取得する"
        />

        <h3>Wi-Fiの設定</h3>
        <a href="/#wifi">Wi-Fiの設定画面</a>

        {otaAvailable === true && (
          <>
            <h3>システムアップデート</h3>
            <div
              style={{
                display: "flex",
                flexDirection: "column",
                gap: 16,
                alignItems: "start",
              }}
            >
              <CheckboxConfigEditor
                hubHost={hubHost}
                path="/config/auto_ota_is_enabled"
                fieldKey="auto_ota_is_enabled"
                value={settings?.auto_ota_is_enabled}
                label="自動アップデートを有効にする"
              />
              <div>
                {otaLabel && <div>リリースチャネル: {otaLabel}</div>}
                <div>
                  現在のバージョン:{" "}
                  {hubVersion ? <code>{hubVersion}</code> : "(取得中...)"}
                </div>
                <div>
                  最新バージョン:{" "}
                  {desiredHubVersion ? (
                    <code>{desiredHubVersion}</code>
                  ) : (
                    "(取得中...)"
                  )}
                </div>
              </div>
              <div
                className={
                  otaIsRequired !== true || manualOtaStarted
                    ? "disabled"
                    : undefined
                }
              >
                <button
                  disabled={otaIsRequired !== true || manualOtaStarted}
                  onClick={handleManualOta}
                >
                  アップデートを開始する
                </button>
              </div>
            </div>
          </>
        )}

        <h3>データの初期化</h3>
        <ButtonWithConfirmation
          onClick={() =>
            fetch(`http://${hubHost}/clear_all_data`).then(
              () => (window.location.hash = "#reboot-apply"),
            )
          }
          confirmText="ボタンHubの設定を全て消去しますか？"
          confirmButtonText="全て消去"
          style={{ margin: 4 }}
        >
          データを全て消去
        </ButtonWithConfirmation>

        <h3>法的情報</h3>
        <a href="/license.txt">サードパーティライセンス</a>
      </div>
    </>
  );
}
