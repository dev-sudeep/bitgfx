/*
 * bitgfx.h - Minimal single-file, header-only graphics library
 *
 * Provides only:
 *   bg_init()         — create window and pixel buffer
 *   bg_terminate()    — destroy window and free resources
 *   bg_should_close() — returns 1 when the window has been closed
 *   bg_swap_buffers() — push pixel buffer to screen
 *   bg_poll_events()  — process window and input events
 *   bg_set_pixel()    — draw a single pixel
 *   bg_draw_line()    — draw a line (Bresenham)
 *
 * Supports Linux (X11) and macOS (ObjC runtime + CoreGraphics).
 * No .m files, no [] syntax. Pure C throughout.
 *
 * USAGE:
 *   In exactly ONE .c file, before including this header:
 *     #define BITGFX_IMPLEMENTATION
 *   Then in all files:
 *     #include "bitgfx.h"
 *
 * Build — Linux:   gcc -o app main.c -lX11
 * Build — macOS:   clang -o app main.c -framework Cocoa -framework CoreGraphics -lobjc
 *
 * LICENSE: MIT
 */

#ifndef BITGFX_H
#define BITGFX_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/* =========================================================================
 * PUBLIC API
 * ========================================================================= */

/* =========================================================================
 * IMPLEMENTATION
 * ========================================================================= */
#ifdef BITGFX_IMPLEMENTATION

static int       bg__width        = 0;
static int       bg__height       = 0;
static uint32_t *bg__pixels       = NULL;
static int       bg__should_close = 0;

static inline void bg__put_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= bg__width || y < 0 || y >= bg__height) return;
#if defined(__APPLE__)
    bg__pixels[y * bg__width + x] =
        (0xFFu << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
#else
    bg__pixels[y * bg__width + x] =
        ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
#endif
}

/* =========================================================================
 * LINUX / X11
 * ========================================================================= */
#if defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>

static Display *bg__dpy      = NULL;
static Window   bg__win      = 0;
static GC       bg__gc       = 0;
static XImage  *bg__ximg     = NULL;
static Atom     bg__wm_delete;

int bg_init(int width, int height, const char *title) {
    bg__width  = width;
    bg__height = height;
    bg__pixels = (uint32_t*)calloc((size_t)(width * height), sizeof(uint32_t));
    if (!bg__pixels) return 0;

    bg__dpy = XOpenDisplay(NULL);
    if (!bg__dpy) { free(bg__pixels); return 0; }

    int     screen = DefaultScreen(bg__dpy);
    Visual *vis    = DefaultVisual(bg__dpy, screen);
    int     depth  = DefaultDepth(bg__dpy, screen);

    XSetWindowAttributes wa;
    wa.background_pixel = BlackPixel(bg__dpy, screen);
    wa.event_mask = ExposureMask | StructureNotifyMask;

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
        if (ev.type == ClientMessage &&
            (Atom)ev.xclient.data.l[0] == bg__wm_delete)
            bg__should_close = 1;
        if (ev.type == DestroyNotify)
            bg__should_close = 1;
    }
}

void bg_swap_buffers(void) {
    XPutImage(bg__dpy, bg__win, bg__gc, bg__ximg,
              0, 0, 0, 0, (unsigned)bg__width, (unsigned)bg__height);
    XFlush(bg__dpy);
}

void bg_terminate(void) {
    if (bg__ximg) { bg__ximg->data = NULL; XDestroyImage(bg__ximg); bg__ximg = NULL; }
    if (bg__gc)   { XFreeGC(bg__dpy, bg__gc);        bg__gc  = 0; }
    if (bg__win)  { XDestroyWindow(bg__dpy, bg__win); bg__win = 0; }
    if (bg__dpy)  { XCloseDisplay(bg__dpy);           bg__dpy = NULL; }
    free(bg__pixels); bg__pixels = NULL;
}

/* =========================================================================
 * macOS  (ObjC runtime + CoreGraphics, pure C)
 * ========================================================================= */
