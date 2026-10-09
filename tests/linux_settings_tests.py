#!/usr/bin/python3
# Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
import importlib.util
import os
from pathlib import Path
import gi
import cairo
gi.require_version('Gtk', '3.0')
gi.require_foreign('cairo')
from gi.repository import Gio, Gtk, GLib

# Require isolated preferences, including when a contributor runs this test.
assert os.environ.get('GSETTINGS_BACKEND') == 'memory'
spec = importlib.util.spec_from_file_location('vimek_settings', '/usr/libexec/vimek/vimek-settings')
if spec is None:
    from importlib.machinery import SourceFileLoader
    spec = importlib.util.spec_from_loader('vimek_settings', SourceFileLoader('vimek_settings', '/usr/libexec/vimek/vimek-settings'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
app = module.App()
assert app.register(None)
app.activate()
panel = app.panel
def pump():
    context = GLib.MainContext.default()
    for unused in range(30):
        while context.pending():
            context.iteration(False)
        GLib.usleep(10000)
pump()
settings = panel.settings
theme = Gtk.IconTheme.get_default()
for name in ('vimek-v-symbolic', 'vimek-e-symbolic'):
    info = theme.lookup_icon(name, 40, Gtk.IconLookupFlags.FORCE_SIZE)
    assert info is not None and info.load_icon() is not None, 'V/E SVG icon failed to load'
panel.methods[1].clicked()
assert settings.get_int('method') == 1 and panel.methods[1].get_active()
assert 'tie6ng1' in panel.example.get_text()
panel.shortcut.clicked()
assert settings.get_string('shortcut') == 'ctrl-shift'
panel.shortcut.clicked()
assert settings.get_string('shortcut') == 'ctrl-alt'
panel.spelling.clicked()
panel.sound.clicked()
assert not settings.get_boolean('spelling') and not settings.get_boolean('sound')
panel.languages[1].clicked()
assert not settings.get_boolean('vietnamese')
assert panel.icon.get_icon_name()[0] == 'vimek-e-symbolic'
settings.set_int('method', 3)
settings.set_boolean('vietnamese', True)
assert panel.methods[3].get_active() and panel.languages[0].get_active()
print('Installed settings panel: language, method, both shortcuts, spelling, sound and external preference refresh passed.')

settings.set_int('method', 0)
settings.set_boolean('spelling', True)
settings.set_boolean('sound', True)
output = Path('build/ui')
output.mkdir(parents=True, exist_ok=True)
Gtk.Settings.get_default().set_property('gtk-theme-name', 'Adwaita')
for dark in (False, True):
    Gtk.Settings.get_default().set_property('gtk-application-prefer-dark-theme', dark)
    for vietnamese in (True, False):
        settings.set_boolean('vietnamese', vietnamese)
        pump()
        width, height = panel.get_size()
        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, width, height)
        panel.draw(cairo.Context(surface))
        surface.write_to_png(str(output / ('linux-%s-%s.png' % ('dark' if dark else 'light', 'v' if vietnamese else 'e'))))
panel.destroy()
print('Linux GTK light/dark previews rendered for V and E.')
