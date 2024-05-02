import { useState, useCallback, useEffect, useMemo, useRef } from "react";

import { debounce } from "ts-debounce";

import { useRangeInput } from "./hooks";

function useValue(
  hubHost: string,
  path: string,
  fieldKey: string,
): [number | undefined, (volume: number) => void, (volume: number) => void] {
  const [localValue, setLocalValue] = useState<number>();
  const setRemoteValue = useCallback(
    (newValue: number) => {
      fetch(`http://${hubHost}${path}`, {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ [fieldKey]: newValue }),
      });
    },
    [hubHost, path, fieldKey],
  );
  return [localValue, setLocalValue, setRemoteValue];
}

export function SliderConfigEditor({
  hubHost,
  path,
  fieldKey,
  value,
  min,
  max,
  step,
}: {
  hubHost: string;
  path: string;
  fieldKey: string;
  value: number | undefined;
  min: number;
  max: number;
  step: number;
}) {
  const [localValue, setLocalValue, setRemoteValue] = useValue(
    hubHost,
    path,
    fieldKey,
  );
  const first = useRef(true);
  useEffect(() => {
    if (value !== undefined && first.current) {
      setLocalValue(value);
      first.current = false;
    }
  }, [value, setLocalValue]);
  const debouncedSetValue: (volume: number) => void = useMemo(
    () => debounce(setRemoteValue, 200),
    [setRemoteValue],
  );
  const setValue = useCallback(
    (value: number) => {
      setLocalValue(value);
      debouncedSetValue(value);
    },
    [setLocalValue, debouncedSetValue],
  );
  const beepVolumeInput = useRangeInput(localValue, min, max, step, setValue);

  return (
    <div style={{ display: "flex", gap: 8, alignItems: "center" }}>
      <input {...beepVolumeInput} />
      {value}
    </div>
  );
}
