#!/usr/bin/python3
# Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
import locale
import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, Gtk

VI = (locale.getlocale()[0] or '').startswith('vi')
def tr(english, vietnamese):
    return vietnamese if VI else english

class Panel(Gtk.ApplicationWindow):
    def __init__(self, application):
        super().__init__(application=application, title='VIMEK')
        self.set_default_size(640, 0)
        self.set_resizable(False)
        self.settings = Gio.Settings.new('org.vimek.settings')
        self.refreshing = False
        body = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=18)
        for edge in ('top', 'bottom', 'start', 'end'):
            getattr(body, 'set_margin_' + edge)(28)
        self.add(body)
        header = Gtk.Box()
        brand = Gtk.Label(label='V I M E K', xalign=0)
        header.pack_start(brand, True, True, 0)
        self.icon = Gtk.Image(pixel_size=40)
        header.pack_end(self.icon, False, False, 0)
        body.pack_start(header, False, False, 0)
        self.languages = []
        language = Gtk.Box(spacing=12, homogeneous=True)
        for vietnamese, title in ((True, 'Tiếng Việt'), (False, 'English')):
            button = Gtk.ToggleButton(label=title)
            button.connect('clicked', self.select_language, vietnamese)
            language.pack_start(button, True, True, 0)
            self.languages.append(button)
        body.pack_start(language, False, False, 0)
        body.pack_start(Gtk.Label(label=tr('INPUT METHOD', 'KIỂU GÕ'), xalign=0), False, False, 0)
        self.methods = []
        methods = Gtk.Box(spacing=10, homogeneous=True)
        for index, title in enumerate(('Telex', 'VNI', 'Simple Telex 1', 'Simple Telex 2')):
            button = Gtk.ToggleButton(label=title)
            button.connect('clicked', self.select_method, index)
            methods.pack_start(button, True, True, 0)
            self.methods.append(button)
        body.pack_start(methods, False, False, 0)
        self.example = Gtk.Label(xalign=0)
        body.pack_start(self.example, False, False, 0)
        body.pack_start(Gtk.Separator(), False, False, 0)
        self.shortcut = self.row(body, tr('Vietnamese / English shortcut', 'Phím chuyển Việt / Anh'))
        self.shortcut.connect('clicked', self.cycle_shortcut)
        self.spelling = self.row(body, tr('Spell checking', 'Kiểm tra chính tả'))
        self.spelling.connect('clicked', self.toggle, 'spelling')
        self.sound = self.row(body, tr('Sound when switching', 'Âm thanh khi chuyển'))
        self.sound.connect('clicked', self.toggle, 'sound')
        body.pack_start(Gtk.Entry(placeholder_text=tr('Try typing here', 'Thử gõ ở đây')), False, False, 0)
        advanced = Gtk.Expander(label=tr('Advanced', 'Nâng cao'))
        extra = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        extra.set_margin_top(12)
        for key, title in (
            ('restore', tr('Restore words with invalid spelling', 'Khôi phục từ sai chính tả')),
            ('modern', tr('Modern tone placement (oà, uý)', 'Đặt dấu oà, uý (thay vì òa, úy)')),
        ):
            checkbox = Gtk.CheckButton(label=title)
            self.settings.bind(key, checkbox, 'active', Gio.SettingsBindFlags.DEFAULT)
            extra.pack_start(checkbox, False, False, 0)
        advanced.add(extra)
        body.pack_start(advanced, False, False, 0)
        body.pack_start(Gtk.Separator(), False, False, 0)
        body.pack_start(Gtk.Label(label='Unicode', xalign=0), False, False, 0)
        self.settings.connect('changed', self.refresh)
        self.refresh()

    def row(self, body, label):
        row = Gtk.Box(spacing=20)
        row.pack_start(Gtk.Label(label=label, xalign=0), True, True, 0)
        button = Gtk.Button()
        button.set_size_request(150, 36)
        row.pack_end(button, False, False, 0)
        body.pack_start(row, False, False, 0)
        return button

    def select_language(self, button, vietnamese):
        if not self.refreshing:
            self.settings.set_boolean('vietnamese', vietnamese)
            self.refresh()

    def select_method(self, button, index):
        if not self.refreshing:
            self.settings.set_int('method', index)
            self.refresh()

    def toggle(self, button, key):
        self.settings.set_boolean(key, not self.settings.get_boolean(key))

    def cycle_shortcut(self, button):
        selected = self.settings.get_string('shortcut')
        self.settings.set_string('shortcut', 'ctrl-shift' if selected == 'ctrl-alt' else 'ctrl-alt')

    def refresh(self, *unused):
        self.refreshing = True
        vietnamese = self.settings.get_boolean('vietnamese')
        self.icon.set_from_icon_name('vimek-v-symbolic' if vietnamese else 'vimek-e-symbolic', Gtk.IconSize.DIALOG)
        self.icon.set_pixel_size(40)
        for index, button in enumerate(self.languages):
            button.set_active(vietnamese == (index == 0))
        method = self.settings.get_int('method')
        for index, button in enumerate(self.methods):
            button.set_active(index == method)
        keys = 'tie6ng1 Vie6t5' if method == 1 else 'tieengs Vieetj'
        self.example.set_text(tr('Example: ', 'Ví dụ: ') + keys + ' → tiếng Việt')
        self.shortcut.set_label('Ctrl + Alt' if self.settings.get_string('shortcut') == 'ctrl-alt' else 'Ctrl + Shift')
        for button, key in ((self.spelling, 'spelling'), (self.sound, 'sound')):
            button.set_label(tr('On', 'Bật') if self.settings.get_boolean(key) else tr('Off', 'Tắt'))
        self.refreshing = False

class App(Gtk.Application):
    def __init__(self):
        super().__init__(application_id='org.vimek.Settings')
        self.panel = None

    def do_activate(self):
        if self.panel is None:
            self.panel = Panel(self)
            self.panel.connect('destroy', self.closed)
        self.panel.show_all()
        self.panel.present()

    def closed(self, *unused):
        self.panel = None

if __name__ == '__main__':
    App().run()
