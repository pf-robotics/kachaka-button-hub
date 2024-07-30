import { useEffect } from "react";

export function Rebooting({ apply }: { apply?: boolean }) {
  useEffect(() => {
    setTimeout(() => {
      window.location.hash = "";
    }, 7000);
  }, []);
  return (
    <div
      style={{
        width: "100vw",
        height: "100dvh",
        display: "flex",
        justifyContent: "center",
        alignItems: "center",
      }}
    >
      {apply ? (
        <>
          設定を有効にするために
          <br />
          Hubが再起動しています...
        </>
      ) : (
        <>Hubを再起動しています...</>
      )}
    </div>
  );
}
