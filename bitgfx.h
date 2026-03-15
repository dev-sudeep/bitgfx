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

/* Special key codes returned by bg_next_key() */
#define BG_KEY_NONE       0
#define BG_KEY_BACKSPACE  8
#define BG_KEY_TAB        9
#define BG_KEY_ENTER      13
#define BG_KEY_ESCAPE     27
#define BG_KEY_LEFT       0x10001u
#define BG_KEY_RIGHT      0x10002u
#define BG_KEY_UP         0x10003u
#define BG_KEY_DOWN       0x10004u
#define BG_KEY_DELETE     0x10005u
#define BG_KEY_HOME       0x10006u
#define BG_KEY_END        0x10007u

/*
 * Dequeue one key event from the internal ring buffer.
 * Returns BG_KEY_NONE (0) when the queue is empty.
 * Printable ASCII characters are returned as their char value (32-126).
 * Special keys are returned as one of the BG_KEY_* constants above.
 */
uint32_t bg_next_key(void);

/* -- UI -- */
/* Returns 1 if button is currently being clicked */
int bg_draw_button(int x, int y, int w, int h, const char *label);

/*
 * Text input box  --  immediate-mode, caller owns the BgTextbox struct.
 *
 * Usage:
 *   static BgTextbox tb;          // zero-initialise once
 *   bg_draw_textbox(10, 10, 200, 24, &tb);
 *   // tb.buf holds the current text, tb.len is its length.
 *
 * The box gains focus when clicked and loses it when Escape is pressed or
 * the user clicks elsewhere.  Backspace deletes the last character.
 * Returns 1 on the frame that Enter is pressed while the box is focused.
 */
#define BG_TEXTBOX_MAX 255

typedef struct {
    char     buf[BG_TEXTBOX_MAX + 1]; /* null-terminated text            */
    int      len;                      /* current character count         */
    int      focused;                  /* 1 = this box has keyboard focus */
    uint32_t cursor_timer;             /* frame counter for cursor blink  */
} BgTextbox;

int bg_draw_textbox(int x, int y, int w, int h, BgTextbox *tb);

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
static int bg__mouse_click    = 0;   /* 1 on the frame LMB transitions low->high */

/* Key event ring buffer (platform-independent) */
#define BG__KEY_QUEUE_SIZE 64
static uint32_t bg__key_queue[BG__KEY_QUEUE_SIZE];
static int      bg__key_head = 0; /* read  index */
static int      bg__key_tail = 0; /* write index */

static inline void bg__key_push(uint32_t k) {
    int next = (bg__key_tail + 1) % BG__KEY_QUEUE_SIZE;
    if (next != bg__key_head) {
        bg__key_queue[bg__key_tail] = k;
        bg__key_tail = next;
    }
}

/* -------------------------------------------------------------------------
 * Internal helpers (shared)
 * ------------------------------------------------------------------------- */
static inline int bg__clamp(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static inline void bg__put_pixel_raw(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= bg__width || y < 0 || y >= bg__height) return;
#if defined(__APPLE__)
    /* CoreGraphics with kCGBitmapByteOrder32Little|kCGImageAlphaNoneSkipFirst:
       bytes in memory are [B, G, R, X] — on a little-endian machine this is
       stored as 0x00RRGGBB in a uint32_t with the alpha/skip byte in bits 24-31.
       We write 0xFF in the high byte so CGImageCreate sees a valid pixel. */
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

    /* XCreateImage: bitmap_pad=32, bytes_per_line=0 (auto-calculated).
       Works on both 24-bit and 32-bit TrueColor visuals because X11 uses
       32bpp storage for 24-bit depth internally.  If DefaultVisual is not
       TrueColor (very rare on modern systems) XPutImage will produce wrong
       colours but won't crash. */
    bg__ximg = XCreateImage(bg__dpy, vis, (unsigned)depth, ZPixmap, 0,
                             (char*)bg__pixels,
                             (unsigned)width, (unsigned)height, 32, 0);
    if (!bg__ximg) { free(bg__pixels); return 0; }

    XMapWindow(bg__dpy, bg__win);
    XFlush(bg__dpy);
    return 1;
}

