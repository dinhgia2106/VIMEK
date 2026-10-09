# Changelog

## 0.1.0-alpha.7

- Sửa đồng bộ chế độ Việt/Anh và biểu tượng tray trên Windows; thử lại khi Shell chưa sẵn sàng và khôi phục từ trạng thái hiện tại khi Explorer khởi động lại.
- Đưa cập nhật tray ra khỏi callback bàn phím để phím chuyển không phải chờ Explorer.
- Sửa nhớ chế độ theo ứng dụng lấy nhầm bit bảng mã; giữ chế độ khi taskbar hoặc cửa sổ VIMEK nhận focus.

## 0.1.0-alpha.6

- Thêm bản Linux với engine IBus, Unicode, Telex, VNI và hai biến thể Simple Telex.
- Giao diện cài đặt GTK theo sáng/tối hệ thống, biểu tượng V/E viền mảnh, lựa chọn Ctrl+Alt hoặc Ctrl+Shift và âm thanh khi chuyển chế độ.
- Giữ riêng trạng thái ô nhập liệu, xử lý preedit và Backspace, giữ chữ khi đổi focus và bỏ qua trường mật khẩu.
- Thêm gói `.deb` amd64, hướng dẫn Linux, kiểm thử IBus/GTK và nút tải trên website.

## 0.1.0-alpha.5

- Thu gọn bảng điều khiển trên Windows và macOS: bỏ tên ngôn ngữ lặp lại ở đầu cửa sổ, đưa biểu tượng V/E viền mảnh sang bên phải, cùng hàng với VIMEK.

## 0.1.0-alpha.4

- Bấm nút phím chuyển trên Windows để đổi giữa Ctrl+Alt và Ctrl+Shift, áp dụng ngay và giữ lựa chọn âm thanh.
- Thay hai tùy chọn chính tả và nhớ chế độ theo ứng dụng trong bảng điều khiển bằng Chạy với quyền Admin và Khởi động cùng Windows. Hai tùy chọn cũ vẫn có trong Nâng cao.
- Dùng chung cấu hình hệ thống giữa bảng điều khiển và Nâng cao; hủy UAC không đóng VIMEK.
- Sửa khởi động cùng Windows với đường dẫn chứa dấu cách hoặc ký tự tiếng Việt.

## 0.1.0-alpha.3

- Sửa tab Thông tin bị cắt nội dung và nút Giấy phép & nguồn gốc do dùng đơn vị kích thước khác với cửa sổ Nâng cao.
- Hiển thị đầy đủ tên nút và giữ các tùy chọn trong tab nằm trong vùng hiển thị.

## 0.1.0-alpha.2

- Sửa vùng Điều khiển trong Nâng cao bị trắng và che các tùy chọn kiểu gõ, bảng mã, phím chuyển, âm thanh và chế độ Việt/Anh.
- Sửa khung nhóm tùy chọn trong Chuyển mã, kể cả sau khi cửa sổ vẽ lại hoặc đổi giao diện sáng/tối.
- README tiếng Anh và tiếng Việt; landing page có nút tải cho Windows và macOS.
- Sửa hiển thị dấu tiếng Việt trên website bằng font đi kèm.

## 0.1.0 — Alpha

- Telex, VNI và hai biến thể Simple Telex.
- Bảng điều khiển tự theo chế độ sáng/tối của hệ điều hành.
- Cửa sổ Nâng cao, Gõ tắt, Chuyển mã và Giới thiệu dùng cùng chế độ sáng/tối.
- Sửa lỗi các tùy chọn trong Nâng cao bị mất sau khi chuyển tab hoặc vẽ lại cửa sổ.
- Biểu tượng V/E trong suốt, hỗ trợ DPI cao và Retina.
- Phím chuyển Ctrl+Alt trên Windows, Control+Option trên macOS.
- Giữ một phím và bấm phím còn lại để chuyển Việt/Anh nhiều lần.
- Âm thanh hệ thống khi chuyển thủ công, với tùy chọn tắt âm thanh.
- Kiểm tra chính tả, gõ tắt, nhớ chế độ theo ứng dụng và chuyển mã văn bản.
