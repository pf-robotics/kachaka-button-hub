export function DurationText({ durationMsec }: { durationMsec: number }) {
  const sec = durationMsec / 1000;
  if (sec < 60) {
    return <>{sec.toFixed(0)}秒</>;
  }
  const min = sec / 60;
  if (min < 60) {
    return <>{min.toFixed(0)}分</>;
  }
  const hour = min / 60;
  if (hour < 24) {
    return <>{hour.toFixed(0)}時間</>;
  }
  const day = hour / 24;
  return <>{day.toFixed(0)}日</>;
}
