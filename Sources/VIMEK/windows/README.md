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

Gõ không dấu (hoặc gõ Telex như thường), VIMEK hiện bản có dấu của tối đa 3 từ cuối
gần con trỏ:

- Space: nhận gợi ý 1, rồi thêm dấu cách. Hai từ trước vẫn có thể được sửa lại khi
  gõ từ tiếp theo.
- 1-5: chọn gợi ý tương ứng; phần đã chọn được giữ cố định.
- Esc: giữ nguyên chữ đã gõ; phím số sau đó gõ ra số.
- Từ đã gõ dấu bằng Telex được giữ nguyên.

Model chạy ở server cục bộ của
[vi-restore-llm](https://github.com/dinhgia2106/vi-restore-llm), cổng 8765 (đổi bằng
giá trị `vAiSuggestPort` trong HKCU\Software\VIMEK):

```
python -m virestore.serve --model tagger-onnx --tagger runs/ds-tagger-large --onnx-file tagger.int8.onnx --threads 4 --port 8765
```

Khi server không chạy, VIMEK gõ Telex như bình thường. Kiểm thử tự động: `VIMEK.exe
--test-input` chỉ xử lý phím giả lập mang thẻ `TEST_INPUT_TAG` (xem `WindowsInput.h`),
bỏ qua phím thật, không xin quyền admin và không ghi cấu hình.