#elif defined(__APPLE__)

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <CoreGraphics/CoreGraphics.h>

#ifndef BOOL
#  define BOOL signed char
#endif
#ifndef YES
#  define YES ((BOOL)1)
#endif
#ifndef NO
#  define NO  ((BOOL)0)
#endif
#ifndef NSInteger
   typedef long          NSInteger;
   typedef unsigned long NSUInteger;
#endif

#define BG_MSG_VOID(o,s)        ((void(*)(id,SEL))objc_msgSend)((id)(o),(s))
#define BG_MSG_ID(o,s)          ((id(*)(id,SEL))objc_msgSend)((id)(o),(s))
#define BG_MSG_VOID_ID(o,s,a)   ((void(*)(id,SEL,id))objc_msgSend)((id)(o),(s),(id)(a))
#define BG_MSG_VOID_BOOL(o,s,a) ((void(*)(id,SEL,BOOL))objc_msgSend)((id)(o),(s),(BOOL)(a))
#define BG_MSG_VOID_INT(o,s,a)  ((void(*)(id,SEL,NSInteger))objc_msgSend)((id)(o),(s),(NSInteger)(a))
#define BG_MSG_BOOL(o,s)        ((BOOL(*)(id,SEL))objc_msgSend)((id)(o),(s))
#define BG_MSG_INT(o,s)         ((NSInteger(*)(id,SEL))objc_msgSend)((id)(o),(s))
#define BG_MSG_ID_STR(o,s,c)    ((id(*)(id,SEL,const char*))objc_msgSend)((id)(o),(s),(c))
#define BG_MSG_CGCTX(o,s)       ((CGContextRef(*)(id,SEL))objc_msgSend)((id)(o),(s))

static id              bg__app        = nil;
static id              bg__window     = nil;
static id              bg__view       = nil;
static id              bg__pool       = nil;
static CGColorSpaceRef bg__colorspace = NULL;

static SEL bg__sel_alloc, bg__sel_init, bg__sel_drain;
static SEL bg__sel_sharedApp, bg__sel_setActPolicy, bg__sel_finishLaunching;
static SEL bg__sel_initContentRect, bg__sel_setTitle, bg__sel_initFrame;
static SEL bg__sel_setContentView, bg__sel_makeKey, bg__sel_activate;
static SEL bg__sel_setDelegate;
static SEL bg__sel_nextEvent, bg__sel_sendEvent, bg__sel_updateWindows;
static SEL bg__sel_type, bg__sel_isVisible, bg__sel_close;
static SEL bg__sel_setNeedsDisplay, bg__sel_display;
static SEL bg__sel_currentCtx, bg__sel_cgCtx;
static SEL bg__sel_distantPast, bg__sel_stringUTF8, bg__sel_runMode;

static Class bg__cls_NSApp, bg__cls_NSWindow, bg__cls_NSString;
static Class bg__cls_NSDate, bg__cls_NSGfxCtx, bg__cls_NSPool;
static Class bg__cls_NSRunLoop, bg__cls_NSObject;
static Class bg__cls_BITGFXView = NULL;

