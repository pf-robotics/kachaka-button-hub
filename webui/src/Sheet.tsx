import { Fragment, useCallback, useMemo } from "react";

import {
  AppleIBeacon,
  M5Button,
  GpioButton,
  Button,
  Command,
  CommandType,
  RobotInfo,
  GetButtonId,
  GetButtonName,
} from "./types";
import { ButtonNameEditor } from "./ButtonNameEditor";
import { Distance } from "./Distance";
import { DurationText } from "./DurationText";
import { SheetRow } from "./SheetRow";
import { SheetRowEmpty } from "./SheetRowEmpty";

interface ButtonGroup {
  address: string;
  button: Map<
    string,
    {
      label: string;
      button: Button;
      command?: Command;
    }
  >;
}

function AddAppleIBeacon(
  button: { apple_i_beacon: AppleIBeacon },
  out: ButtonGroup[],
) {
  const id = GetButtonId(button);
  const idPrefix = id.substr(0, id.length - 2);
  let group = out.find((x) => x.address === button.apple_i_beacon.address);
  if (!group) {
    const buttonBase = { ...button };
    group = {
      address: button.apple_i_beacon.address,
      button: new Map([
        [
          `${idPrefix}:1`,
          {
            label: "1回押し",
            button: {
              ...buttonBase,
              apple_i_beacon: { ...buttonBase.apple_i_beacon, major: 1 },
            },
          },
        ],
        [
          `${idPrefix}:2`,
          {
            label: "2回押し",
            button: {
              ...buttonBase,
              apple_i_beacon: {
                ...buttonBase.apple_i_beacon,
                major: 12289,
              },
            },
          },
        ],
        [
          `${idPrefix}:L`,
          {
            label: "長押し",
            button: {
              ...buttonBase,
              apple_i_beacon: { ...buttonBase.apple_i_beacon, major: 4097 },
            },
          },
        ],
      ]),
    };
    out.push(group);
  }
}

function AddM5Button(button: { m5_button: M5Button }, out: ButtonGroup[]) {
  let group = out.find((x) => x.address === "m5_button");
  if (!group) {
    const buttonBase = { ...button };
    group = {
      address: "m5_button",
      button: new Map([
        [
          `m5_button:2`,
          {
            label: "HubボタンA",
            button: { ...buttonBase, m5_button: { id: 2 } },
          },
        ],
        [
          `m5_button:3`,
          {
            label: "HubボタンB",
            button: { ...buttonBase, m5_button: { id: 3 } },
          },
        ],
      ]),
    };
    out.push(group);
  }
}

function AddGpioButton(
  button: { gpio_button: GpioButton },
  out: ButtonGroup[],
) {
  let group = out.find((x) => x.address === "gpio_button");
  if (!group) {
    const buttonBase = { ...button };
    group = {
      address: "gpio_button",
      button: new Map(
        [1, 2, 3, 4, 5, 6].map((id) => [
          `gpio_button:${id}`,
          {
            label: `HubPlusボタン${id}`,
            button: { ...buttonBase, gpio_button: { id } },
          },
        ]),
      ),
    };
    out.push(group);
  }
}

function CreateButtonGroup(
  buttons: Button[],
  commands: { button: Button; command: Command }[],
): ButtonGroup[] {
  const out: ButtonGroup[] = [];
  for (const { button, command } of commands) {
    if ("apple_i_beacon" in button) {
      AddAppleIBeacon(button, out);
      const group = out.find(
        (x) => x.address === button.apple_i_beacon.address,
      );
      const item = group?.button.get(GetButtonId(button));
      if (item) {
        item.command = command;
      }
    } else if ("m5_button" in button) {
      AddM5Button(button, out);
      const group = out.find((x) => x.address === "m5_button");
      const item = group?.button.get(GetButtonId(button));
      if (item) {
        item.command = command;
      }
    } else if ("gpio_button" in button) {
      AddGpioButton(button, out);
      const group = out.find((x) => x.address === "gpio_button");
      const item = group?.button.get(GetButtonId(button));
      if (item) {
        item.command = command;
      }
    }
  }
  for (const button of buttons) {
    if ("apple_i_beacon" in button) {
      AddAppleIBeacon(button, out);
    } else if ("m5_button" in button) {
      AddM5Button(button, out);
    } else if ("gpio_button" in button) {
      AddGpioButton(button, out);
    }
  }
  return out;
}

