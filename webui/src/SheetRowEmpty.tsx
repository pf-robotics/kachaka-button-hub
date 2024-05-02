export function SheetRowEmpty({
  groupSpan,
  groupLabel,
  itemLabel,
  onAdd,
}: {
  groupSpan: number;
  groupLabel?: JSX.Element;
  itemLabel?: string;
  onAdd: () => Promise<void>;
}) {
  return (
    <tr>
      {groupSpan === 0 ? null : <td rowSpan={groupSpan}>{groupLabel}</td>}
      <td>{itemLabel}</td>
      <td></td>
      <td></td>
      <td></td>
      <td></td>
      <td></td>
      <td></td>
      <td>
        <button onClick={onAdd}>追加</button>
      </td>
    </tr>
  );
}