static void bg__cache(void) {
    bg__sel_alloc          = sel_registerName("alloc");
    bg__sel_init           = sel_registerName("init");
    bg__sel_drain          = sel_registerName("drain");
    bg__sel_sharedApp      = sel_registerName("sharedApplication");
    bg__sel_setActPolicy   = sel_registerName("setActivationPolicy:");
    bg__sel_finishLaunching= sel_registerName("finishLaunching");
    bg__sel_initContentRect= sel_registerName("initWithContentRect:styleMask:backing:defer:");
    bg__sel_setTitle       = sel_registerName("setTitle:");
    bg__sel_initFrame      = sel_registerName("initWithFrame:");
    bg__sel_setContentView = sel_registerName("setContentView:");
    bg__sel_makeKey        = sel_registerName("makeKeyAndOrderFront:");
    bg__sel_activate       = sel_registerName("activateIgnoringOtherApps:");
    bg__sel_setDelegate    = sel_registerName("setDelegate:");
    bg__sel_nextEvent      = sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:");
    bg__sel_sendEvent      = sel_registerName("sendEvent:");
    bg__sel_updateWindows  = sel_registerName("updateWindows");
    bg__sel_type           = sel_registerName("type");
    bg__sel_isVisible      = sel_registerName("isVisible");
    bg__sel_close          = sel_registerName("close");
    bg__sel_setNeedsDisplay= sel_registerName("setNeedsDisplay:");
    bg__sel_display        = sel_registerName("display");
    bg__sel_currentCtx     = sel_registerName("currentContext");
    bg__sel_cgCtx          = sel_registerName("CGContext");
    bg__sel_distantPast    = sel_registerName("distantPast");
    bg__sel_stringUTF8     = sel_registerName("stringWithUTF8String:");
    bg__sel_runMode        = sel_registerName("runMode:beforeDate:");

    bg__cls_NSApp    = objc_getClass("NSApplication");
    bg__cls_NSWindow = objc_getClass("NSWindow");
    bg__cls_NSString = objc_getClass("NSString");
    bg__cls_NSDate   = objc_getClass("NSDate");
    bg__cls_NSGfxCtx = objc_getClass("NSGraphicsContext");
    bg__cls_NSPool   = objc_getClass("NSAutoreleasePool");
    bg__cls_NSRunLoop= objc_getClass("NSRunLoop");
    bg__cls_NSObject = objc_getClass("NSObject");
}

static void bg__drawRect(id self, SEL _cmd, CGRect rect) {
    (void)self; (void)_cmd; (void)rect;
    id gc = BG_MSG_ID((id)bg__cls_NSGfxCtx, bg__sel_currentCtx);
    if (!gc) return;
    CGContextRef ctx = BG_MSG_CGCTX(gc, bg__sel_cgCtx);
    if (!ctx) return;
    size_t row_bytes = (size_t)bg__width * 4;
    CGDataProviderRef prov = CGDataProviderCreateWithData(
        NULL, bg__pixels, row_bytes * (size_t)bg__height, NULL);
    if (!prov) return;
    CGImageRef img = CGImageCreate(
        (size_t)bg__width, (size_t)bg__height, 8, 32, row_bytes,
        bg__colorspace,
        kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little,
        prov, NULL, false, kCGRenderingIntentDefault);
    CGDataProviderRelease(prov);
    if (!img) return;
    CGContextDrawImage(ctx,
        CGRectMake(0, (CGFloat)bg__height,
                   (CGFloat)bg__width, -(CGFloat)bg__height),
        img);
    CGImageRelease(img);
}

static void bg__register_view(void) {
    if (objc_getClass("BITGFXView")) {
        bg__cls_BITGFXView = (Class)objc_getClass("BITGFXView"); return;
    }
    bg__cls_BITGFXView = objc_allocateClassPair(
        (Class)objc_getClass("NSView"), "BITGFXView", 0);
    Method m = class_getInstanceMethod(
        (Class)objc_getClass("NSView"), sel_registerName("drawRect:"));
    class_addMethod(bg__cls_BITGFXView, sel_registerName("drawRect:"),
                    (IMP)bg__drawRect, method_getTypeEncoding(m));
    objc_registerClassPair(bg__cls_BITGFXView);
}

static void bg__winWillClose(id self, SEL _cmd, id n) {
    (void)self; (void)_cmd; (void)n;
    bg__should_close = 1;
}

static NSUInteger bg__appShouldTerminate(id self, SEL _cmd, id sender) {
    (void)self; (void)_cmd; (void)sender;
    bg__should_close = 1;
    return 0;
}

