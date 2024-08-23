import { useCallback } from "react";

export function CheckboxConfigEditor({
  hubHost,
  path,
  fieldKey,
  value,
  label,
}: {
  hubHost: string;
  path: string;
  fieldKey: string;
  value: boolean | undefined;
  label: React.ReactNode;
}) {
  const applyValue = useCallback(
    (newValue: boolean) =>
      fetch(`http://${hubHost}${path}`, {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ [fieldKey]: newValue }),
      }),
    [hubHost, path, fieldKey],
  );

  const handleChange = useCallback(
    (e: React.ChangeEvent<HTMLInputElement>) => applyValue(e.target.checked),
    [applyValue],
  );

  return (
    <label
      className={value === undefined ? "disabled" : undefined}
      style={{ display: "flex", gap: 4 }}
    >
      <input
        type="checkbox"
        checked={value ?? false}
        onChange={handleChange}
        disabled={value === undefined}
      />
      {label}
    </label>
  );
}
