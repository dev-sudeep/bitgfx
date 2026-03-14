/*
 * bitgfx.h - Single-file, header-only 2D graphics library
 *
 * Supports Linux (X11) and macOS (ObjC runtime + CoreGraphics).
 * No .m files, no [] syntax on macOS. Pure C throughout.
 *
 * USAGE:
 *   In exactly ONE .c file, before including this header:
 *     #define BITGFX_IMPLEMENTATION
 *   Then in all files:
 *     #include "bitgfx.h"
 *
 * LICENSE: MIT
 */

#ifndef BITGFX_H
#define BITGFX_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* =========================================================================
 * PUBLIC API DECLARATIONS
 * ========================================================================= */

/* -- Lifecycle -- */
int  bg_init(int width, int height, const char *title);
void bg_terminate(void);

/* -- Event loop -- */
void bg_poll_events(void);
int  bg_should_close(void);

/* -- Rendering -- */
void bg_swap_buffers(void);
void bg_clear(uint8_t r, uint8_t g, uint8_t b);

/* -- Drawing -- */
void bg_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void bg_draw_line(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b);
void bg_fill_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
void bg_draw_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);

/* -- Text -- */
void bg_draw_text(int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b);

/* -- Input -- */
void bg_get_mouse_pos(int *x, int *y);
int  bg_is_mouse_down(int button); /* 0=left, 1=right, 2=middle */
int  bg_is_key_down(int keycode);

/* -- UI -- */
/* Returns 1 if button is currently being clicked */
int bg_draw_button(int x, int y, int w, int h, const char *label);

/* =========================================================================
 * 8x8 BITMAP FONT  (visible in header so client can reference glyph data)
 * ========================================================================= */

