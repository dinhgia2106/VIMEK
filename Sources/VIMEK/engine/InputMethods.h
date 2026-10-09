/* VIMEK additions, 2026. GPL-3.0, see LICENSE. */
#ifndef VIMEK_INPUT_METHODS_H
#define VIMEK_INPUT_METHODS_H

/* Keep IDs stable: preferences and the upstream engine store these numbers.
 * Shared by C/Objective-C UI code and the C++ engine. A new method needs
 * both a descriptor here and an implementation in Engine.cpp/DataType.h. */
enum { VIMEK_INPUT_METHOD_COUNT = 4 };

#define VIMEK_INPUT_METHOD_LIST(X) \
    X(0, "Telex") \
    X(1, "VNI") \
    X(2, "Simple Telex 1") \
    X(3, "Simple Telex 2")

static inline int vimekIsValidInputMethod(int id) {
    return id >= 0 && id < VIMEK_INPUT_METHOD_COUNT;
}

static inline int vimekNormalizeInputMethod(int id) {
    return vimekIsValidInputMethod(id) ? id : 0;
}

static inline const char* vimekInputMethodName(int id) {
    static const char* const names[VIMEK_INPUT_METHOD_COUNT] = {
#define VIMEK_METHOD_NAME(id, name) name,
        VIMEK_INPUT_METHOD_LIST(VIMEK_METHOD_NAME)
#undef VIMEK_METHOD_NAME
    };
    return names[vimekNormalizeInputMethod(id)];
}

#endif
