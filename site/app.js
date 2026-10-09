// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
(() => {
  "use strict";
  const translations = {
    en: {
      skip: "Skip to content",
      navigation: "Main navigation",
      language: "Language",
      features: "Features",
      download: "Download",
      eyebrow: "A LITTLE TOOL. A NATURAL FLOW.",
      headline: "Vietnamese.",
      headlineAccent: "Naturally.",
      description:
        "Your words, without the extra effort. A familiar Vietnamese input method with a quiet interface, for Windows, macOS and Linux.",
      downloadWindows: "Download for Windows",
      downloadMac: "Download for macOS",
      downloadLinux: "Download for Linux",
      linuxDescription: "For Ubuntu/Debian with IBus. Unicode input.",
      linuxSetup: "Setup guide ↗",
      otherPlatforms: "Other downloads",
      free: "Free & open source",
      inYourElement: "IN YOUR ELEMENT",
      screenshot:
        "VIMEK control panel with input methods and Vietnamese/English switching",
      visualNote: "Less in the way. More room for your words.",
      typingExample: "Typing example",
      familiarRhythm: "A FAMILIAR RHYTHM",
      sameKeystrokes: "The keystrokes you already know.",
      exampleMethod: "Example input method",
      designedForEveryday: "DESIGNED FOR EVERYDAY",
      familiarSimple: "Familiar by feel.\nSimple by design.",
      typeYourWay: "Type your way.",
      typeYourWayBody:
        "Telex, VNI, and two Simple Telex variants. Keep the rhythm you've always used.",
      oneEasySwitch: "One easy switch.",
      oneEasySwitchBody:
        "Hold Ctrl, tap Alt. Switch between Vietnamese and English as often as you need, with an optional system sound.",
      macShortcut: "Control + Option on Mac",
      smallDetails: "Small details. Less effort.",
      smallDetailsBody:
        "Spelling checks and a familiar light or dark interface. Windows and macOS also offer text expansion and per-app mode memory.",
      followsSystem: "Follows your system appearance",
      makeYourselfAtHome: "MAKE YOURSELF AT HOME",
      readyWhenYouAre: "Ready when you are.",
      releaseNotes: "Release notes",
      windowsDescription: "Unzip, open VIMEK, and start typing.",
      downloadX64: "Download for Windows x64",
      windows32: "Windows 32-bit (x86) ↗",
      macDescription: "For Intel & Apple Silicon. macOS 11 or later.",
      macSetup: "Setup guide ↗",
      sourceCode: "Source code ↗",
      checksums: "SHA-256 checksums ↗",
      firstTime: "Before your first launch",
      signingNote:
        "Windows builds are currently unsigned; macOS builds are signed ad-hoc and are not notarized. Your operating system may show a security warning. Check the download source and the exact warning before proceeding.",
      setupNote:
        "On macOS, grant Accessibility permission when prompted, then reopen VIMEK. On Linux, install the DEB, log out and back in, and add Vietnamese → VIMEK to your IBus input sources. Select ENG on Windows or ABC/U.S. on macOS, and disable other Vietnamese input methods to avoid conflicts.",
      reportIssue: "Something not working? Report an issue ↗",
      builtInTheOpen: "BUILT IN THE OPEN",
      yourWords: "Your words. Your way.",
      openSourceBody:
        "Free to use, open to explore. Help shape what VIMEK becomes next.",
      exploreGithub: "Explore on GitHub",
      madeBy: "Made by",
      credits: "Credits",
      feedback: "Feedback ↗",
      themeDark: "Switch to dark mode",
      themeLight: "Switch to light mode",
      title: "VIMEK — Vietnamese. Naturally.",
    },
    vi: {
      skip: "Đi đến nội dung",
      navigation: "Điều hướng chính",
      language: "Ngôn ngữ",
      features: "Tính năng",
      download: "Tải về",
      eyebrow: "GỌN NHẸ. GÕ THẬT TỰ NHIÊN.",
      headline: "Tiếng Việt.",
      headlineAccent: "Thật tự nhiên.",
      description:
        "Viết điều bạn muốn, gõ theo cách bạn quen. Bộ gõ tiếng Việt với giao diện tinh gọn, dành cho Windows, macOS và Linux.",
      downloadWindows: "Tải cho Windows",
      downloadMac: "Tải cho macOS",
      downloadLinux: "Tải cho Linux",
      linuxDescription: "Cho Ubuntu/Debian dùng IBus. Hỗ trợ Unicode.",
      linuxSetup: "Hướng dẫn cài đặt ↗",
      otherPlatforms: "Các bản tải khác",
      free: "Miễn phí & mã nguồn mở",
      inYourElement: "QUEN THUỘC TỪ LẦN ĐẦU",
      screenshot:
        "Bảng điều khiển VIMEK với các kiểu gõ và nút chuyển Việt/Anh",
      visualNote: "Gọn hơn, để dành chỗ cho điều bạn muốn viết.",
      typingExample: "Ví dụ gõ tiếng Việt",
      familiarRhythm: "NHỊP GÕ QUEN THUỘC",
      sameKeystrokes: "Vẫn là những phím bạn đã quen.",
      exampleMethod: "Kiểu gõ minh họa",
      designedForEveryday: "CHO TỪNG NGÀY SỬ DỤNG",
      familiarSimple: "Quen thuộc khi gõ.\nTinh gọn khi dùng.",
      typeYourWay: "Gõ theo cách của bạn.",
      typeYourWayBody:
        "Telex, VNI và hai biến thể Simple Telex. Giữ nguyên thói quen gõ bạn đã quen thuộc.",
      oneEasySwitch: "Chuyển đổi thật nhẹ nhàng.",
      oneEasySwitchBody:
        "Giữ Ctrl, bấm Alt. Chuyển Việt/Anh nhiều lần khi cần, kèm âm thanh hệ thống có thể tắt.",
      macShortcut: "Control + Option trên Mac",
      smallDetails: "Bớt thao tác, thêm tiện lợi.",
      smallDetailsBody:
        "Kiểm tra chính tả và giao diện sáng/tối quen thuộc. Windows và macOS có thêm gõ tắt và nhớ chế độ theo ứng dụng.",
      followsSystem: "Theo giao diện sáng/tối của hệ điều hành",
      makeYourselfAtHome: "SẴN SÀNG ĐỂ BẮT ĐẦU",
      readyWhenYouAre: "Tải về. Gõ tự nhiên.",
      releaseNotes: "Thông tin bản phát hành",
      windowsDescription: "Giải nén, mở VIMEK và bắt đầu gõ.",
      downloadX64: "Tải cho Windows x64",
      windows32: "Windows 32-bit (x86) ↗",
      macDescription: "Cho Intel & Apple Silicon. Từ macOS 11.",
      macSetup: "Hướng dẫn cài đặt ↗",
      sourceCode: "Mã nguồn ↗",
      checksums: "Mã kiểm tra SHA-256 ↗",
      firstTime: "Trước khi mở lần đầu",
      signingNote:
        "Bản Windows hiện chưa có chữ ký số; bản macOS được ký ad-hoc và chưa notarize. Hệ điều hành có thể hiển thị cảnh báo bảo mật. Kiểm tra nguồn tải và nội dung cảnh báo trước khi tiếp tục.",
      setupNote:
        "Trên macOS, cấp quyền Accessibility khi được yêu cầu rồi mở lại VIMEK. Trên Linux, cài DEB, đăng xuất rồi đăng nhập lại và thêm Vietnamese → VIMEK trong nguồn nhập liệu IBus. Chọn ENG trên Windows hoặc ABC/U.S. trên macOS và tắt các bộ gõ tiếng Việt khác để tránh xung đột.",
      reportIssue: "Gặp vấn đề? Báo lỗi tại đây ↗",
      builtInTheOpen: "CÙNG NHAU PHÁT TRIỂN",
      yourWords: "Lời của bạn. Cách của bạn.",
      openSourceBody:
        "Tự do sử dụng, cùng nhau khám phá. Góp phần tạo nên phiên bản VIMEK tiếp theo.",
      exploreGithub: "Khám phá trên GitHub",
      madeBy: "Phát triển bởi",
      credits: "Ghi nhận",
      feedback: "Góp ý ↗",
      themeDark: "Chuyển sang giao diện tối",
      themeLight: "Chuyển sang giao diện sáng",
      title: "VIMEK — Gõ tiếng Việt thật tự nhiên.",
    },
  };
  const getPreference = (key) => {
    try {
      return localStorage.getItem(key);
    } catch {
      return null;
    }
  };
  const savePreference = (key, value) => {
    try {
      localStorage.setItem(key, value);
    } catch {
      /* Preferences are optional. */
    }
  };
  const scheme = window.matchMedia("(prefers-color-scheme: dark)");
  const themeButton = document.querySelector(".theme-button");
  const heroDownload = document.querySelector(".hero-download");
  const isMac = /Mac|iPhone|iPad/.test(navigator.userAgent);
  const isLinux = /Linux|X11/.test(navigator.userAgent) && !/Android/.test(navigator.userAgent);
  const platform = isMac ? "macos" : isLinux && !document.querySelector('[data-platform="linux"]').hidden ? "linux" : "windows-x64";
  let language = "en";
  let chosenTheme = getPreference("vimek-theme");
  if (!["dark", "light"].includes(chosenTheme)) chosenTheme = null;
  const dark = () => (chosenTheme ? chosenTheme === "dark" : scheme.matches);

  function refreshTheme() {
    if (chosenTheme) document.documentElement.dataset.theme = chosenTheme;
    else delete document.documentElement.dataset.theme;
    const label = translations[language][dark() ? "themeLight" : "themeDark"];
    themeButton.setAttribute("aria-label", label);
    themeButton.title = label;
    document.querySelector(".app-screenshot").src =
      `images/dashboard-${dark() ? "dark" : "light"}.png`;
    document.querySelector('meta[name="theme-color"]').content = dark()
      ? "#171c18"
      : "#f6f5f0";
  }
  function setLanguage(value) {
    language = value === "vi" ? "vi" : "en";
    const copy = translations[language];
    document.documentElement.lang = language;
    document.title = copy.title;
    document.querySelectorAll("[data-i18n]").forEach((element) => {
      const key = element.dataset.i18n;
      if (key === "releaseChannel")
        element.textContent = document.body.dataset.releaseChannel;
      else if (copy[key]) element.textContent = copy[key];
    });
    for (const [attribute, dataset] of [
      ["aria-label", "i18nAria"],
      ["alt", "i18nAlt"],
    ]) {
      document
        .querySelectorAll(
          `[data-${dataset === "i18nAria" ? "i18n-aria" : "i18n-alt"}]`,
        )
        .forEach((element) =>
          element.setAttribute(attribute, copy[element.dataset[dataset]]),
        );
    }
    document
      .querySelectorAll("[data-language]")
      .forEach((button) =>
        button.setAttribute(
          "aria-pressed",
          String(button.dataset.language === language),
        ),
      );
    const platformKey = platform === "macos" ? "downloadMac" : platform === "linux" ? "downloadLinux" : "downloadWindows";
    heroDownload.querySelector("span").textContent = copy[platformKey];
    refreshTheme();
  }
  document.querySelectorAll("[data-language]").forEach((button) =>
    button.addEventListener("click", () => {
      setLanguage(button.dataset.language);
      savePreference("vimek-language", language);
      const url = new URL(location.href);
      if (language === "vi") url.searchParams.set("lang", "vi");
      else url.searchParams.delete("lang");
      history.replaceState(null, "", url);
    }),
  );
  themeButton.addEventListener("click", () => {
    chosenTheme = dark() ? "light" : "dark";
    savePreference("vimek-theme", chosenTheme);
    refreshTheme();
  });
  scheme.addEventListener("change", refreshTheme);
  document.querySelectorAll("[data-method]").forEach((button) =>
    button.addEventListener("click", () => {
      document
        .querySelectorAll("[data-method]")
        .forEach((item) =>
          item.setAttribute("aria-pressed", String(item === button)),
        );
      document.querySelector("#example-input").textContent =
        button.dataset.method === "vni" ? "tie6ng1 Vie6t5" : "tieengs Vieetj";
    }),
  );
  heroDownload.href = document.querySelector(
    `[data-download="${platform}"]`,
  ).href;
  setLanguage(
    new URLSearchParams(location.search).get("lang") ||
      getPreference("vimek-language") ||
      "en",
  );
})();