/* Each character is 8 rows of 8 bits.  Only printable ASCII (32-126). */
static const uint8_t bg_font8x8[95][8] = {
    /* 0x20 SPACE */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x21 !     */ {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00},
    /* 0x22 "     */ {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x23 #     */ {0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00},
    /* 0x24 $     */ {0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00},
    /* 0x25 %     */ {0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00},
    /* 0x26 &     */ {0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00},
    /* 0x27 '     */ {0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00},
    /* 0x28 (     */ {0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00},
    /* 0x29 )     */ {0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00},
    /* 0x2A *     */ {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},
    /* 0x2B +     */ {0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00},
    /* 0x2C ,     */ {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06},
    /* 0x2D -     */ {0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00},
    /* 0x2E .     */ {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00},
    /* 0x2F /     */ {0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00},
    /* 0x30 0     */ {0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00},
    /* 0x31 1     */ {0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00},
    /* 0x32 2     */ {0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00},
    /* 0x33 3     */ {0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00},
    /* 0x34 4     */ {0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00},
    /* 0x35 5     */ {0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00},
    /* 0x36 6     */ {0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00},
    /* 0x37 7     */ {0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00},
    /* 0x38 8     */ {0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00},
    /* 0x39 9     */ {0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00},
    /* 0x3A :     */ {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00},
    /* 0x3B ;     */ {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x06},
    /* 0x3C <     */ {0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00},
    /* 0x3D =     */ {0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00},
    /* 0x3E >     */ {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00},
    /* 0x3F ?     */ {0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00},
    /* 0x40 @     */ {0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00},
    /* 0x41 A     */ {0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00},
    /* 0x42 B     */ {0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00},
    /* 0x43 C     */ {0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00},
    /* 0x44 D     */ {0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00},
    /* 0x45 E     */ {0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00},
    /* 0x46 F     */ {0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00},
    /* 0x47 G     */ {0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00},
    /* 0x48 H     */ {0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00},
    /* 0x49 I     */ {0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
    /* 0x4A J     */ {0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00},
    /* 0x4B K     */ {0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00},
    /* 0x4C L     */ {0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00},
    /* 0x4D M     */ {0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0x00},
    /* 0x4E N     */ {0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00},
    /* 0x4F O     */ {0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00},
    /* 0x50 P     */ {0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00},
    /* 0x51 Q     */ {0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00},
    /* 0x52 R     */ {0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00},
    /* 0x53 S     */ {0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00},
    /* 0x54 T     */ {0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
    /* 0x55 U     */ {0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00},
    /* 0x56 V     */ {0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00},
    /* 0x57 W     */ {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},
    /* 0x58 X     */ {0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00},
    /* 0x59 Y     */ {0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00},
    /* 0x5A Z     */ {0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00},
    /* 0x5B [     */ {0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00},
    /* 0x5C \     */ {0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00},
    /* 0x5D ]     */ {0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00},
    /* 0x5E ^     */ {0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00},
    /* 0x5F _     */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF},
    /* 0x60 `     */ {0x0C,0x0C,0x18,0x00,0x00,0x00,0x00,0x00},
    /* 0x61 a     */ {0x00,0x00,0x1E,0x30,0x3E,0x33,0x6E,0x00},
    /* 0x62 b     */ {0x07,0x06,0x06,0x3E,0x66,0x66,0x3B,0x00},
    /* 0x63 c     */ {0x00,0x00,0x1E,0x33,0x03,0x33,0x1E,0x00},
    /* 0x64 d     */ {0x38,0x30,0x30,0x3e,0x33,0x33,0x6E,0x00},
    /* 0x65 e     */ {0x00,0x00,0x1E,0x33,0x3f,0x03,0x1E,0x00},
    /* 0x66 f     */ {0x1C,0x36,0x06,0x0f,0x06,0x06,0x0F,0x00},
    /* 0x67 g     */ {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x1F},
    /* 0x68 h     */ {0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00},
    /* 0x69 i     */ {0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00},
    /* 0x6A j     */ {0x30,0x00,0x30,0x30,0x30,0x33,0x33,0x1E},
    /* 0x6B k     */ {0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00},
    /* 0x6C l     */ {0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
    /* 0x6D m     */ {0x00,0x00,0x33,0x7F,0x7F,0x6B,0x63,0x00},
    /* 0x6E n     */ {0x00,0x00,0x1F,0x33,0x33,0x33,0x33,0x00},
    /* 0x6F o     */ {0x00,0x00,0x1E,0x33,0x33,0x33,0x1E,0x00},
    /* 0x70 p     */ {0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x0F},
    /* 0x71 q     */ {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x78},
    /* 0x72 r     */ {0x00,0x00,0x3B,0x6E,0x66,0x06,0x0F,0x00},
    /* 0x73 s     */ {0x00,0x00,0x1E,0x03,0x1E,0x30,0x1F,0x00},
    /* 0x74 t     */ {0x08,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0x00},
    /* 0x75 u     */ {0x00,0x00,0x33,0x33,0x33,0x33,0x6E,0x00},
    /* 0x76 v     */ {0x00,0x00,0x33,0x33,0x33,0x1E,0x0C,0x00},
    /* 0x77 w     */ {0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00},
    /* 0x78 x     */ {0x00,0x00,0x63,0x36,0x1C,0x36,0x63,0x00},
    /* 0x79 y     */ {0x00,0x00,0x33,0x33,0x33,0x3E,0x30,0x1F},
    /* 0x7A z     */ {0x00,0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00},
    /* 0x7B {     */ {0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0x00},
    /* 0x7C |     */ {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00},
    /* 0x7D }     */ {0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0x00},
    /* 0x7E ~     */ {0x6E,0x3B,0x00,0x00,0x00,0x00,0x00,0x00},
};

/* =========================================================================
 * IMPLEMENTATION
 * ========================================================================= */
#ifdef BITGFX_IMPLEMENTATION

/* -------------------------------------------------------------------------
 * Shared pixel buffer state
 * ------------------------------------------------------------------------- */
static int      bg__width  = 0;
static int      bg__height = 0;
static uint32_t *bg__pixels = NULL;  /* XRGB / BGRX depending on platform */

static int bg__should_close  = 0;
static int bg__mouse_x       = 0;
static int bg__mouse_y       = 0;
static int bg__mouse_buttons = 0;   /* bit 0=left, 1=right, 2=middle */

/* -------------------------------------------------------------------------
 * Internal helpers (shared)
 * ------------------------------------------------------------------------- */
static inline int bg__clamp(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static inline void bg__put_pixel_raw(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= bg__width || y < 0 || y >= bg__height) return;
#if defined(__APPLE__)
    /* CoreGraphics: ARGB big-endian => stored as 0xAARRGGBB in native uint32 */
    bg__pixels[y * bg__width + x] = (0xFFu << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
#else
    /* X11 XPutImage with 32-bit depth: pixel = 0x00RRGGBB */
    bg__pixels[y * bg__width + x] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
#endif
}

/* =========================================================================
 * LINUX / X11 IMPLEMENTATION
 * ========================================================================= */
#if defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

static Display  *bg__dpy    = NULL;
static Window    bg__win    = 0;
static GC        bg__gc     = 0;
static XImage   *bg__ximg   = NULL;
static Atom      bg__wm_delete;

int bg_init(int width, int height, const char *title) {
    bg__width  = width;
    bg__height = height;
    bg__pixels = (uint32_t*)calloc((size_t)(width * height), sizeof(uint32_t));
    if (!bg__pixels) return 0;

    bg__dpy = XOpenDisplay(NULL);
    if (!bg__dpy) { free(bg__pixels); return 0; }

    int screen = DefaultScreen(bg__dpy);
    Visual *vis = DefaultVisual(bg__dpy, screen);
    int depth   = DefaultDepth(bg__dpy, screen);

    XSetWindowAttributes wa;
    wa.background_pixel = BlackPixel(bg__dpy, screen);
    wa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                    ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                    StructureNotifyMask;

    bg__win = XCreateWindow(bg__dpy, RootWindow(bg__dpy, screen),
                            0, 0, (unsigned)width, (unsigned)height, 0,
                            depth, InputOutput, vis,
                            CWBackPixel | CWEventMask, &wa);

    XStoreName(bg__dpy, bg__win, title);

    bg__wm_delete = XInternAtom(bg__dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(bg__dpy, bg__win, &bg__wm_delete, 1);

    bg__gc = XCreateGC(bg__dpy, bg__win, 0, NULL);

    bg__ximg = XCreateImage(bg__dpy, vis, (unsigned)depth, ZPixmap, 0,
                             (char*)bg__pixels,
                             (unsigned)width, (unsigned)height, 32, 0);
    if (!bg__ximg) { free(bg__pixels); return 0; }

    XMapWindow(bg__dpy, bg__win);
    XFlush(bg__dpy);
    return 1;
}

void bg_poll_events(void) {
    XEvent ev;
    while (XPending(bg__dpy)) {
        XNextEvent(bg__dpy, &ev);
        switch (ev.type) {
            case ClientMessage:
                if ((Atom)ev.xclient.data.l[0] == bg__wm_delete)
                    bg__should_close = 1;
                break;
            case MotionNotify:
                bg__mouse_x = ev.xmotion.x;
                bg__mouse_y = ev.xmotion.y;
                break;
            case ButtonPress:
                if (ev.xbutton.button == Button1) bg__mouse_buttons |=  (1 << 0);
                if (ev.xbutton.button == Button3) bg__mouse_buttons |=  (1 << 1);
                if (ev.xbutton.button == Button2) bg__mouse_buttons |=  (1 << 2);
                break;
            case ButtonRelease:
                if (ev.xbutton.button == Button1) bg__mouse_buttons &= ~(1 << 0);
                if (ev.xbutton.button == Button3) bg__mouse_buttons &= ~(1 << 1);
                if (ev.xbutton.button == Button2) bg__mouse_buttons &= ~(1 << 2);
                break;
            case DestroyNotify:
                bg__should_close = 1;
                break;
            default: break;
        }
    }
}

void bg_swap_buffers(void) {
    XPutImage(bg__dpy, bg__win, bg__gc, bg__ximg,
              0, 0, 0, 0, (unsigned)bg__width, (unsigned)bg__height);
    XFlush(bg__dpy);
}

void bg_terminate(void) {
    if (bg__ximg) {
        bg__ximg->data = NULL; /* we own the buffer */
        XDestroyImage(bg__ximg);
        bg__ximg = NULL;
    }
    if (bg__gc)  { XFreeGC(bg__dpy, bg__gc);      bg__gc  = 0; }
    if (bg__win) { XDestroyWindow(bg__dpy, bg__win); bg__win = 0; }
    if (bg__dpy) { XCloseDisplay(bg__dpy);           bg__dpy = NULL; }
    free(bg__pixels); bg__pixels = NULL;
}

int bg_is_key_down(int keycode) {
    char keys[32];
    XQueryKeymap(bg__dpy, keys);
    KeyCode kc = XKeysymToKeycode(bg__dpy, (KeySym)keycode);
    return (keys[kc >> 3] >> (kc & 7)) & 1;
}

/* =========================================================================
 * macOS IMPLEMENTATION  (ObjC runtime + CoreGraphics, pure C)
 * ========================================================================= */
#elif defined(__APPLE__)

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <CoreGraphics/CoreGraphics.h>
#include <dlfcn.h>

/* Suppress variadic cast warnings on Apple clang */
typedef id   (*id_msgSend)(id, SEL, ...);
typedef void (*void_msgSend)(id, SEL, ...);
typedef BOOL (*bool_msgSend)(id, SEL, ...);
typedef id   (*id_msgSend_rect)(id, SEL, CGRect, ...);

#define BG_MSG(ret, obj, sel_str, ...) \
    ((ret(*)(id,SEL,...))objc_msgSend)((id)(obj), sel_getUid(sel_str), ##__VA_ARGS__)

#define BG_MSG_SUPER(ret, obj, cls_str, sel_str, ...) \
    ((ret(*)(id,SEL,...))objc_msgSendSuper)(\
        &(struct objc_super){(id)(obj), (Class)objc_getClass(cls_str)}, \
        sel_getUid(sel_str), ##__VA_ARGS__)

static id bg__app       = nil;
static id bg__window    = nil;
static id bg__view      = nil;
static id bg__pool      = nil;
static CGContextRef bg__cg_ctx = NULL;
static CGColorSpaceRef bg__colorspace = NULL;

/* ---- Custom NSView subclass for drawing ---- */
static Class bg__ViewClass = NULL;

static void bg__view_drawRect(id self, SEL _cmd, CGRect rect) {
    (void)_cmd; (void)rect;
    CGContextRef ctx = BG_MSG(CGContextRef, BG_MSG(id, objc_getClass("NSGraphicsContext"),
                               "currentContext"), "CGContext");
    if (!ctx) return;

    CGDataProviderRef provider = CGDataProviderCreateWithData(
        NULL, bg__pixels,
        (size_t)(bg__width * bg__height * 4), NULL);
    if (!provider) return;

    CGImageRef img = CGImageCreate(
        (size_t)bg__width, (size_t)bg__height,
        8, 32, (size_t)(bg__width * 4),
        bg__colorspace,
        kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little,
        provider, NULL, false, kCGRenderingIntentDefault);

    CGDataProviderRelease(provider);
    if (!img) return;

    CGRect bounds = CGRectMake(0, 0, bg__width, bg__height);
    CGContextDrawImage(ctx, bounds, img);
    CGImageRelease(img);
}

static void bg__register_view_class(void) {
    bg__ViewClass = objc_allocateClassPair(
        (Class)objc_getClass("NSView"), "BITGFXView", 0);

    Method m = class_getInstanceMethod((Class)objc_getClass("NSView"),
                                        sel_getUid("drawRect:"));
    class_addMethod(bg__ViewClass, sel_getUid("drawRect:"),
                    (IMP)bg__view_drawRect,
                    method_getTypeEncoding(m));
    objc_registerClassPair(bg__ViewClass);
}

/* Track mouse position via event monitoring */
static void bg__update_mouse_from_event(id event) {
    /* locationInWindow returns NSPoint */
    CGPoint loc;
    /* objc_msgSend_stret is needed for structs on some archs,
       but NSPoint is returned in registers on arm64/x86_64 */
    typedef CGPoint (*NSPointFn)(id, SEL);
    loc = ((NSPointFn)objc_msgSend)(event, sel_getUid("locationInWindow"));
    /* Flip Y (AppKit origin is bottom-left) */
    bg__mouse_x = (int)loc.x;
    bg__mouse_y = bg__height - 1 - (int)loc.y;
    bg__mouse_x = bg__clamp(bg__mouse_x, 0, bg__width  - 1);
    bg__mouse_y = bg__clamp(bg__mouse_y, 0, bg__height - 1);
}

int bg_init(int width, int height, const char *title) {
    bg__width  = width;
    bg__height = height;
    bg__pixels = (uint32_t*)calloc((size_t)(width * height), sizeof(uint32_t));
    if (!bg__pixels) return 0;

    bg__colorspace = CGColorSpaceCreateDeviceRGB();

    /* AutoreleasePool */
    bg__pool = BG_MSG(id, BG_MSG(id, objc_getClass("NSAutoreleasePool"), "alloc"), "init");

    /* NSApplication */
    bg__app = BG_MSG(id, objc_getClass("NSApplication"), "sharedApplication");
    BG_MSG(void, bg__app, "setActivationPolicy:", (NSInteger)0); /* NSApplicationActivationPolicyRegular */
    BG_MSG(void, bg__app, "finishLaunching");

    /* Register our view class */
    bg__register_view_class();

    /* NSWindow */
    NSUInteger style = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3); /* titled|closable|miniaturizable|resizable */
    CGRect frame = CGRectMake(100, 100, width, height);

    id win = BG_MSG(id, objc_getClass("NSWindow"), "alloc");
    win = ((id(*)(id,SEL,CGRect,NSUInteger,NSUInteger,BOOL))objc_msgSend)(
              win, sel_getUid("initWithContentRect:styleMask:backing:defer:"),
              frame, style, (NSUInteger)2, NO);
    bg__window = win;

    /* Set title */
    id nsTitle = BG_MSG(id, objc_getClass("NSString"),
                         "stringWithUTF8String:", title);
    BG_MSG(void, bg__window, "setTitle:", nsTitle);

    /* Create view */
    id view = BG_MSG(id, (id)bg__ViewClass, "alloc");
    view = ((id(*)(id,SEL,CGRect))objc_msgSend)(
               view, sel_getUid("initWithFrame:"), frame);
    bg__view = view;

    BG_MSG(void, bg__window, "setContentView:", bg__view);
    BG_MSG(void, bg__window, "makeKeyAndOrderFront:", nil);
    BG_MSG(void, bg__app, "activateIgnoringOtherApps:", YES);

    return 1;
}

void bg_poll_events(void) {
    /* Drain & renew autorelease pool */
    BG_MSG(void, bg__pool, "drain");
    bg__pool = BG_MSG(id, BG_MSG(id, objc_getClass("NSAutoreleasePool"), "alloc"), "init");

    for (;;) {
        id event = ((id(*)(id,SEL,NSUInteger,id,id,BOOL))objc_msgSend)(
                       bg__app,
                       sel_getUid("nextEventMatchingMask:untilDate:inMode:dequeue:"),
                       (NSUInteger)~0UL,   /* NSEventMaskAny */
                       nil,               /* no wait */
                       BG_MSG(id, objc_getClass("NSString"), "stringWithUTF8String:", "kCFRunLoopDefaultMode"),
                       YES);
        if (!event) break;

        NSInteger type = BG_MSG(NSInteger, event, "type");

        switch (type) {
            case  1: /* NSEventTypeLeftMouseDown  */
                bg__mouse_buttons |=  (1 << 0);
                bg__update_mouse_from_event(event);
                break;
            case  2: /* NSEventTypeLeftMouseUp    */
                bg__mouse_buttons &= ~(1 << 0);
                bg__update_mouse_from_event(event);
                break;
            case  3: /* NSEventTypeRightMouseDown */
                bg__mouse_buttons |=  (1 << 1);
                bg__update_mouse_from_event(event);
                break;
            case  4: /* NSEventTypeRightMouseUp   */
                bg__mouse_buttons &= ~(1 << 1);
                bg__update_mouse_from_event(event);
                break;
            case  5: /* NSEventTypeMouseMoved     */
            case  6: /* NSEventTypeLeftMouseDragged */
            case  7: /* NSEventTypeRightMouseDragged */
                bg__update_mouse_from_event(event);
                break;
            case 12: { /* NSEventTypeKeyDown */
                /* Check for window close shortcut Cmd+W */
                NSUInteger mods = BG_MSG(NSUInteger, event, "modifierFlags");
                if (mods & (1 << 20)) { /* NSEventModifierFlagCommand */
                    id chars = BG_MSG(id, event, "charactersIgnoringModifiers");
                    const char *cs = BG_MSG(const char*, chars, "UTF8String");
                    if (cs && cs[0] == 'w') bg__should_close = 1;
                }
                break;
            }
            default: break;
        }

        /* Check if window was closed */
        if (!BG_MSG(BOOL, bg__window, "isVisible"))
            bg__should_close = 1;

        BG_MSG(void, bg__app, "sendEvent:", event);
    }
}

void bg_swap_buffers(void) {
    /* Mark our view as needing display */
    BG_MSG(void, bg__view, "setNeedsDisplay:", YES);
    BG_MSG(void, bg__view, "display");
}

void bg_terminate(void) {
    if (bg__colorspace) { CGColorSpaceRelease(bg__colorspace); bg__colorspace = NULL; }
    if (bg__pool)       { BG_MSG(void, bg__pool, "drain"); bg__pool = NULL; }
    free(bg__pixels); bg__pixels = NULL;
}

int bg_is_key_down(int keycode) {
    /* Basic: check NSEvent modifierFlags for modifier keys */
    (void)keycode;
    return 0; /* Full keymap polling not available without Carbon on modern macOS */
}

#else
#  error "bitgfx.h: Unsupported platform. Only Linux (X11) and macOS are supported."
#endif /* platform */

/* =========================================================================
 * SHARED DRAWING IMPLEMENTATIONS
 * ========================================================================= */

int bg_should_close(void) { return bg__should_close; }

void bg_get_mouse_pos(int *x, int *y) {
    if (x) *x = bg__mouse_x;
    if (y) *y = bg__mouse_y;
}

int bg_is_mouse_down(int button) {
    return (bg__mouse_buttons >> button) & 1;
}

/* ---- bg_clear ---- */
void bg_clear(uint8_t r, uint8_t g, uint8_t b) {
    int n = bg__width * bg__height;
    uint32_t color;
#if defined(__APPLE__)
    color = (0xFFu << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
#else
    color = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
#endif
    for (int i = 0; i < n; i++) bg__pixels[i] = color;
}

/* ---- bg_set_pixel ---- */
void bg_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    bg__put_pixel_raw(x, y, r, g, b);
}

/* ---- bg_draw_line (Bresenham) ---- */
void bg_draw_line(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy, e2;
    for (;;) {
        bg__put_pixel_raw(x0, y0, r, g, b);
        if (x0 == x1 && y0 == y1) break;
        e2 = err * 2;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

/* ---- bg_fill_rect ---- */
void bg_fill_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
    int x1 = bg__clamp(x,     0, bg__width);
    int y1 = bg__clamp(y,     0, bg__height);
    int x2 = bg__clamp(x + w, 0, bg__width);
    int y2 = bg__clamp(y + h, 0, bg__height);
    for (int py = y1; py < y2; py++)
        for (int px = x1; px < x2; px++)
            bg__put_pixel_raw(px, py, r, g, b);
}

/* ---- bg_draw_rect (outline) ---- */
void bg_draw_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
    bg_draw_line(x,         y,         x + w - 1, y,         r, g, b);
    bg_draw_line(x,         y + h - 1, x + w - 1, y + h - 1, r, g, b);
    bg_draw_line(x,         y,         x,          y + h - 1, r, g, b);
    bg_draw_line(x + w - 1, y,         x + w - 1, y + h - 1, r, g, b);
}

/* ---- bg_draw_text ---- */
void bg_draw_text(int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b) {
    if (!text) return;
    int cx = x;
    for (const char *p = text; *p; p++) {
        unsigned char c = (unsigned char)*p;
        if (c == '\n') { cx = x; y += 9; continue; }
        if (c < 32 || c > 126) c = '?';
        const uint8_t *glyph = bg_font8x8[c - 32];
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                if (glyph[row] & (0x80 >> col))
                    bg__put_pixel_raw(cx + col, y + row, r, g, b);
            }
        }
        cx += 9; /* 8px glyph + 1px spacing */
    }
}

/* ---- bg_draw_button ---- */
int bg_draw_button(int x, int y, int w, int h, const char *label) {
    /* Hit test */
    int mx, my;
    bg_get_mouse_pos(&mx, &my);
    int hovered = (mx >= x && mx < x + w && my >= y && my < y + h);
    int clicked = hovered && bg_is_mouse_down(0);

    /* Draw background */
    uint8_t bg_r, bg_g, bg_b;
    if (clicked) {
        bg_r = 60;  bg_g = 60;  bg_b = 80;
    } else if (hovered) {
        bg_r = 80;  bg_g = 80;  bg_b = 120;
    } else {
        bg_r = 50;  bg_g = 50;  bg_b = 90;
    }
    bg_fill_rect(x, y, w, h, bg_r, bg_g, bg_b);

    /* Draw border */
    uint8_t br = clicked ? 120 : 160;
    uint8_t bg_border_g = clicked ? 120 : 160;
    uint8_t bb = clicked ? 180 : 220;
    bg_draw_rect(x, y, w, h, br, bg_border_g, bb);

    /* Center label */
    if (label) {
        int tw = (int)strlen(label) * 9;
        int tx = x + (w - tw) / 2;
        int ty = y + (h - 8) / 2;
        uint8_t tr = clicked ? 180 : 230;
        uint8_t tg = clicked ? 180 : 230;
        uint8_t tb = clicked ? 220 : 255;
        bg_draw_text(tx, ty, label, tr, tg, tb);
    }

    return clicked;
}

#endif /* BITGFX_IMPLEMENTATION */
#endif /* BITGFX_H */
