const IMMEDIATE_THRESHOLD = 1.0;
const NEAR_THRESHOLD = 8.0;

export function Distance({
  distance,
  showMeter,
}: {
  distance: number;
  showMeter?: boolean;
}) {
  let label = "遠い";
  let color = "lightgray";
  let precision = 0;
  if (distance < IMMEDIATE_THRESHOLD) {
    label = "非常に近い";
    color = "orange";
    precision = 1;
  } else if (distance < NEAR_THRESHOLD) {
    label = "近い";
    color = "green";
    precision = 1;
  }
  return (
    <span style={{ color }} title={`${distance.toFixed(precision)}m`}>
      {label}
      {showMeter ? ` (${distance.toFixed(1)}m)` : null}
    </span>
  );
}
