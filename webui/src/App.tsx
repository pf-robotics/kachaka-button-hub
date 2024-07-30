import { useState, useEffect, useMemo, useCallback } from "react";

import { useKachakaButtonHub, useFilteredCommandsAndButtons } from "./hooks";

import { Button, CommandType, Command, GetButtonId } from "./types";
import { Backdrop } from "./Backdrop";
import { StackedWarning } from "./StackedWarning";
import { CommandEditor } from "./CommandEditor";
import { MainCardUi } from "./MainCardUi";
import { InitialUi } from "./InitialUi";
import { Sheet } from "./Sheet";
import { Modal } from "./Modal";
import { SettingPage } from "./SettingPage";
import { WiFiSignalLevel } from "./WiFiSignalLevel";
import { getHubHost, isValidVersion, getIntVersion } from "./utils";
import { Page, BottomNav } from "./BottomNav";
import { LogList } from "./LogList";
import { TopHeader } from "./TopHeader";

const HUB_HOST = getHubHost();

export function App({
  page,
  setPage,
}: {
  page: Page;
  setPage: (page: Page) => void;
}) {
  const {
    networkState,
    hubInfo,
    robotInfo,
    settings,
    buttons,
    commands,
    wifiRssi,
  } = useKachakaButtonHub(HUB_HOST);

  const [filterredCommands, filterredButtons] = useFilteredCommandsAndButtons(
    commands,
    buttons,
    settings?.gpio_button_is_enabled ?? false,
  );

  const buttonIdToNameMap = useMemo(
    () =>
      new Map(
        filterredButtons
          ?.map((button) => [GetButtonId(button), button.name])
          .filter((x): x is [string, string] => x[1] !== undefined),
      ),
    [filterredButtons],
  );

  const [progressCount, setProgressCount] = useState(0);

  const editCommand = useCallback((button: Button, command: Command) => {
    setProgressCount((prev) => prev + 1);
    return fetch(`http://${HUB_HOST}/commands`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ button, command }),
    })
      .catch((err) => console.error(err))
      .then(() => setProgressCount((prev) => prev - 1));
  }, []);
  const deleteCommand = useCallback((button: Button) => {
    setProgressCount((prev) => prev + 1);
    return fetch(`http://${HUB_HOST}/commands`, {
      method: "DELETE",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ button }),
    })
      .catch((err) => console.error(err))
      .then(() => setProgressCount((prev) => prev - 1));
  }, []);
  const setButtonName = useCallback((button: Button, name: string) => {
    setProgressCount((prev) => prev + 1);
    return fetch(`http://${HUB_HOST}/buttons`, {
      method: "PUT",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ button, name }),
    })
      .catch((err) => console.error(err))
      .then(() => setProgressCount((prev) => prev - 1));
  }, []);
  const deleteButtonName = useCallback((button: Button) => {
    setProgressCount((prev) => prev + 1);
    return fetch(`http://${HUB_HOST}/buttons`, {
      method: "DELETE",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ button, name }),
    })
      .catch((err) => console.error(err))
      .then(() => setProgressCount((prev) => prev - 1));
  }, []);
  const deleteLog = useCallback((path: string) => {
    setProgressCount((prev) => prev + 1);
    return fetch(`http://${HUB_HOST}/log`, {
      method: "DELETE",
      body: path,
    }).then(() => setProgressCount((prev) => prev - 1));
  }, []);

  const [editingButton, setEditingButton] = useState<Button>();
  const newCommand = useMemo<Command>(() => {
    const name = editingButton
      ? buttonIdToNameMap.get(GetButtonId(editingButton))
      : undefined;
    return {
      type: CommandType.SPEAK,
      speak: { text: name ?? "新しいボタンです" },
      cancel_all: false,
      lock_duration_sec: 0.0,
      deferrable: false,
    };
  }, [editingButton, buttonIdToNameMap]);
  const addNewCommand = useCallback(
    (command: Command) => editingButton && editCommand(editingButton, command),
    [editingButton, editCommand],
  );

  const robotVersionInt = useMemo(() => {
    const version = robotInfo?.robot_version;
    if (version === undefined || !isValidVersion(version)) {
      return undefined;
    }
    return getIntVersion(version);
  }, [robotInfo?.robot_version]);
  const useLockAndProceed =
    robotVersionInt === undefined || robotVersionInt >= getIntVersion("2.6.0");
  const enableShortcutFeature =
    robotVersionInt === undefined || robotVersionInt >= getIntVersion("3.1.0");

  const [warnings, setWarnings] = useState<JSX.Element[]>([]);
  useEffect(() => {
    const out: JSX.Element[] = [];
    if (hubInfo && hubInfo.client_count > 2) {
      out.push(
        <>
          ⚠
          接続中の画面数が多くなっています。ボタンの動作に影響するため、接続数を減らしてください。
          <br />
          <b>接続中の設定画面 : {hubInfo.client_count}</b>
        </>,
      );
    }
    if (networkState === "unstable") {
      out.push(<>⚠ ネットワーク接続が不安定です。</>);
    }
    setWarnings(out);
  }, [hubInfo, networkState]);

  return (
    <>
      <TopHeader>
        {page === "home"
          ? "カチャカボタンHub"
          : page === "sheet"
            ? "表形式"
            : page === "info"
              ? "デバッグログ"
              : page === "settings"
                ? "設定"
                : ""}
      </TopHeader>
      <div className="content">
        {page === "home" ? (
          robotInfo === undefined ||
          robotInfo.robot_version === undefined ||
          robotInfo.shelves === undefined ||
          robotInfo.locations === undefined ||
          robotInfo.shortcuts === undefined ? (
            <InitialUi
              hubHost={HUB_HOST}
              robotInfo={robotInfo}
              settings={settings}
            />
          ) : (
            <MainCardUi
              buttons={filterredButtons}
              commands={filterredCommands}
              buttonIdToNameMap={buttonIdToNameMap}
              robotInfo={robotInfo}
              onEdit={editCommand}
              onDelete={deleteCommand}
              onSetButtonName={setButtonName}
              setEditingButton={setEditingButton}
              onDeleteButtonName={deleteButtonName}
              useLockAndProceed={useLockAndProceed}
              enableShortcutFeature={enableShortcutFeature}
            />
          )
        ) : page === "sheet" ? (
          <Sheet
            buttons={filterredButtons}
            commands={filterredCommands}
            buttonIdToNameMap={buttonIdToNameMap}
            robotInfo={robotInfo}
            enableShortcutFeature={enableShortcutFeature}
            onEdit={editCommand}
            onDelete={deleteCommand}
            onSetButtonName={setButtonName}
            onDeleteButtonName={deleteButtonName}
          />
        ) : page === "info" ? (
          <LogList hubHost={HUB_HOST} onDelete={deleteLog} />
        ) : page === "settings" ? (
          <SettingPage
            hubHost={HUB_HOST}
            hubVersion={hubInfo?.hub_version}
            otaAvailable={hubInfo?.ota_available}
            otaLabel={hubInfo?.ota_label}
            settings={settings}
            robotHost={settings?.robot_host}
          />
        ) : null}
      </div>
      <BottomNav page={page} onPageChange={setPage} />

      <Modal open={!!editingButton} onClose={() => setEditingButton(undefined)}>
        {editingButton && (
          <>
            <div style={{ margin: 8, marginRight: 32, fontWeight: "bold" }}>
              {buttonIdToNameMap.get(GetButtonId(editingButton)) ??
                "新規ボタン"}
            </div>
            <CommandEditor
              key={GetButtonId(editingButton)}
              command={newCommand}
              robotInfo={robotInfo}
              onSubmit={addNewCommand}
              useLockAndProceed={useLockAndProceed}
              enableShortcutFeature={enableShortcutFeature}
              closeDialog={() => setEditingButton(undefined)}
            />
          </>
        )}
      </Modal>
      <div
        style={{
          position: "fixed",
          top: 14,
          right: 16,
          zIndex: 1,
          fontSize: "1.2em",
        }}
      >
        <WiFiSignalLevel online={networkState === "online"} rssi={wifiRssi} />
      </div>
      {warnings.length > 0 && <StackedWarning items={warnings} />}
      {progressCount > 0 && (
        <Backdrop alpha={0.4}>
          <p>処理中...</p>
        </Backdrop>
      )}
      {networkState === "offline" && (
        <Backdrop>
          <p>カチャカボタンHubに接続を試みています</p>
          <p>
            <a href="#" onClick={() => window.location.reload()}>
              リロード
            </a>
          </p>
        </Backdrop>
      )}
    </>
  );
}
