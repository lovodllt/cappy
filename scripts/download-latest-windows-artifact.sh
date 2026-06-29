#!/usr/bin/env bash
set -euo pipefail

repo="${1:-lovodllt/cappy}"
branch="${2:-main}"
artifact_name="${3:-cappy-windows-packages}"
output_dir="${4:-dist/windows-artifact}"

require_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Missing required command: $1" >&2
        exit 1
    fi
}

require_cmd curl
require_cmd jq
require_cmd unzip

token="${GH_TOKEN:-${GITHUB_TOKEN:-}}"
if [[ -z "${token}" ]] && command -v gh >/dev/null 2>&1; then
    token="$(gh auth token 2>/dev/null || true)"
fi

if [[ -z "${token}" ]]; then
    echo "GitHub token is required to download Actions artifacts." >&2
    echo "Set GH_TOKEN or GITHUB_TOKEN, or authenticate with gh." >&2
    exit 1
fi

api_base="https://api.github.com/repos/${repo}"
auth_header="Authorization: Bearer ${token}"
accept_header="Accept: application/vnd.github+json"

run_json="$(
    curl -fsSL \
        -H "${auth_header}" \
        -H "${accept_header}" \
        "${api_base}/actions/runs?branch=${branch}&status=success&per_page=20"
)"

run_id="$(
    jq -r '
        .workflow_runs[]
        | select(.name == "windows")
        | .id
        ' <<<"${run_json}" | head -n 1
)"

if [[ -z "${run_id}" || "${run_id}" == "null" ]]; then
    echo "No successful windows workflow run found for ${repo} on branch ${branch}." >&2
    exit 1
fi

artifact_json="$(
    curl -fsSL \
        -H "${auth_header}" \
        -H "${accept_header}" \
        "${api_base}/actions/runs/${run_id}/artifacts"
)"

artifact_id="$(
    jq -r --arg name "${artifact_name}" '
        .artifacts[]
        | select(.name == $name and (.expired | not))
        | .id
        ' <<<"${artifact_json}" | head -n 1
)"

if [[ -z "${artifact_id}" || "${artifact_id}" == "null" ]]; then
    echo "No non-expired artifact named ${artifact_name} found in run ${run_id}." >&2
    exit 1
fi

mkdir -p "${output_dir}"
archive_path="${output_dir}/${artifact_name}.zip"

curl -fsSL \
    -H "${auth_header}" \
    -H "${accept_header}" \
    -L \
    -o "${archive_path}" \
    "${api_base}/actions/artifacts/${artifact_id}/zip"

unzip -o "${archive_path}" -d "${output_dir}" >/dev/null

echo "Downloaded artifact:"
echo "  repo: ${repo}"
echo "  branch: ${branch}"
echo "  run_id: ${run_id}"
echo "  artifact_id: ${artifact_id}"
echo "  archive: ${archive_path}"
echo "Extracted files:"
find "${output_dir}" -maxdepth 2 -type f | sort