static void bg__register_delegates(void) {
    if (!objc_getClass("BITGFXWinDelegate")) {
        Class wc = objc_allocateClassPair(bg__cls_NSObject, "BITGFXWinDelegate", 0);
        class_addMethod(wc, sel_registerName("windowWillClose:"),
                        (IMP)bg__winWillClose, "v@:@");
        objc_registerClassPair(wc);
    }
    if (!objc_getClass("BITGFXAppDelegate")) {
        Class ac = objc_allocateClassPair(bg__cls_NSObject, "BITGFXAppDelegate", 0);
        class_addMethod(ac, sel_registerName("applicationShouldTerminate:"),
                        (IMP)bg__appShouldTerminate, "L@:@");
        objc_registerClassPair(ac);
    }
}

int bg_init(int width, int height, const char *title) {
    bg__width  = width;
    bg__height = height;
    bg__pixels = (uint32_t*)calloc((size_t)(width * height), sizeof(uint32_t));
    if (!bg__pixels) return 0;

    bg__colorspace = CGColorSpaceCreateDeviceRGB();
    bg__cache();

    bg__pool = BG_MSG_ID((id)bg__cls_NSPool, bg__sel_alloc);
    bg__pool = BG_MSG_ID(bg__pool, bg__sel_init);

    bg__app = BG_MSG_ID((id)bg__cls_NSApp, bg__sel_sharedApp);
    BG_MSG_VOID_INT(bg__app, bg__sel_setActPolicy, (NSInteger)0);

    bg__register_view();
    bg__register_delegates();

    id appDel = BG_MSG_ID(
        BG_MSG_ID((id)objc_getClass("BITGFXAppDelegate"), bg__sel_alloc),
        bg__sel_init);
    BG_MSG_VOID_ID(bg__app, bg__sel_setDelegate, appDel);
    BG_MSG_VOID(bg__app, bg__sel_finishLaunching);

    NSUInteger style = (1<<0)|(1<<1)|(1<<2)|(1<<3);
    CGRect frame = CGRectMake(200, 200, (CGFloat)width, (CGFloat)height);
    id win = BG_MSG_ID((id)bg__cls_NSWindow, bg__sel_alloc);
    win = ((id(*)(id,SEL,CGRect,NSUInteger,NSUInteger,BOOL))objc_msgSend)(
              win, bg__sel_initContentRect, frame, style, (NSUInteger)2, (BOOL)0);
    bg__window = win;

    id nsTitle = BG_MSG_ID_STR((id)bg__cls_NSString, bg__sel_stringUTF8, title);
    BG_MSG_VOID_ID(bg__window, bg__sel_setTitle, nsTitle);

    CGRect cr = CGRectMake(0, 0, (CGFloat)width, (CGFloat)height);
    id view = BG_MSG_ID((id)bg__cls_BITGFXView, bg__sel_alloc);
    view = ((id(*)(id,SEL,CGRect))objc_msgSend)(view, bg__sel_initFrame, cr);
    bg__view = view;

    BG_MSG_VOID_ID(bg__window, bg__sel_setContentView, bg__view);

    id winDel = BG_MSG_ID(
        BG_MSG_ID((id)objc_getClass("BITGFXWinDelegate"), bg__sel_alloc),
        bg__sel_init);
    BG_MSG_VOID_ID(bg__window, bg__sel_setDelegate, winDel);
    BG_MSG_VOID_ID(bg__window, bg__sel_makeKey, nil);
    BG_MSG_VOID_BOOL(bg__app, bg__sel_activate, (BOOL)1);

    return 1;
}