function SpacerRow() {
  return (
    <tr className="spacer">
      <td />
      <td />
      <td />
      <td />
      <td />
      <td />
      <td />
      <td />
      <td />
    </tr>
  );
}

export function Sheet({
  buttons,
  commands,
  buttonIdToNameMap,
  robotInfo,
  enableShortcutFeature,
  onEdit,
  onDelete,
  onSetButtonName,
  onDeleteButtonName,
}: {
  buttons: Button[] | undefined;
  commands: { button: Button; command: Command }[] | undefined;
  buttonIdToNameMap: Map<string, string>;
  robotInfo: RobotInfo | undefined;
  enableShortcutFeature: boolean;
  onEdit: (button: Button, command: Command) => Promise<void>;
  onDelete: (button: Button) => Promise<void>;
  onSetButtonName: (button: Button, name: string) => Promise<void>;
  onDeleteButtonName: (button: Button) => Promise<void>;
}) {
  const groups = useMemo(
    () => CreateButtonGroup(buttons ?? [], commands ?? []),
    [buttons, commands],
  );

  const createGroupLabel = useCallback(
    (button: Button) => (
      <>
        <ButtonNameEditor
          button={button}
          name={
            buttonIdToNameMap.get(GetButtonId(button)) ?? GetButtonName(button)
          }
          onSetButtonName={onSetButtonName}
          onDeleteButtonName={onDeleteButtonName}
        />
        <div style={{ display: "flex", justifyContent: "space-between" }}>
          {button.timestamp ? (
            <span>
              <DurationText
                durationMsec={Date.now() - button.timestamp.getTime()}
              />
              前
            </span>
          ) : (
            <span />
          )}
          {button.estimated_distance !== undefined ? (
            <Distance distance={button.estimated_distance} />
          ) : (
            <span />
          )}
        </div>
      </>
    ),
    [buttonIdToNameMap, onSetButtonName, onDeleteButtonName],
  );

  return (
    <table>
      <tbody>
        <tr>
          <th>代表の名前</th>
          <th>ボタン種類</th>
          <th>コマンドタイプ</th>
          <th>コマンド詳細</th>
          <th>
            他を
            <br />
            キャン
            <br />
            セル
          </th>
          <th>
            後回し
            <br />
            にして
            <br />
            よい
          </th>
          <th>コマンド成功後に発話</th>
          <th>到着後待機</th>
          <th></th>
        </tr>
        {groups.map(({ address, button }) => (
          <Fragment key={address}>
            <SpacerRow />
            {Array.from(button).map(
              ([id, { label, button, command }], index, array) =>
                command === undefined ? (
                  <SheetRowEmpty
                    key={id}
                    groupSpan={index === 0 ? array.length : 0}
                    groupLabel={
                      index === 0 ? createGroupLabel(button) : undefined
                    }
                    itemLabel={label}
                    onAdd={() =>
                      onEdit(button, {
                        type: CommandType.RETURN_HOME,
                        cancel_all: false,
                      })
                    }
                  />
                ) : (
                  <SheetRow
                    key={GetButtonId(button)}
                    groupSpan={index === 0 ? array.length : 0}
                    groupLabel={
                      index === 0 ? createGroupLabel(button) : undefined
                    }
                    itemLabel={label}
                    button={button}
                    command={
                      command ?? {
                        type: CommandType.RETURN_HOME,
                        cancel_all: false,
                      }
                    }
                    enableShortcutFeature={enableShortcutFeature}
                    robotInfo={robotInfo}
                    onEdit={onEdit}
                    onDelete={onDelete}
                  />
                ),
            )}
          </Fragment>
        ))}
      </tbody>
    </table>
  );
}