void bg_poll_events(void) {
    bg__mouse_click = 0; /* cleared at the start of every poll */
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
                if (ev.xbutton.button == Button1) { bg__mouse_buttons |= (1<<0); bg__mouse_click = 1; }
                if (ev.xbutton.button == Button3)   bg__mouse_buttons |= (1<<1);
                if (ev.xbutton.button == Button2)   bg__mouse_buttons |= (1<<2);
                break;
            case ButtonRelease:
                if (ev.xbutton.button == Button1) bg__mouse_buttons &= ~(1 << 0);
                if (ev.xbutton.button == Button3) bg__mouse_buttons &= ~(1 << 1);
                if (ev.xbutton.button == Button2) bg__mouse_buttons &= ~(1 << 2);
                break;
            case DestroyNotify:
                bg__should_close = 1;
                break;
            case KeyPress: {
                char    buf[8] = {0};
                KeySym  sym    = 0;
                int     n      = XLookupString(&ev.xkey, buf, sizeof(buf)-1, &sym, NULL);
                uint32_t k     = BG_KEY_NONE;
                /* Map special keys first */
                switch (sym) {
                    case XK_BackSpace: k = BG_KEY_BACKSPACE; break;
                    case XK_Tab:       k = BG_KEY_TAB;       break;
                    case XK_Return:
                    case XK_KP_Enter:  k = BG_KEY_ENTER;     break;
                    case XK_Escape:    k = BG_KEY_ESCAPE;     break;
                    case XK_Left:      k = BG_KEY_LEFT;       break;
                    case XK_Right:     k = BG_KEY_RIGHT;      break;
                    case XK_Up:        k = BG_KEY_UP;         break;
                    case XK_Down:      k = BG_KEY_DOWN;       break;
                    case XK_Delete:    k = BG_KEY_DELETE;     break;
                    case XK_Home:      k = BG_KEY_HOME;       break;
                    case XK_End:       k = BG_KEY_END;        break;
                    default:
                        if (n > 0 && (unsigned char)buf[0] >= 32 && (unsigned char)buf[0] <= 126)
                            k = (uint32_t)(unsigned char)buf[0];
                        break;
                }
                if (k != BG_KEY_NONE) bg__key_push(k);
                break;
            }
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
 *
 * bg_init() is called from main() which IS the OS main thread.
 * All AppKit/NSWindow setup happens there. bg_poll_events() spins NSRunLoop
 * for one iteration per frame -- the correct way to drive AppKit without
 * blocking. No pthread needed.
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
#define BG_MSG_UINT(o,s)        ((NSUInteger(*)(id,SEL))objc_msgSend)((id)(o),(s))
#define BG_MSG_ID_STR(o,s,c)    ((id(*)(id,SEL,const char*))objc_msgSend)((id)(o),(s),(c))
#define BG_MSG_CSTR(o,s)        ((const char*(*)(id,SEL))objc_msgSend)((id)(o),(s))
#define BG_MSG_CGCTX(o,s)       ((CGContextRef(*)(id,SEL))objc_msgSend)((id)(o),(s))

static id              bg__app    = nil;
static id              bg__window = nil;
static id              bg__view   = nil;
static id              bg__pool   = nil;
static CGColorSpaceRef bg__colorspace = NULL;

static SEL bg__sel_alloc, bg__sel_init, bg__sel_drain;
static SEL bg__sel_sharedApp, bg__sel_setActPolicy, bg__sel_finishLaunching;
static SEL bg__sel_initContentRect, bg__sel_setTitle, bg__sel_initFrame;
static SEL bg__sel_setContentView, bg__sel_makeKey, bg__sel_activate;
static SEL bg__sel_setDelegate, bg__sel_setMouseMoved;
static SEL bg__sel_nextEvent, bg__sel_sendEvent, bg__sel_updateWindows;
static SEL bg__sel_type, bg__sel_locationInWindow;
static SEL bg__sel_modFlags, bg__sel_keyCode, bg__sel_chars, bg__sel_charsIgn;
static SEL bg__sel_utf8, bg__sel_isVisible, bg__sel_close;
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
    bg__sel_setMouseMoved  = sel_registerName("setAcceptsMouseMovedEvents:");
    bg__sel_nextEvent      = sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:");
    bg__sel_sendEvent      = sel_registerName("sendEvent:");
    bg__sel_updateWindows  = sel_registerName("updateWindows");
    bg__sel_type           = sel_registerName("type");
    bg__sel_locationInWindow = sel_registerName("locationInWindow");
    bg__sel_modFlags       = sel_registerName("modifierFlags");
    bg__sel_keyCode        = sel_registerName("keyCode");
    bg__sel_chars          = sel_registerName("characters");
    bg__sel_charsIgn       = sel_registerName("charactersIgnoringModifiers");
    bg__sel_utf8           = sel_registerName("UTF8String");
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
    /* CGContextDrawImage maps the image's row 0 to the bottom-left of the
       dest rect (CG lower-left origin). Our buffer has row 0 at the top.
       Drawing into a rect with origin.y = height and height = -height makes
       CG map row 0 of the image to the top of the view instead. We do NOT
       touch the CTM — AppKit may have already applied transforms and adding
       another flip would double-invert the image. */
    CGContextDrawImage(ctx,
        CGRectMake(0, (CGFloat)bg__height,
                   (CGFloat)bg__width, -(CGFloat)bg__height),
        img);
    CGImageRelease(img);
}

