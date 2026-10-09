# VIMEK trên macOS

## Yêu cầu

- macOS 11 trở lên.
- Xcode và Command Line Tools.
- Intel hoặc Apple Silicon.

## Build

Từ thư mục gốc dự án:

```bash
bash scripts/build-macos.sh
```

Script chạy kiểm thử, build ứng dụng universal cho Intel và Apple Silicon,
đóng gói helper khởi động và ký ad-hoc cho bản build cục bộ.
Đầu ra: `dist/macos/VIMEK.app` và `dist/macos/VIMEK-macOS.zip`.

Để làm việc trong Xcode, mở `Sources/VIMEK/macOS/VIMEK.xcodeproj`
và chọn scheme **VIMEK**.

## Cài đặt và sử dụng

1. Chép `VIMEK.app` vào **Applications** và mở ứng dụng.
2. Cấp quyền **Accessibility** tại **System Settings → Privacy & Security**.
3. Mở lại VIMEK, chọn input source **ABC/U.S.** và tắt các bộ gõ khác.
4. Chọn Telex hoặc VNI từ menu VIMEK trên menu bar.

Phím chuyển mặc định là **Control+Option**. Giữ Control, bấm rồi thả Option
để chuyển Việt/Anh; có thể bấm Option nhiều lần khi vẫn giữ Control.
Âm thanh và phím tắt được điều chỉnh trong **Nâng cao**.

## Phân phối

Bản build cục bộ dùng chữ ký ad-hoc. Để phân phối qua Internet, ký bằng
Developer ID và notarize bằng công cụ của Apple.
Phân phối mã nguồn tương ứng cùng [LICENSE](LICENSE) và [NOTICE.md](NOTICE.md).
