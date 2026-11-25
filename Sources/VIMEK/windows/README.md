# VIMEK cho Windows

Ứng dụng chính ở [App](App/). Mở [VIMEK.sln](VIMEK.sln) bằng Visual Studio
2022, hoặc dùng CMake/script portable từ thư mục gốc. Xem [hướng dẫn build](../../../README.md).

Ctrl+Alt chuyển Việt/Anh. Bảng chính tự theo sáng/tối Windows; biểu tượng
V/E viền trắng nằm ở tray. Cấu hình lưu riêng tại HKCU\Software\VIMEK.

Thông tin nguồn gốc và giấy phép ở [NOTICE](../../../NOTICE.md).

## Gợi ý dấu bằng AI (thử nghiệm)

Bật ở bảng điều khiển (dòng Gợi ý dấu bằng AI) hoặc trong tray: Tùy chọn > Gợi ý dấu
bằng AI (thử nghiệm). Chỉ chạy với Unicode dựng
sẵn và kiểu gõ Telex; VNI dùng phím số để bỏ dấu nên không hỗ trợ.

Gõ một từ không dấu, VIMEK hiện các bản có dấu của **chỉ từ đang gõ** (từ đầu câu hoặc
sau dấu cách gần nhất) ngay dưới con trỏ. Các từ trước không bao giờ bị sửa.

- Space: nhận gợi ý 1, rồi thêm dấu cách.
- 1-5: chọn gợi ý tương ứng.
- Esc: giữ nguyên chữ đã gõ; phím số sau đó gõ ra số.
- Từ đang gõ có dấu Telex (`tieengs`, `dd`, `aa`, ...) không được gợi ý và giữ nguyên.

Vị trí con trỏ lấy qua UI Automation (trình duyệt, Office, ứng dụng mới), rồi caret
Win32, MSAA; không tìm được thì dùng vị trí chuột.

Model chạy ở server cục bộ của
[vi-restore-llm](https://github.com/dinhgia2106/vi-restore-llm), cổng 8765 (đổi bằng
giá trị `vAiSuggestPort` trong HKCU\Software\VIMEK):

```
python -m virestore.serve --model tagger --tagger runs/ds-tagger-prefix --threads 4 --port 8765
```

Model nên dùng là tagger huấn luyện trên câu dở dang (`ds-tagger-prefix`): khi chỉ được
sửa từ đang gõ, model không thấy các từ phía sau, và tagger này đúng nhiều từ nhất ngay
lần đầu (xem `docs/streaming.md` của vi-restore-llm).

Khi server không chạy, VIMEK gõ Telex như bình thường. Kiểm thử tự động: `VIMEK.exe
--test-input` chỉ xử lý phím giả lập mang thẻ `TEST_INPUT_TAG` (xem `WindowsInput.h`),
bỏ qua phím thật, không xin quyền admin và không ghi cấu hình.
