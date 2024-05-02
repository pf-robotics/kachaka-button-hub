export function ClientCountWarning({ clientCount }: { clientCount: number }) {
  return (
    <div
      style={{
        position: "fixed",
        top: 8,
        left: 8,
        backgroundColor: "var(--ginger-yellow1)",
        color: "var(--status-danger)",
        padding: 16,
        fontSize: "small",
        boxShadow: "0 0 8px 0 rgba(0, 0, 0, 0.5)",
        maxWidth: "70vw",
        borderRadius: 8,
      }}
    >
      ⚠
      接続中の画面数が多くなっています。ボタンの動作に影響するため、接続数を減らしてください。
      <br />
      <b>接続中の設定画面 : {clientCount}</b>
    </div>
  );
}