void bg_poll_events(void) {
    BG_MSG_VOID(bg__pool, bg__sel_drain);
    bg__pool = BG_MSG_ID((id)bg__cls_NSPool, bg__sel_alloc);
    bg__pool = BG_MSG_ID(bg__pool, bg__sel_init);

    id rl   = BG_MSG_ID((id)bg__cls_NSRunLoop, sel_registerName("currentRunLoop"));
    id mode = BG_MSG_ID_STR((id)bg__cls_NSString, bg__sel_stringUTF8, "NSDefaultRunLoopMode");
    id past = BG_MSG_ID((id)bg__cls_NSDate, bg__sel_distantPast);
    ((BOOL(*)(id,SEL,id,id))objc_msgSend)(rl, bg__sel_runMode, mode, past);

    for (;;) {
        id ev = ((id(*)(id,SEL,NSUInteger,id,id,BOOL))objc_msgSend)(
                    bg__app, bg__sel_nextEvent,
                    (NSUInteger)~0UL, past, mode, (BOOL)1);
        if (!ev) break;
        BG_MSG_VOID_ID(bg__app, bg__sel_sendEvent, ev);
        if (!bg__should_close && bg__window &&
            !BG_MSG_BOOL(bg__window, bg__sel_isVisible))
            bg__should_close = 1;
    }
    BG_MSG_VOID(bg__app, bg__sel_updateWindows);
}

void bg_swap_buffers(void) {
    BG_MSG_VOID_BOOL(bg__view, bg__sel_setNeedsDisplay, (BOOL)1);
    BG_MSG_VOID(bg__view, bg__sel_display);
}

void bg_terminate(void) {
    if (bg__window)     { BG_MSG_VOID(bg__window, bg__sel_close); bg__window = nil; }
    if (bg__colorspace) { CGColorSpaceRelease(bg__colorspace); bg__colorspace = NULL; }
    if (bg__pool)       { BG_MSG_VOID(bg__pool, bg__sel_drain); bg__pool = nil; }
    bg__view = nil; bg__app = nil;
    free(bg__pixels); bg__pixels = NULL;
}

#else
#  error "bitgfx.h: unsupported platform (Linux and macOS only)"
#endif /* platform */

/* =========================================================================
 * SHARED IMPLEMENTATIONS
 * ========================================================================= */


struct bgColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};
struct bgPos{
    int x;
    int y;
};
struct Rect{
    int x0;
    int y0;
    int x1;
    int y1;
};

typedef struct bgColor bgColor;
typedef struct bgPos bgPos;
typedef struct Rect Rect;

int bg_should_close(void) { return bg__should_close; }

void bg_set_pixel(bgPos p, bgColor c) {
    bg__put_pixel(p.x, p.y, c.r, c.g, c.b);
}

void bg_draw_line(bgPos p0, bgPos p1, bgColor c) {
    int dx = abs(p1.x - p0.x), dy = abs(p1.y - p0.y);
    int sx = p0.x < p1.x ? 1 : -1;
    int sy = p0.y < p1.y ? 1 : -1;
    int err = dx - dy, e2;
    for (;;) {
        bg__put_pixel(p0.x, p0.y, c.r, c.g, c.b);
        if (p0.x == p1.x && p0.y == p1.y) break;
        e2 = err * 2;
        if (e2 > -dy) { err -= dy; p0.x += sx; }
        if (e2 <  dx) { err += dx; p0.y += sy; }
    }
}

void bg_draw_rect(Rect r, bool outlined, bgColor c){

    if(!outlined){
        for(int i = r.y0; i != r.y1;){
            bg_draw_line((bgPos){r.x0, i}, (bgPos){r.x1, i}, c);
            if(r.y0 < r.y1) i++;
            if(r.y0 > r.y1) i--;
        }
    }else{
        bg_draw_line((bgPos){r.x0, r.y0}, (bgPos){r.x0, r.y1}, c);
        bg_draw_line((bgPos){r.x0, r.y1}, (bgPos){r.x1, r.y1}, c);
        bg_draw_line((bgPos){r.x1, r.y0}, (bgPos){r.x1, r.y1}, c);
        bg_draw_line((bgPos){r.x0, r.y0}, (bgPos){r.x1, r.y1}, c);
    }
}



#endif /* BITGFX_IMPLEMENTATION */
#endif /* BITGFX_H */
