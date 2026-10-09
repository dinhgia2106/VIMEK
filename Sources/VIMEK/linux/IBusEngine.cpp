// Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
#include <ibus.h>
#include <canberra.h>
#include <cstring>
#include <exception>
#include "Composer.h"
#include "ModifierShortcut.h"

typedef struct {
    IBusEngine parent;
    vimek::Composer* composer;
    VimekModifierShortcut* chord;
    GSettings* settings;
    IBusPropList* properties;
    ca_context* sound;
    gboolean privateField;
    gboolean focused;
} VimekIBusEngine;
typedef struct {IBusEngineClass parent;} VimekIBusEngineClass;
G_DEFINE_TYPE(VimekIBusEngine,vimek_ibus_engine,IBUS_TYPE_ENGINE)

static void preedit(VimekIBusEngine* self) {
    IBusText* text=ibus_text_new_from_string(self->composer->text().c_str());
    guint length=self->composer->length();
    ibus_text_append_attribute(text,IBUS_ATTR_TYPE_UNDERLINE,IBUS_ATTR_UNDERLINE_SINGLE,0,length);
    ibus_engine_update_preedit_text_with_mode(IBUS_ENGINE(self),text,length,length>0,IBUS_ENGINE_PREEDIT_COMMIT);
}
static void commit(VimekIBusEngine* self,const std::string& text) {
    if(!text.empty())ibus_engine_commit_text(IBUS_ENGINE(self),ibus_text_new_from_string(text.c_str()));
}
static void finish(VimekIBusEngine* self) {
    auto text=self->composer->finish();preedit(self);commit(self,text);
}
static vimek::Options options(GSettings* settings) {
    return {g_settings_get_int(settings,"method"),bool(g_settings_get_boolean(settings,"spelling")),
        bool(g_settings_get_boolean(settings,"restore")),bool(g_settings_get_boolean(settings,"modern"))};
}
static void add(IBusPropList* list,const char* key,const char* label,IBusPropType type,
                bool selected=false,IBusPropList* children=nullptr,const char* icon="") {
    auto* property=ibus_property_new(key,type,ibus_text_new_from_string(label),icon,
        ibus_text_new_from_string(label),TRUE,TRUE,selected?PROP_STATE_CHECKED:PROP_STATE_UNCHECKED,children);
    ibus_prop_list_append(list,property);
}
static void refreshProperties(VimekIBusEngine* self) {
    g_clear_object(&self->properties);
    self->properties=ibus_prop_list_new();g_object_ref_sink(self->properties);
    bool vietnamese=g_settings_get_boolean(self->settings,"vietnamese");
    add(self->properties,"mode",vietnamese?"V":"E",PROP_TYPE_TOGGLE,vietnamese,nullptr,
        vietnamese?"vimek-v-symbolic":"vimek-e-symbolic");
    auto* methods=ibus_prop_list_new();
    const char* names[]={"Telex","VNI","Simple Telex 1","Simple Telex 2"};
    int method=g_settings_get_int(self->settings,"method");
    for(int i=0;i<4;++i) {
        std::string key="method."+std::to_string(i);add(methods,key.c_str(),names[i],PROP_TYPE_RADIO,i==method);
    }
    add(self->properties,"method",names[method],PROP_TYPE_MENU,false,methods);
    auto* shortcuts=ibus_prop_list_new();
    gchar* shortcut=g_settings_get_string(self->settings,"shortcut");
    bool useAlt=strcmp(shortcut,"ctrl-alt")==0;g_free(shortcut);
    add(shortcuts,"ctrl-alt","Ctrl + Alt",PROP_TYPE_RADIO,useAlt);
    add(shortcuts,"ctrl-shift","Ctrl + Shift",PROP_TYPE_RADIO,!useAlt);
    add(self->properties,"shortcut","Phím chuyển Việt / Anh",PROP_TYPE_MENU,false,shortcuts);
    add(self->properties,"spelling","Kiểm tra chính tả",PROP_TYPE_TOGGLE,g_settings_get_boolean(self->settings,"spelling"));
    add(self->properties,"sound","Âm thanh khi chuyển",PROP_TYPE_TOGGLE,g_settings_get_boolean(self->settings,"sound"));
    add(self->properties,"settings","Cài đặt VIMEK",PROP_TYPE_NORMAL);
    ibus_engine_register_properties(IBUS_ENGINE(self),self->properties);
}
static void changed(GSettings*,gchar* key,gpointer data) {
    auto* self=static_cast<VimekIBusEngine*>(data);
    if(strcmp(key,"sound")!=0) {
        finish(self);self->composer->configure(options(self->settings));
    }
    if(strcmp(key,"shortcut")==0)self->chord->reset();
    if(strcmp(key,"vietnamese")==0&&self->focused&&self->sound&&g_settings_get_boolean(self->settings,"sound"))
        ca_context_play(self->sound,0,CA_PROP_EVENT_ID,g_settings_get_boolean(self->settings,"vietnamese")?"button-toggle-on":"button-toggle-off",
            CA_PROP_APPLICATION_NAME,"VIMEK",nullptr);
    if(self->focused)refreshProperties(self);
}
static void toggle(VimekIBusEngine* self) {
    bool vietnamese=!g_settings_get_boolean(self->settings,"vietnamese");
    g_settings_set_boolean(self->settings,"vietnamese",vietnamese);
}
static unsigned modifier(guint key) {
    switch(key) {
    case IBUS_KEY_Control_L:case IBUS_KEY_Control_R:return 1;
    case IBUS_KEY_Alt_L:case IBUS_KEY_Alt_R:return 2;
    case IBUS_KEY_Super_L:case IBUS_KEY_Super_R:case IBUS_KEY_Meta_L:case IBUS_KEY_Meta_R:return 4;
    case IBUS_KEY_Shift_L:case IBUS_KEY_Shift_R:return 8;
    default:return 0;
    }
}
static gboolean process(IBusEngine* engine,guint key,guint,guint state) {
    auto* self=reinterpret_cast<VimekIBusEngine*>(engine);
    if(self->privateField)return FALSE;
    bool release=state&IBUS_RELEASE_MASK;
    unsigned current=((state&IBUS_CONTROL_MASK)?1:0)|((state&IBUS_MOD1_MASK)?2:0)|
        ((state&(IBUS_SUPER_MASK|IBUS_META_MASK|IBUS_MOD4_MASK))?4:0)|((state&IBUS_SHIFT_MASK)?8:0);
    unsigned bit=modifier(key);
    if(bit)current=release?(current&~bit):(current|bit);
    gchar* shortcut=g_settings_get_string(self->settings,"shortcut");
    unsigned required=strcmp(shortcut,"ctrl-shift")==0?9:3;g_free(shortcut);
    bool altGr=(state&IBUS_MOD5_MASK)||key==IBUS_KEY_Alt_R||key==IBUS_KEY_ISO_Level3_Shift;
    if(self->chord->update(current,required,!bit&&!release,altGr)) {toggle(self);return TRUE;}
    if(release||bit||key==IBUS_KEY_ISO_Level3_Shift)return FALSE;
    if(current&7||altGr) {finish(self);return FALSE;}
    if(!g_settings_get_boolean(self->settings,"vietnamese"))return FALSE;
    if(key==IBUS_KEY_Escape) {
        if(self->composer->empty())return FALSE;
        self->composer->clear();preedit(self);return TRUE;
    }
    if(key==IBUS_KEY_BackSpace) {
        if(self->composer->empty())return FALSE;
        self->composer->press('\b');preedit(self);return TRUE;
    }
    gunichar character=ibus_keyval_to_unicode(key);
    if(character<32||character>126) {finish(self);return FALSE;}
    bool word=(character>='a'&&character<='z')||(character>='A'&&character<='Z')||
        (character>='0'&&character<='9')||character=='['||character==']';
    if(!word) {
        if(self->composer->empty())return FALSE;
        std::string text=self->composer->finish()+char(character);preedit(self);commit(self,text);return TRUE;
    }
    try {
        if(self->composer->full())finish(self);
        self->composer->press(char(character));preedit(self);return TRUE;
    } catch(const std::exception& error) {
        g_warning("VIMEK composition failed: %s",error.what());
        // Preserve existing text and let the application handle the new key.
        auto text=self->composer->text();self->composer->clear();preedit(self);commit(self,text);return FALSE;
    }
}
static void activate(IBusEngine* engine,const gchar* key,guint) {
    auto* self=reinterpret_cast<VimekIBusEngine*>(engine);
    if(strcmp(key,"mode")==0)toggle(self);
    else if(g_str_has_prefix(key,"method.")&&strlen(key)==8&&key[7]>='0'&&key[7]<='3')
        g_settings_set_int(self->settings,"method",key[7]-'0');
    else if(strcmp(key,"ctrl-alt")==0||strcmp(key,"ctrl-shift")==0)g_settings_set_string(self->settings,"shortcut",key);
    else if(strcmp(key,"spelling")==0||strcmp(key,"sound")==0)
        g_settings_set_boolean(self->settings,key,!g_settings_get_boolean(self->settings,key));
    else if(strcmp(key,"settings")==0) {
        gchar* argv[]={const_cast<gchar*>(VIMEK_SETUP_PATH),nullptr};
        GError* error=nullptr;
        if(!g_spawn_async(nullptr,argv,nullptr,G_SPAWN_DEFAULT,nullptr,nullptr,nullptr,&error)) {
            g_warning("Cannot open VIMEK settings: %s",error->message);g_error_free(error);
        }
    }
}
static void focusIn(IBusEngine* engine) {
    auto* self=reinterpret_cast<VimekIBusEngine*>(engine);
    self->focused=TRUE;
    self->chord->reset();self->composer->configure(options(self->settings));refreshProperties(self);
}
static void enable(IBusEngine* engine) {
    auto* self=reinterpret_cast<VimekIBusEngine*>(engine);
    self->composer->configure(options(self->settings));refreshProperties(self);
}
static void focusOut(IBusEngine* engine) {
    // IBus commits the visible preedit while it still owns the old context.
    // Sending a second commit here is too late or duplicates that text.
    auto* self=reinterpret_cast<VimekIBusEngine*>(engine);self->focused=FALSE;
    self->composer->clear();self->chord->reset();
}
static void reset(IBusEngine* engine) {
    auto* self=reinterpret_cast<VimekIBusEngine*>(engine);self->composer->clear();self->chord->reset();preedit(self);
}
static void contentType(IBusEngine* engine,guint purpose,guint) {
    auto* self=reinterpret_cast<VimekIBusEngine*>(engine);
    bool privateField=purpose==IBUS_INPUT_PURPOSE_PASSWORD||purpose==IBUS_INPUT_PURPOSE_PIN;
    if(privateField&&!self->privateField)reset(engine);
    self->privateField=privateField;
}
static void finalize(GObject* object) {
    auto* self=reinterpret_cast<VimekIBusEngine*>(object);
    if(self->sound)ca_context_destroy(self->sound);
    g_clear_object(&self->settings);g_clear_object(&self->properties);
    delete self->composer;delete self->chord;
    G_OBJECT_CLASS(vimek_ibus_engine_parent_class)->finalize(object);
}
static void vimek_ibus_engine_init(VimekIBusEngine* self) {
    self->settings=g_settings_new("org.vimek.settings");
    self->composer=new vimek::Composer(options(self->settings));self->chord=new VimekModifierShortcut;
    ca_context_create(&self->sound);
    g_signal_connect(self->settings,"changed",G_CALLBACK(changed),self);
}
static void vimek_ibus_engine_class_init(VimekIBusEngineClass* klass) {
    G_OBJECT_CLASS(klass)->finalize=finalize;
    auto* engine=IBUS_ENGINE_CLASS(klass);
    engine->process_key_event=process;engine->property_activate=activate;
    engine->focus_in=focusIn;engine->enable=enable;engine->focus_out=focusOut;engine->disable=focusOut;
    engine->reset=reset;engine->set_content_type=contentType;
}
static void disconnected(IBusBus*,gpointer) {ibus_quit();}
int main(int argc,char** argv) {
    if(argc>1&&strcmp(argv[1],"--version")==0) {g_print("VIMEK %s (IBus)\n",VIMEK_VERSION);return 0;}
    ibus_init();
    IBusBus* bus=ibus_bus_new();
    if(!ibus_bus_is_connected(bus)) {g_printerr("VIMEK: start IBus before activating the input method.\n");g_object_unref(bus);return 1;}
    g_signal_connect(bus,"disconnected",G_CALLBACK(disconnected),nullptr);
    IBusFactory* factory=ibus_factory_new(ibus_bus_get_connection(bus));
    ibus_factory_add_engine(factory,"vimek",vimek_ibus_engine_get_type());
    if(!ibus_bus_request_name(bus,"org.freedesktop.IBus.VIMEK",0)) {
        g_printerr("VIMEK: could not register the IBus service.\n");return 1;
    }
    ibus_main();g_object_unref(factory);g_object_unref(bus);return 0;
}