static void bg__register_view(void) {
    if (objc_getClass("BITGFXView")) { bg__cls_BITGFXView = (Class)objc_getClass("BITGFXView"); return; }
    bg__cls_BITGFXView = objc_allocateClassPair((Class)objc_getClass("NSView"), "BITGFXView", 0);
    Method m = class_getInstanceMethod((Class)objc_getClass("NSView"), sel_registerName("drawRect:"));
    class_addMethod(bg__cls_BITGFXView, sel_registerName("drawRect:"), (IMP)bg__drawRect, method_getTypeEncoding(m));
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
        class_addMethod(wc, sel_registerName("windowWillClose:"), (IMP)bg__winWillClose, "v@:@");
        objc_registerClassPair(wc);
    }
    if (!objc_getClass("BITGFXAppDelegate")) {
        Class ac = objc_allocateClassPair(bg__cls_NSObject, "BITGFXAppDelegate", 0);
        class_addMethod(ac, sel_registerName("applicationShouldTerminate:"),
                        (IMP)bg__appShouldTerminate, "L@:@");
        objc_registerClassPair(ac);
    }
}

static void bg__mouse_from_event(id ev) {
    typedef CGPoint (*PointFn)(id, SEL);
    CGPoint p = ((PointFn)objc_msgSend)(ev, bg__sel_locationInWindow);
    bg__mouse_x = bg__clamp((int)p.x,                  0, bg__width  - 1);
    bg__mouse_y = bg__clamp(bg__height - 1 - (int)p.y, 0, bg__height - 1);
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

    id appDel = BG_MSG_ID(BG_MSG_ID((id)objc_getClass("BITGFXAppDelegate"), bg__sel_alloc), bg__sel_init);
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

    id winDel = BG_MSG_ID(BG_MSG_ID((id)objc_getClass("BITGFXWinDelegate"), bg__sel_alloc), bg__sel_init);
    BG_MSG_VOID_ID(bg__window, bg__sel_setDelegate, winDel);
    BG_MSG_VOID_BOOL(bg__window, bg__sel_setMouseMoved, (BOOL)1);
    BG_MSG_VOID_ID(bg__window, bg__sel_makeKey, nil);
    BG_MSG_VOID_BOOL(bg__app, bg__sel_activate, (BOOL)1);

    return 1;
}

