// Version of VIMEK: version.json is the source, every other file follows it.
//
//   node scripts/set-version.mjs 0.2.0     set the version everywhere (and date the changelog)
//   node scripts/set-version.mjs --check   fail if a file disagrees with version.json, or, on a
//                                          tag build (GITHUB_REF_TYPE=tag), if the tag is not v<version>
//
// Rules: see "Phiên bản" in CONTRIBUTING.md.
import { readFileSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const SEMVER = /^(\d+)\.(\d+)\.(\d+)(?:-(rc\.\d+))?$/;

// [file, pattern with one capture group around the version, how the version is written there]
const core = (v) => v.split("-")[0]; // 0.2.0-rc.1 -> 0.2.0
const quad = (v) => `${core(v)}.0`; // Windows resources: four numbers
const PLACES = [
  ["CMakeLists.txt", /project\(VIMEK VERSION (\S+) /, core],
  ["Sources/VIMEK/windows/App/VIMEK.rc", /FILEVERSION (\S+)/, (v) => quad(v).replaceAll(".", ",")],
  ["Sources/VIMEK/windows/App/VIMEK.rc", /PRODUCTVERSION (\S+)/, (v) => quad(v).replaceAll(".", ",")],
  ["Sources/VIMEK/windows/App/VIMEK.rc", /"FileVersion", "([^"]+)"/, quad],
  ["Sources/VIMEK/windows/App/VIMEK.rc", /"ProductVersion", "([^"]+)"/, quad],
  ["Sources/VIMEK/windows/App/VIMEK.manifest", /assemblyIdentity version="([^"]+)"/, quad],
  ["Sources/VIMEK/windows/App/AboutDialog.cpp", /"VIMEK (\S+) chưa có kênh cập nhật/, (v) => v],
  ["Sources/VIMEK/windows/App/MainControlDialog.cpp", /"VIMEK (\S+) chưa có kênh cập nhật/, (v) => v],
  ["Sources/VIMEK/macOS/App/VimekManager.m", /@"VIMEK (\S+) chưa có kênh cập nhật/, (v) => v],
  ["Sources/VIMEK/macOS/App/Base.lproj/Main.storyboard", /title="VIMEK (\d[^"]*)" id="FiL-a8-YwW"/, (v) => v],
  ["Sources/VIMEK/macOS/VIMEK.xcodeproj/project.pbxproj", /MARKETING_VERSION = ([^;]+);/g, core],
  ["scripts/build-linux.sh", /vimek_version="\$\{1:-([^}]+)\}"/, (v) => v],
  ["scripts/package-windows.ps1", /\[string\]\$Version = '([^']+)'/, (v) => v],
  [".github/release-notes.md", /\.\/VIMEK-(\S+)-Linux-amd64\.deb/, (v) => v],
  ["README.md", /dist\/linux\/VIMEK-(\S+)-Linux-amd64\.deb/g, (v) => v],
  ["README.vi.md", /dist\/linux\/VIMEK-(\S+)-Linux-amd64\.deb/g, (v) => v],
  ["Linux_Build.md", /\.\/VIMEK-(\S+)-Linux-amd64\.deb/g, (v) => v],
  ["Linux_Build.md", /dist\/linux\/VIMEK-(\S+)-Linux-amd64\.deb/g, (v) => v],
  ["Linux_Build.md", /build-linux\.sh (\S+?)`/, (v) => v],
];

const read = (f) => readFileSync(join(root, f), "utf8");
const versionFile = JSON.parse(read("version.json"));

function apply(text, re, value, file, write) {
  const g = re.global ? re : new RegExp(re.source, re.flags + "g");
  const found = [...text.matchAll(g)];
  if (!found.length) throw new Error(`${file}: no version found for ${re}`);
  if (!re.global && found.length > 1) throw new Error(`${file}: ${re} matches ${found.length} times`);
  const wrong = found.filter((m) => m[1] !== value).map((m) => m[1]);
  if (!write) return { text, wrong };
  return { text: text.replace(g, (m, old) => m.replace(old, value)), wrong };
}

const args = process.argv.slice(2);
if (args[0] === "--check") {
  const v = versionFile.version;
  if (!SEMVER.test(v)) throw new Error(`version.json: ${v} is not X.Y.Z or X.Y.Z-rc.N`);
  const problems = [];
  for (const [file, re, fmt] of PLACES) {
    const { wrong } = apply(read(file), re, fmt(v), file, false);
    if (wrong.length) problems.push(`${file}: ${wrong.join(", ")} (expected ${fmt(v)})`);
  }
  if (process.env.GITHUB_REF_TYPE === "tag" && process.env.GITHUB_REF_NAME !== `v${v}`)
    problems.push(`tag ${process.env.GITHUB_REF_NAME} is not v${v} from version.json`);
  if (problems.length) {
    console.error("Version mismatch; run node scripts/set-version.mjs " + v + "\n  " + problems.join("\n  "));
    process.exit(1);
  }
  console.log(`VIMEK ${v}: every file agrees.`);
} else if (args.length === 1 && SEMVER.test(args[0])) {
  const v = args[0];
  const files = new Map();
  for (const [file, re, fmt] of PLACES) {
    files.set(file, apply(files.get(file) ?? read(file), re, fmt(v), file, true).text);
  }
  for (const [file, text] of files) writeFileSync(join(root, file), text);
  writeFileSync(join(root, "version.json"), JSON.stringify({ ...versionFile, version: v }, null, 2) + "\n");
  // A final release takes the changes collected under "Chưa phát hành".
  if (!v.includes("-")) {
    const log = read("CHANGELOG.md");
    if (!log.includes("## Chưa phát hành")) throw new Error('CHANGELOG.md: no "## Chưa phát hành" section');
    const date = new Date().toISOString().slice(0, 10);
    writeFileSync(join(root, "CHANGELOG.md"), log.replace("## Chưa phát hành", `## ${v} (${date})`));
  }
  console.log(`VIMEK ${v} set in version.json and ${files.size} files.`);
} else {
  console.error("usage: node scripts/set-version.mjs <X.Y.Z | X.Y.Z-rc.N> | --check");
  process.exit(2);
}
