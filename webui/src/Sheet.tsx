import { Fragment, useMemo } from "react";

import { Button, Command, CommandType, RobotInfo, GetButtonId } from "./types";
import { ButtonNameEditor } from "./ButtonNameEditor";
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

function CreateButtonGroup(
  commands: { button: Button; command: Command }[],
): ButtonGroup[] {
  const out: ButtonGroup[] = [];
  for (const { button, command } of commands) {
    if ("apple_i_beacon" in button) {
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
      const item = group.button.get(id);
      if (item) {
        item.command = command;
      }
    } else if ("m5_button" in button) {
      const id = GetButtonId(button);
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
      const item = group.button.get(id);
      if (item) {
        item.command = command;
      }
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
  commands,
  buttonIdToNameMap,
  robotInfo,
  onEdit,
  onDelete,
  onSetButtonName,
}: {
  commands: { button: Button; command: Command }[] | undefined;
  buttonIdToNameMap: Map<string, string>;
  robotInfo: RobotInfo | undefined;
  onEdit: (button: Button, command: Command) => Promise<void>;
  onDelete: (button: Button) => Promise<void>;
  onSetButtonName: (button: Button, name: string) => Promise<void>;
}) {
  const groups = useMemo(() => CreateButtonGroup(commands ?? []), [commands]);

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
                      index === 0 ? (
                        <ButtonNameEditor
                          button={button}
                          name={
                            buttonIdToNameMap.get(GetButtonId(button)) ?? ""
                          }
                          onSetButtonName={onSetButtonName}
                        />
                      ) : undefined
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
                      index === 0 ? (
                        <ButtonNameEditor
                          button={button}
                          name={
                            buttonIdToNameMap.get(GetButtonId(button)) ?? ""
                          }
                          onSetButtonName={onSetButtonName}
                        />
                      ) : undefined
                    }
                    itemLabel={label}
                    button={button}
                    command={
                      command ?? {
                        type: CommandType.RETURN_HOME,
                        cancel_all: false,
                      }
                    }
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
