#!/usr/bin/python3
# Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
"""Exercise the installed engine over a real isolated IBus/D-Bus connection."""
import subprocess
import time
import gi
gi.require_version('IBus', '1.0')
from gi.repository import Gio, GLib, IBus

def pump(seconds=0.05):
    deadline = time.monotonic() + seconds
    context = GLib.MainContext.default()
    while time.monotonic() < deadline:
        while context.pending():
            context.iteration(False)
        time.sleep(0.002)

def wait(predicate, label, timeout=8):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        pump()
        if predicate():
            return
    raise AssertionError(label)

IBus.init()
subprocess.run(['ibus-daemon', '--daemonize', '--replace', '--xim', '--panel=disable',
                '--config=disable', '--emoji-extension=disable', '--cache=refresh'], check=True)
bus = IBus.Bus.new()
wait(bus.is_connected, 'IBus daemon did not start')
settings = Gio.Settings.new('org.vimek.settings')
settings.set_boolean('sound', False)
settings.set_boolean('vietnamese', True)
settings.set_int('method', 0)
settings.set_string('shortcut', 'ctrl-alt')
Gio.Settings.sync()

context = bus.create_input_context('VIMEK integration tests')
context.set_capabilities(IBus.Capabilite.PREEDIT_TEXT | IBus.Capabilite.FOCUS | IBus.Capabilite.PROPERTY)
committed = []
preedit = ['']
properties = {}
def register_properties(ctx, props):
    index = 0
    while props.get(index) is not None:
        prop = props.get(index)
        properties[prop.get_key()] = prop
        index += 1
context.connect('register-properties', register_properties)
context.connect('commit-text', lambda ctx, text: committed.append(text.get_text()))
context.connect('update-preedit-text', lambda ctx, text, cursor, visible: preedit.__setitem__(0, text.get_text() if visible else ''))
context.focus_in()
context.set_engine('vimek')
wait(lambda: context.get_engine() and context.get_engine().get_name() == 'vimek', 'Installed VIMEK engine cannot activate')
wait(lambda: 'mode' in properties, 'IBus properties were not registered')

def key(value, state=0):
    handled = context.process_key_event(value, 0, state)
    pump()
    return handled

def type_word(value):
    for character in value:
        handled = key(ord(character), IBus.ModifierType.SHIFT_MASK if character.isupper() else 0)
        if not handled:
            committed.append(character)

def clear():
    context.reset()
    pump()
    committed.clear()
    preedit[0] = ''

for method, keys in ((0, 'tieengs Vieetj '), (1, 'tie6ng1 Vie6t5 '), (2, 'tieengs Vieetj '), (3, 'tieengs Vieetj ')):
    context.property_activate('method.' + str(method), IBus.PropState.CHECKED)
    wait(lambda: settings.get_int('method') == method, 'IBus input method menu did not save its choice')
    clear()
    type_word(keys)
    assert ''.join(committed) == 'tiếng Việt ', (method, committed)
    assert not preedit[0], 'Committed word still appears in preedit'
print('Four input methods commit Unicode through IBus.')

settings.set_int('method', 0)
pump(0.2)
clear()
type_word('as')
assert preedit[0] == 'á'
assert key(IBus.KEY_BackSpace)
assert not preedit[0], 'Backspace must delete the accented character'
type_word('f ')
assert ''.join(committed) == 'f '
clear()
type_word('tieengs')
assert key(IBus.KEY_Escape)
assert not preedit[0] and not committed, 'Escape should cancel composition'
print('Preedit, Backspace and Escape work without injecting app backspaces.')

control = IBus.ModifierType.CONTROL_MASK
alt = IBus.ModifierType.MOD1_MASK
shift = IBus.ModifierType.SHIFT_MASK
release = IBus.ModifierType.RELEASE_MASK
key(IBus.KEY_Control_L)
for expected in (False, True, False, True):
    key(IBus.KEY_Alt_L, control)
    assert key(IBus.KEY_Alt_L, control | alt | release)
    wait(lambda: settings.get_boolean('vietnamese') == expected, 'Ctrl+Alt did not toggle mode')
    wait(lambda: properties['mode'].get_label().get_text() == ('V' if expected else 'E'), 'IBus mode label did not refresh')
key(IBus.KEY_Control_L, control | release)
context.property_activate('ctrl-shift', IBus.PropState.CHECKED)
wait(lambda: settings.get_string('shortcut') == 'ctrl-shift', 'Shortcut menu did not save its choice')
key(IBus.KEY_Control_L)
for expected in (False, True):
    key(IBus.KEY_Shift_L, control)
    assert key(IBus.KEY_Shift_L, control | shift | release)
    wait(lambda: settings.get_boolean('vietnamese') == expected, 'Ctrl+Shift did not toggle mode')
    wait(lambda: properties['mode'].get_label().get_text() == ('V' if expected else 'E'), 'IBus mode label did not refresh')
key(IBus.KEY_Control_L, control | release)
assert not key(ord('c'), control), 'Ctrl+C must reach the application'
assert not key(ord('a'), IBus.ModifierType.MOD5_MASK), 'AltGr must reach the application'
print('Both shortcuts support holding Ctrl and tapping the other key repeatedly; app shortcuts and AltGr pass through.')

clear()
type_word('tieengs')
context.focus_out()
wait(lambda: ''.join(committed) == 'tiếng', 'Focus out lost uncommitted text')
context.focus_in()
pump()
type_word('as ')
assert ''.join(committed) == 'tiếngá ', 'Focus change committed twice or leaked a word'
clear()
context.set_content_type(IBus.InputPurpose.PASSWORD, IBus.InputHints.NONE)
pump()
assert not key(ord('a')) and not key(ord('s')), 'Password fields must bypass composition'
assert not preedit[0] and not committed
context.set_content_type(IBus.InputPurpose.FREE_FORM, IBus.InputHints.NONE)
pump()
type_word('as ')
assert ''.join(committed) == 'á '
print('Focus changes preserve text exactly once; password fields bypass the engine.')

context.destroy()
# Validate a toolkit client too: physical X11 key events travel through the
# installed GTK IBus module, not the direct input-context test helpers.
gi.require_version('Gtk', '3.0')
gi.require_version('GdkX11', '3.0')
from gi.repository import Gtk, GdkX11
window = Gtk.Window(title='VIMEK GTK integration')
box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
entry = Gtk.Entry()
password = Gtk.Entry(visibility=False, input_purpose=Gtk.InputPurpose.PASSWORD)
box.add(entry)
box.add(password)
window.add(box)
window.show_all()
entry.grab_focus()
pump(0.3)
subprocess.run(['xdotool', 'windowfocus', '--sync', str(window.get_window().get_xid())], check=True)
assert bus.set_global_engine('vimek')
pump(0.3)
def physical_type(text):
    process = subprocess.Popen(['xdotool', 'type', '--clearmodifiers', '--delay', '15', text])
    wait(lambda: process.poll() is not None, 'Physical typing did not complete')
    assert process.returncode == 0
    pump(0.3)
physical_type('tieengs Vieetj ')
assert entry.get_text() == 'tiếng Việt ', entry.get_text()
password.grab_focus()
pump(0.3)
physical_type('as')
assert password.get_text() == 'as', 'GTK password entry was transformed'
window.destroy()
pump()
print('Real GTK entries receive Vietnamese; password entries remain literal.')
bus.exit(False)
print('Installed Linux IBus integration tests passed.')
