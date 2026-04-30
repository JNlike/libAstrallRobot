#!/usr/bin/env bash
set -euo pipefail

IFACE="${1:-}"

if [[ -z "${IFACE}" ]]; then
  echo "Usage: $0 <network-interface>"
  echo
  echo "Available interfaces:"
  ip -brief addr
  exit 2
fi

echo "Checking local 10.18.0.x address on ${IFACE}"
ip -4 addr show dev "${IFACE}" | grep -q "10\.18\.0\." || {
  echo "WARN: ${IFACE} does not appear to have a 10.18.0.x IPv4 address."
  echo "      The Astrall SDK host is usually configured as 10.18.0.200."
}

echo "Pinging front LiDAR 10.18.0.120"
ping -c 3 -W 1 10.18.0.120

echo "Pinging rear LiDAR 10.18.0.121"
ping -c 3 -W 1 10.18.0.121

cat <<'EOF'

UDP capture commands:

  Front LiDAR MSOP:
    sudo tcpdump -i <iface> udp port 6699

  Front LiDAR IMU:
    sudo tcpdump -i <iface> udp port 6688

  Rear LiDAR MSOP:
    sudo tcpdump -i <iface> udp port 6969

  Rear LiDAR IMU:
    sudo tcpdump -i <iface> udp port 6868

Capturing UDP packets only proves that the LiDAR network path is alive.
PointCloud2 still requires the vendor SDK, protocol parser, or an existing ROS2 driver.
EOF
