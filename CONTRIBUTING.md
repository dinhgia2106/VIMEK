# Đóng góp cho VIMEK

Cảm ơn bạn đã quan tâm đến VIMEK. Dự án được phát triển và duy trì bởi **GrazT**.

## Báo lỗi và đề xuất

Khi báo lỗi, hãy cung cấp phiên bản VIMEK, hệ điều hành, ứng dụng đang dùng,
kiểu gõ và các bước tái hiện. Với lỗi gõ tiếng Việt, kèm chuỗi phím đã nhập,
kết quả thực tế và kết quả mong đợi.

Bạn có thể đề xuất tính năng trong Issues. Với thay đổi lớn, hãy trao đổi
về hành vi mong muốn trước khi triển khai.

## Mã nguồn

| Thư mục | Nội dung |
| --- | --- |
| `Sources/VIMEK/engine` | Lõi xử lý tiếng Việt dùng chung |
| `Sources/VIMEK/windows/App` | Hook bàn phím, tray và giao diện Windows |
| `Sources/VIMEK/macOS/App` | Event tap, menu bar và giao diện macOS |
| `Sources/VIMEK/macOS/VIMEKHelper` | Helper khởi động trên macOS |
| `Sources/VIMEK/linux` | Engine IBus, giao diện GTK và cấu hình Linux |
| `tests` | Kiểm thử lõi gõ, phím tắt, giao diện Windows và tích hợp Linux |
| `assets/brand` | Vector biểu tượng V/E |

Hướng dẫn build và chạy kiểm thử nằm trong [README](README.md).
Thay đổi quy tắc gõ cần có ca kiểm thử cho chuỗi phím liên quan.
Thay đổi hook hoặc giao diện cần được thử trên hệ điều hành tương ứng.

## Phiên bản

VIMEK đánh số theo [SemVer](https://semver.org/lang/vi/): `MAJOR.MINOR.PATCH`.
[version.json](version.json) là nguồn duy nhất; không sửa số phiên bản bằng tay ở chỗ khác.

Trước 1.0 (giai đoạn alpha hiện nay):

- **MINOR** (`0.2.0` → `0.3.0`): có tính năng mới hoặc hành vi người dùng thấy được thay đổi
  (phím tắt, mặc định, cấu hình cũ cần chuyển đổi).
- **PATCH** (`0.2.0` → `0.2.1`): chỉ sửa lỗi, không thêm tính năng.
- **MAJOR** lên `1.0.0` khi VIMEK đủ ổn định để dùng hằng ngày: bản Windows có chữ ký số, bản
  macOS được notarize, không còn thay đổi lớn về cách gõ.
- Hậu tố `-rc.N` (`0.3.0-rc.1`) chỉ dùng khi cần cho người khác thử trước một bản MINOR, và
  chỉ vài bản. Không dùng chuỗi `-alpha.N` kéo dài; bản có hậu tố được đánh dấu pre-release.

Quy trình phát hành:

1. Trong lúc làm, ghi thay đổi vào mục `## Chưa phát hành` ở đầu [CHANGELOG](CHANGELOG.md)
   và cập nhật [release notes](.github/release-notes.md) nếu cần.
2. Chọn số theo quy tắc trên rồi chạy `node scripts/set-version.mjs X.Y.Z`. Script đặt số ở
   mọi nơi (CMake, tài nguyên Windows, Xcode, script đóng gói, tài liệu) và đổi
   `## Chưa phát hành` thành `## X.Y.Z (ngày)`.
3. Commit `Release X.Y.Z`, tạo tag `vX.Y.Z`, push cả commit và tag. CI chạy
   `set-version.mjs --check`: tag khác `version.json` hoặc một file lệch số thì không phát hành.

## Pull request

Giữ mỗi pull request tập trung vào một thay đổi. Mô tả vấn đề, hành vi mới
và cách bạn đã kiểm tra. Nếu thay đổi giao diện, kèm ảnh minh họa.

Đóng góp được phát hành theo [GNU GPL v3](LICENSE).
Giữ thông báo bản quyền và giấy phép của các thành phần bên thứ ba.