void bg_poll_events(void) {
    bg__mouse_click = 0;

    BG_MSG_VOID(bg__pool, bg__sel_drain);
    bg__pool = BG_MSG_ID((id)bg__cls_NSPool, bg__sel_alloc);
    bg__pool = BG_MSG_ID(bg__pool, bg__sel_init);

    /* Spin NSRunLoop once so AppKit can process internal sources */
    id rl   = BG_MSG_ID((id)bg__cls_NSRunLoop, sel_registerName("currentRunLoop"));
    id mode = BG_MSG_ID_STR((id)bg__cls_NSString, bg__sel_stringUTF8, "NSDefaultRunLoopMode");
    id past = BG_MSG_ID((id)bg__cls_NSDate, bg__sel_distantPast);
    ((BOOL(*)(id,SEL,id,id))objc_msgSend)(rl, bg__sel_runMode, mode, past);

    /* Drain all pending NSEvents */
    for (;;) {
        id ev = ((id(*)(id,SEL,NSUInteger,id,id,BOOL))objc_msgSend)(
                    bg__app, bg__sel_nextEvent,
                    (NSUInteger)~0UL, past, mode, (BOOL)1);
        if (!ev) break;

        NSInteger type = BG_MSG_INT(ev, bg__sel_type);
        switch (type) {
            case 1: bg__mouse_buttons |= (1<<0); bg__mouse_click = 1; bg__mouse_from_event(ev); break;
            case 2: bg__mouse_buttons &= ~(1<<0); bg__mouse_from_event(ev); break;
            case 3: bg__mouse_buttons |= (1<<1); bg__mouse_from_event(ev); break;
            case 4: bg__mouse_buttons &= ~(1<<1); bg__mouse_from_event(ev); break;
            case 5: case 6: case 7: bg__mouse_from_event(ev); break;
            case 10: { /* NSEventTypeKeyDown */
                NSUInteger mods = BG_MSG_UINT(ev, bg__sel_modFlags);
                if (mods & (1u<<20)) {
                    id ci = BG_MSG_ID(ev, bg__sel_charsIgn);
                    const char *cw = BG_MSG_CSTR(ci, bg__sel_utf8);
                    if (cw && cw[0] == 'w') { bg__should_close = 1; break; }
                }
                if (!(mods & ((1u<<20)|(1u<<18)))) {
                    NSUInteger kc = BG_MSG_UINT(ev, bg__sel_keyCode);
                    uint32_t k = BG_KEY_NONE;
                    switch (kc) {
                        case  51: k = BG_KEY_BACKSPACE; break;
                        case  36: case 76: k = BG_KEY_ENTER; break;
                        case  53: k = BG_KEY_ESCAPE;  break;
                        case  48: k = BG_KEY_TAB;     break;
                        case 123: k = BG_KEY_LEFT;    break;
                        case 124: k = BG_KEY_RIGHT;   break;
                        case 126: k = BG_KEY_UP;      break;
                        case 125: k = BG_KEY_DOWN;    break;
                        case 117: k = BG_KEY_DELETE;  break;
                        case 115: k = BG_KEY_HOME;    break;
                        case 119: k = BG_KEY_END;     break;
                        default: {
                            id ch = BG_MSG_ID(ev, bg__sel_chars);
                            const char *cs = BG_MSG_CSTR(ch, bg__sel_utf8);
                            if (cs && (unsigned char)cs[0] >= 32 && (unsigned char)cs[0] <= 126)
                                k = (uint32_t)(unsigned char)cs[0];
                        }
                    }
                    if (k) bg__key_push(k);
                }
                break;
            }
            default: break;
        }
        BG_MSG_VOID_ID(bg__app, bg__sel_sendEvent, ev);
        if (!bg__should_close && bg__window && !BG_MSG_BOOL(bg__window, bg__sel_isVisible))
            bg__should_close = 1;
    }
    BG_MSG_VOID(bg__app, bg__sel_updateWindows);
}

void bg_swap_buffers(void) {
    BG_MSG_VOID_BOOL(bg__view, bg__sel_setNeedsDisplay, (BOOL)1);
    BG_MSG_VOID(bg__view, bg__sel_display);
}

void bg_terminate(void) {
    if (bg__window) { BG_MSG_VOID(bg__window, bg__sel_close); bg__window = nil; }
    if (bg__colorspace) { CGColorSpaceRelease(bg__colorspace); bg__colorspace = NULL; }
    if (bg__pool) { BG_MSG_VOID(bg__pool, bg__sel_drain); bg__pool = nil; }
    bg__view = nil; bg__app = nil;
    free(bg__pixels); bg__pixels = NULL;
}

