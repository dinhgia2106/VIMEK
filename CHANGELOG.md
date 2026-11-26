# Changelog

## 0.2.0 (2026-11-26)

- Windows x64: kèm sẵn server gợi ý dấu bằng AI (thư mục `suggest`). VIMEK tự chạy server khi bật Gợi ý dấu bằng AI và tắt khi tắt gợi ý hoặc thoát; không cần cài Python hay tự chạy vi-restore-llm. Model chạy trên máy bằng onnxruntime. Nếu cổng 8765 đã có server, VIMEK dùng server đó. Bản x86 chưa có server kèm theo.
- Đánh số phiên bản theo SemVer thay cho chuỗi `0.1.0-alpha.N`: thêm tính năng tăng số giữa, chỉ sửa lỗi tăng số cuối (xem CONTRIBUTING).

## 0.1.0-alpha.13

- Space không còn tự nhận gợi ý số 1: chỉ gõ dấu cách; chọn gợi ý bằng phím 1-5.
- Gợi ý dấu bằng AI nhận cả cụm gõ liền không dấu cách (`homnaytroidepqua` -> `hôm nay trời đẹp quá`): server tách từ rồi thêm dấu, phím số thay cả cụm. VIMEK gửi kèm các phím đã bấm vì engine Telex có thể hiểu nhầm chữ trong cụm gõ liền là dấu.

## 0.1.0-alpha.12

- Gợi ý dấu bằng AI chỉ áp dụng cho từ đang gõ (từ đầu câu hoặc sau dấu cách); các từ trước không bị sửa, và từ đã gõ dấu bằng Telex không được gợi ý.
- Ô gợi ý tìm vị trí con trỏ qua UI Automation trước (trình duyệt, Office), việc tìm chạy ở luồng nền.
- Khuyến nghị model `ds-tagger-prefix` cho chế độ này.

## 0.1.0-alpha.11

- Windows: thêm nút bật/tắt Gợi ý dấu bằng AI (thử nghiệm) ngay trên bảng điều khiển, dưới Phím chuyển Việt/Anh; vẫn có trong menu tray.
- Website: ảnh bảng điều khiển mới và giới thiệu gợi ý dấu bằng AI.

## 0.1.0-alpha.10

- Windows: thêm gợi ý dấu bằng AI (thử nghiệm, tắt mặc định), chạy song song với Telex: Space nhận gợi ý 1, phím 1-5 chọn gợi ý, Esc giữ chữ đã gõ. Model chạy ở server cục bộ của vi-restore-llm.

## 0.1.0-alpha.9

- Tăng chiều cao chữ V/E ở tray Windows khoảng 42%, giữ viền trắng và nền trong suốt.
- Mặc định dùng một chế độ Việt/Anh chung cho các ứng dụng trên Windows; vẫn có thể bật nhớ chế độ riêng trong Nâng cao.
- Thêm chế độ chẩn đoán trạng thái gõ và cập nhật tray, không ghi nội dung phím gõ.

## 0.1.0-alpha.8

- Giới hạn thời gian chờ IME trên Windows; xử lý phím chuyển Việt/Anh trước khi hỏi trạng thái IME của ứng dụng.
- Đưa cập nhật cửa sổ và lưu chế độ ra khỏi callback bàn phím; gộp các yêu cầu cập nhật tray liên tiếp và gửi lại trạng thái sau khi đổi chế độ.
- Không áp dụng mẹo sửa gợi ý trình duyệt cho cửa sổ Unity, gồm Goose Goose Duck.
- Xử lý phím thật có thông tin bổ sung từ driver bàn phím; bỏ qua các sự kiện phím được phần mềm gửi vào.

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
