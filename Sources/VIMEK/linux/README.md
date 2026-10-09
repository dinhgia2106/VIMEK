# VIMEK trên Linux

VIMEK trên Linux dùng IBus và lõi gõ dùng chung trong [engine](../engine/).

- `IBusEngine.cpp`: kết nối IBus, phím chuyển, preedit, menu và âm thanh.
- `Composer.cpp`: xử lý Unicode và giữ riêng trạng thái từng ô nhập liệu.
- `settings.py`: cửa sổ cài đặt GTK, theo giao diện hệ thống.
- `org.vimek.settings.gschema.xml`: cấu hình dùng chung cho cửa sổ và engine.

Xem [hướng dẫn Linux](../../../Linux_Build.md) để build và cài đặt.
