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
| `tests` | Kiểm thử lõi gõ và phím tắt |
| `assets/brand` | Vector biểu tượng V/E |

Hướng dẫn build và chạy kiểm thử nằm trong [README](README.md).
Thay đổi quy tắc gõ cần có ca kiểm thử cho chuỗi phím liên quan.
Thay đổi hook hoặc giao diện cần được thử trên hệ điều hành tương ứng.

## Pull request

Giữ mỗi pull request tập trung vào một thay đổi. Mô tả vấn đề, hành vi mới
và cách bạn đã kiểm tra. Nếu thay đổi giao diện, kèm ảnh minh họa.

Đóng góp được phát hành theo [GNU GPL v3](LICENSE).
Giữ thông báo bản quyền và giấy phép của các thành phần bên thứ ba.
