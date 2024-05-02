import { useState, useEffect, useMemo, useCallback } from "react";

import { MdSettings } from "react-icons/md";

import { useKachakaButtonHub, useLocalStorageState } from "./hooks";

import { Button, CommandType, Command, GetButtonId } from "./types";
import { Backdrop } from "./Backdrop";
import { ClientCountWarning } from "./ClientCountWarning";
import { CommandEditor } from "./CommandEditor";
import { Icon } from "./Icon";
import { MainCardUi } from "./MainCardUi";
import { InitialUi } from "./InitialUi";
import { Sheet } from "./Sheet";
import { Modal } from "./Modal";
import { SettingPage } from "./SettingPage";
import { WiFiSignalLevel } from "./WiFiSignalLevel";
import { getHubHost } from "./utils";

const HUB_HOST = getHubHost();

export function App() {
  const { online, hubInfo, robotInfo, settings, buttons, commands, wifiRssi } =
    useKachakaButtonHub(HUB_HOST);

  const buttonIdToNameMap = useMemo(
    () =>
      new Map(
        buttons
          ?.map((button) => [GetButtonId(button), button.name])
          .filter((x): x is [string, string] => x[1] !== undefined),
      ),
    [buttons],
  );

  const [enableSheetUi, setEnableSheetUi] = useState(false);
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

  const [openSettings, setOpenSettings] = useState(false);
  const [showAllIBeacons, setShowAllIBeacons] = useLocalStorageState(
    "show_all_i_beacon",
    false,
  );
  useEffect(() => {
    if (!online) {
      setOpenSettings(false);
    }
  }, [online]);

  const useLockAndProceed = useMemo(() => {
    if (!robotInfo?.robot_version?.match(/^\d+\.\d+\.\d+$/)) {
      return true;
    }
    return (
      (robotInfo?.robot_version
        ?.split(".")
        .map((x) => parseInt(x, 10))
        .reduce((prev, cur) => prev * 1000 + cur, 0) ?? 0) > 2005999
    );
  }, [robotInfo?.robot_version]);

  return (
    <>
      <h1>カチャカボタンHub</h1>
      {robotInfo === undefined ||
      robotInfo.robot_version === undefined ||
      robotInfo.shelves === undefined ||
      robotInfo.locations === undefined ? (
        <InitialUi
          hubHost={HUB_HOST}
          robotInfo={robotInfo}
          settings={settings}
        />
      ) : enableSheetUi ? (
        <div>
          <Sheet
            commands={commands}
            buttonIdToNameMap={buttonIdToNameMap}
            robotInfo={robotInfo}
            onEdit={editCommand}
            onDelete={deleteCommand}
            onSetButtonName={setButtonName}
          />
        </div>
      ) : (
        <MainCardUi
          buttons={buttons}
          commands={commands}
          buttonIdToNameMap={buttonIdToNameMap}
          robotInfo={robotInfo}
          onEdit={editCommand}
          onDelete={deleteCommand}
          onSetButtonName={setButtonName}
          setEditingButton={setEditingButton}
          onDeleteButtonName={deleteButtonName}
          showAllIBeacons={showAllIBeacons}
          useLockAndProceed={useLockAndProceed}
        />
      )}
      <button
        className="icon"
        style={{ position: "absolute", top: 32, right: 16 }}
        onClick={() => setOpenSettings((prev) => !prev)}
      >
        <Icon children={<MdSettings />} />
      </button>
      <Modal open={openSettings} onClose={() => setOpenSettings(false)}>
        {openSettings && (
          <SettingPage
            hubHost={HUB_HOST}
            hubVersion={hubInfo?.hub_version}
            otaAvailable={hubInfo?.ota_available}
            otaLabel={hubInfo?.ota_label}
            settings={settings}
            robotHost={settings?.robot_host}
            enableSheetUi={enableSheetUi}
            setEnableSheetUi={setEnableSheetUi}
            showAllIBeacons={showAllIBeacons}
            setShowAllIBeacons={setShowAllIBeacons}
          />
        )}
      </Modal>
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
              closeDialog={() => setEditingButton(undefined)}
            />
          </>
        )}
      </Modal>
      <div style={{ position: "fixed", top: 2, right: 16 }}>
        <WiFiSignalLevel online={online} rssi={wifiRssi} />
      </div>
      {hubInfo && hubInfo.client_count > 2 && (
        <ClientCountWarning clientCount={hubInfo.client_count} />
      )}
      {progressCount > 0 && (
        <Backdrop alpha={0.4}>
          <p>処理中...</p>
        </Backdrop>
      )}
      {online === false && (
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