int bg_is_key_down(int keycode) {
    (void)keycode;
    return 0;
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
    if (button < 0 || button > 2) return 0;
    return (bg__mouse_buttons >> button) & 1;
}

/* ---- bg_next_key ---- */
uint32_t bg_next_key(void) {
    if (bg__key_head == bg__key_tail) return BG_KEY_NONE;
    uint32_t k = bg__key_queue[bg__key_head];
    bg__key_head = (bg__key_head + 1) % BG__KEY_QUEUE_SIZE;
    return k;
}

/* ---- bg_draw_textbox ---- */
int bg_draw_textbox(int x, int y, int w, int h, BgTextbox *tb) {
    int entered = 0;

    /* --- Focus: click inside gains focus, click outside loses it --- */
    int mx, my;
    bg_get_mouse_pos(&mx, &my);
    int inside = (mx >= x && mx < x + w && my >= y && my < y + h);

    /* Detect left mouse press this frame via a one-shot edge (compare
       previous frame state is not tracked, so we just test the instant
       the button is held and the cursor is inside -- good enough for
       immediate mode at typical frame rates).                          */
    int clicked = bg__mouse_click;

    if (clicked) {
        tb->focused = inside ? 1 : 0;
    }

    /* --- Process key events when focused --- */
    if (tb->focused) {
        uint32_t k;
        while ((k = bg_next_key()) != BG_KEY_NONE) {
            if (k == BG_KEY_ESCAPE) {
                tb->focused = 0;
            } else if (k == BG_KEY_ENTER) {
                entered = 1;
                /* keep focus so caller can decide */
            } else if (k == BG_KEY_BACKSPACE) {
                if (tb->len > 0) {
                    tb->len--;
                    tb->buf[tb->len] = '\0';
                }
            } else if (k >= 32 && k <= 126) {
                if (tb->len < BG_TEXTBOX_MAX) {
                    tb->buf[tb->len++] = (char)k;
                    tb->buf[tb->len]   = '\0';
                }
            }
        }
        tb->cursor_timer++;
    } else {
        tb->cursor_timer = 0;
    }

    /* --- Draw box background --- */
    uint8_t bg_r, bg_g, bg_b;
    if (tb->focused) {
        bg_r = 18; bg_g = 18; bg_b = 30;
    } else {
        bg_r = 28; bg_g = 28; bg_b = 38;
    }
    bg_fill_rect(x, y, w, h, bg_r, bg_g, bg_b);

    /* --- Border (brighter when focused) --- */
    uint8_t bdr_r, bdr_g, bdr_b;
    if (tb->focused) {
        bdr_r = 100; bdr_g = 140; bdr_b = 255;
    } else {
        bdr_r = 80;  bdr_g =  80; bdr_b = 110;
    }
    bg_draw_rect(x, y, w, h, bdr_r, bdr_g, bdr_b);

    /* --- Text content (clipped to box with 4px inner padding) --- */
    int pad   = 4;
    int max_chars = (w - pad * 2 - 10 /* cursor width */) / 9;
    if (max_chars < 1) max_chars = 1;

    /* Show a suffix of the buffer so the newest characters are visible */
    const char *display = tb->buf;
    int dlen = tb->len;
    if (dlen > max_chars) {
        display = tb->buf + (dlen - max_chars);
        dlen    = max_chars;
    }

    int ty = y + (h - 8) / 2;
    bg_draw_text(x + pad, ty, display, 220, 220, 240);

    /* --- Blinking cursor (30-frame period) --- */
    if (tb->focused && (tb->cursor_timer % 30) < 15) {
        int cx = x + pad + dlen * 9;
        /* Make sure cursor stays inside box */
        if (cx < x + w - pad - 2) {
            bg_draw_line(cx, ty, cx, ty + 7, 180, 200, 255);
        }
    }

    return entered;
}

/* ---- bg_clear ---- */
void bg_clear(uint8_t r, uint8_t g, uint8_t b) {
    size_t n = (size_t)bg__width * (size_t)bg__height;
    uint32_t color;
#if defined(__APPLE__)
    color = (0xFFu << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
#else
    color = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
#endif
    for (size_t i = 0; i < n; i++) bg__pixels[i] = color;
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
                if (glyph[row] & (1 << col))
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