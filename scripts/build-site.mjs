// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
import { mkdir, readFile, writeFile, cp } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import path from "node:path";

const root = fileURLToPath(new URL("../", import.meta.url));
const output = path.join(root, "build/site");
const repository = "dinhgia2106/VIMEK";
const headers = {
  Accept: "application/vnd.github+json",
  "User-Agent": "VIMEK-Pages",
  "X-GitHub-Api-Version": "2022-11-28",
};
if (process.env.GITHUB_TOKEN)
  headers.Authorization = `Bearer ${process.env.GITHUB_TOKEN}`;
const response = await fetch(
  `https://api.github.com/repos/${repository}/releases?per_page=100`,
  { headers, signal: AbortSignal.timeout(30000) },
);
if (!response.ok)
  throw new Error(`GitHub release lookup failed: HTTP ${response.status}`);
const releases = (await response.json())
  .filter((release) => !release.draft)
  .sort((a, b) => new Date(b.published_at) - new Date(a.published_at));
const release = releases.find((item) => !item.prerelease) || releases[0];
if (!release)
  throw new Error(
    "Publish a VIMEK release before deploying the download page.",
  );
const asset = (pattern) => {
  const item = release.assets.find((value) => pattern.test(value.name));
  if (
    !item ||
    !item.browser_download_url.startsWith(
      `https://github.com/${repository}/releases/download/`,
    )
  )
    throw new Error(
      `Release ${release.tag_name} is missing a downloadable asset: ${pattern}`,
    );
  return item;
};
const windows64 = asset(/-Windows-x64\.zip$/i);
const windows32 = asset(/-Windows-x86\.zip$/i);
const macos = asset(/-macOS-universal\.zip$/i);
const source = asset(/-source\.zip$/i);
const checksums = asset(/^SHA256SUMS\.txt$/i);
const size = (item) =>
  item.size < 1048576
    ? `${Math.round(item.size / 1024)} KB`
    : `${(item.size / 1048576).toFixed(1)} MB`;
const values = {
  VERSION: release.tag_name.replace(/^v/, ""),
  RELEASE_CHANNEL: release.prerelease ? "Alpha" : "Stable",
  RELEASE_URL: release.html_url,
  WINDOWS_X64_URL: windows64.browser_download_url,
  WINDOWS_X86_URL: windows32.browser_download_url,
  MACOS_URL: macos.browser_download_url,
  SOURCE_URL: source.browser_download_url,
  CHECKSUM_URL: checksums.browser_download_url,
  WINDOWS_X64_SIZE: size(windows64),
  MACOS_SIZE: size(macos),
};
const escape = (value) =>
  String(value).replace(
    /[&<>"']/g,
    (char) =>
      ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" })[
        char
      ],
  );
const template = await readFile(path.join(root, "site/index.html"), "utf8");
const html = template.replace(/\{\{([A-Z0-9_]+)\}\}/g, (_, key) => {
  if (!(key in values)) throw new Error(`Unknown page placeholder: ${key}`);
  return escape(values[key]);
});
await mkdir(output, { recursive: true });
await cp(path.join(root, "site"), output, { recursive: true });
await cp(path.join(root, "docs/images"), path.join(output, "images"), {
  recursive: true,
});
await writeFile(path.join(output, "index.html"), html);
await writeFile(
  path.join(output, "release.json"),
  JSON.stringify(
    {
      tag: release.tag_name,
      prerelease: release.prerelease,
      url: release.html_url,
      publishedAt: release.published_at,
      assets: [windows64, windows32, macos, source, checksums].map(
        ({ name, size, browser_download_url }) => ({
          name,
          size,
          url: browser_download_url,
        }),
      ),
    },
    null,
    2,
  ) + "\n",
);
console.log(`Built build/site with downloads from ${release.tag_name}.`);
