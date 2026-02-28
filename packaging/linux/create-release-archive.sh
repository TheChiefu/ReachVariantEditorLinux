#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

VERSION="${1:-dev}"
BUILD_DIR="${2:-build}"
STAGE_DIR="${3:-stage}"

ARCHIVE="ReachVariantTool-linux-x86_64-${VERSION}.tar.gz"

cd "${REPO_ROOT}"

cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --config Release -j

rm -rf "${STAGE_DIR}"
cmake --install "${BUILD_DIR}" --prefix "${REPO_ROOT}/${STAGE_DIR}"

tar -C "${STAGE_DIR}" -czf "${ARCHIVE}" .
sha256sum "${ARCHIVE}" > "${ARCHIVE}.sha256"

echo "Created ${ARCHIVE}"
echo "Created ${ARCHIVE}.sha256"
