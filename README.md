# bitgfx.h

A single-file, header-only 2D graphics library written in pure C. Create windows, draw pixels, handle mouse and keyboard input, and build immediate-mode UI widgets — with no external dependencies beyond the platform's own system libraries.

Supports **Linux** (via X11) and **macOS** (via the Objective-C runtime and CoreGraphics). No `.m` files, no `[]` syntax — the macOS backend is written entirely in C using `objc_msgSend`.

---

## Features

- Single header, no build system required
- Immediate-mode pixel buffer — every frame you draw directly into a `uint32_t` array
- Bresenham line drawing, filled and outlined rectangles
- Hardcoded 8×8 bitmap font covering all printable ASCII (32–126)
- Mouse position, button state, and click-edge detection
- Key event queue with special key constants (`BG_KEY_ENTER`, `BG_KEY_BACKSPACE`, etc.)
- `bg_draw_button` — hover, press, and release states out of the box
- `bg_draw_textbox` — click-to-focus text input with blinking cursor and scroll

---

## Quick Start

Copy `bitgfx.h` into your project. In **exactly one** `.c` file define the implementation macro before including the header:

```c
#define BITGFX_IMPLEMENTATION
#include "bitgfx.h"
```

In all other files just include normally:

```c
#include "bitgfx.h"
```

### Minimal example

```c
#define BITGFX_IMPLEMENTATION
#include "bitgfx.h"

int main(void) {
    bg_init(800, 600, "Hello bitgfx");

    while (!bg_should_close()) {
        bg_poll_events();
        bg_clear(20, 20, 30);

        bg_draw_text(10, 10, "Hello, World!", 255, 255, 255);
        bg_draw_line(0, 0, 800, 600, 255, 100, 50);
        bg_fill_rect(100, 100, 200, 80, 50, 120, 200);

        bg_swap_buffers();
    }

    bg_terminate();
    return 0;
}
```

---

## Building

### Linux

```bash
gcc -o myapp main.c -lX11
```

### macOS

```bash
clang -o myapp main.c -framework Cocoa -framework CoreGraphics -lobjc
```

---

## API Reference

### Lifecycle

```c
int  bg_init(int width, int height, const char *title);
void bg_terminate(void);
```

`bg_init` opens the window and allocates the pixel buffer. Returns `1` on success, `0` on failure. Call `bg_terminate` to release all resources.

---

### Event Loop

```c
void bg_poll_events(void);
int  bg_should_close(void);
```

Call `bg_poll_events` once per frame to process window and input events. `bg_should_close` returns `1` when the user has closed the window.

---

### Rendering

```c
void bg_swap_buffers(void);
void bg_clear(uint8_t r, uint8_t g, uint8_t b);
```

`bg_clear` fills the entire pixel buffer with the given RGB colour. `bg_swap_buffers` pushes the buffer to the screen — call this once at the end of each frame.

---

### Drawing

```c
void bg_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void bg_draw_line(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b);
void bg_fill_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
void bg_draw_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
```

All coordinates are in screen pixels with the origin at the top-left. Out-of-bounds pixels are silently clipped. `bg_draw_line` uses Bresenham's algorithm. `bg_draw_rect` draws an outline; `bg_fill_rect` draws a solid rectangle.

---

### Text

```c
void bg_draw_text(int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b);
```

Renders a null-terminated string using the built-in 8×8 bitmap font. Each character occupies 9 pixels horizontally (8px glyph + 1px gap). Newlines (`\n`) are supported. Unknown characters are rendered as `?`.

The font data is exposed as a public array for direct use:

```c
extern const uint8_t bg_font8x8[95][8]; // indexed by (char - 32)
```

---

### Mouse Input

```c
void bg_get_mouse_pos(int *x, int *y);
int  bg_is_mouse_down(int button);
```

`bg_get_mouse_pos` writes the current cursor position into the provided pointers (either may be `NULL`). `bg_is_mouse_down` returns `1` while the button is held. Button indices: `0` = left, `1` = right, `2` = middle.

---

### Keyboard Input

```c
uint32_t bg_next_key(void);
int      bg_is_key_down(int keycode);
```

`bg_next_key` dequeues one key event per call from an internal 64-slot ring buffer, returning `BG_KEY_NONE` (0) when empty. Printable ASCII characters are returned as their character value (32–126). Special keys use the constants below.

`bg_is_key_down` polls the current physical state of a key by keycode (platform-specific; uses X11 KeySyms on Linux).

#### Key constants

| Constant | Value |
|---|---|
| `BG_KEY_NONE` | `0` |
| `BG_KEY_BACKSPACE` | `8` |
| `BG_KEY_TAB` | `9` |
| `BG_KEY_ENTER` | `13` |
| `BG_KEY_ESCAPE` | `27` |
| `BG_KEY_LEFT` | `0x10001` |
| `BG_KEY_RIGHT` | `0x10002` |
| `BG_KEY_UP` | `0x10003` |
| `BG_KEY_DOWN` | `0x10004` |
| `BG_KEY_DELETE` | `0x10005` |
| `BG_KEY_HOME` | `0x10006` |
| `BG_KEY_END` | `0x10007` |

---

### UI Widgets

#### Button

```c
int bg_draw_button(int x, int y, int w, int h, const char *label);
```

Draws a button with a centred label. Returns `1` while the left mouse button is held down inside the button area (i.e. it is being clicked). Handles hover highlight automatically.

```c
if (bg_draw_button(100, 200, 120, 30, "Click Me")) {
    // do something
}
```

#### Text Input Box

```c
#define BG_TEXTBOX_MAX 255

typedef struct {
    char     buf[BG_TEXTBOX_MAX + 1];
    int      len;
    int      focused;
    uint32_t cursor_timer;
} BgTextbox;

int bg_draw_textbox(int x, int y, int w, int h, BgTextbox *tb);
```

An immediate-mode text input box. The caller owns and zero-initialises the `BgTextbox` struct (e.g. `static BgTextbox tb;`). The box gains focus on click and loses it on Escape or clicking elsewhere. While focused, typed characters are appended to `tb.buf` and Backspace removes the last character. Long text scrolls to keep the cursor visible. Returns `1` on the frame Enter is pressed while the box is focused.

```c
static BgTextbox name_input;

// In your frame loop:
if (bg_draw_textbox(10, 50, 300, 24, &name_input)) {
    printf("Submitted: %s\n", name_input.buf);
}
```

---

## Full Example

```c
#define BITGFX_IMPLEMENTATION
#include "bitgfx.h"

int main(void) {
    bg_init(800, 600, "bitgfx demo");

    static BgTextbox tb;
    char last_input[BG_TEXTBOX_MAX + 1] = {0};
    int  counter = 0;

    while (!bg_should_close()) {
        bg_poll_events();
        bg_clear(20, 20, 30);

        // Handle key events
        uint32_t k;
        while ((k = bg_next_key()) != BG_KEY_NONE) {
            if (k == BG_KEY_ESCAPE) break;
        }

        // Draw some primitives
        bg_fill_rect(50, 50, 200, 100, 60, 80, 140);
        bg_draw_rect(50, 50, 200, 100, 160, 180, 255);
        bg_draw_line(0, 300, 800, 300, 80, 80, 100);

        // Text
        char info[64];
        int mx, my;
        bg_get_mouse_pos(&mx, &my);
        snprintf(info, sizeof(info), "Mouse: %d, %d", mx, my);
        bg_draw_text(10, 10, info, 200, 200, 200);

        // Button
        if (bg_draw_button(300, 200, 140, 32, "Count")) {
            counter++;
        }
        char cnt[32];
        snprintf(cnt, sizeof(cnt), "Count: %d", counter);
        bg_draw_text(300, 240, cnt, 255, 200, 100);

        // Text input
        bg_draw_text(10, 560, "Name:", 160, 160, 200);
        if (bg_draw_textbox(60, 556, 280, 22, &tb)) {
            snprintf(last_input, sizeof(last_input), "%s", tb.buf);
        }
        if (last_input[0]) {
            char greeting[BG_TEXTBOX_MAX + 16];
            snprintf(greeting, sizeof(greeting), "Hello, %s!", last_input);
            bg_draw_text(360, 560, greeting, 100, 220, 140);
        }

        bg_swap_buffers();
    }

    bg_terminate();
    return 0;
}
```

---

## Design Notes

**Pixel buffer layout.** The internal buffer is a flat `uint32_t` array, row-major, with the origin at the top-left. On Linux the format is `0x00RRGGBB`; on macOS it is `BGRX` (to match the `kCGBitmapByteOrder32Little` format expected by CoreGraphics). All drawing functions handle this transparently.

**macOS backend.** The macOS implementation uses no Objective-C source files and no `[]` message syntax. All Cocoa calls go through `objc_msgSend` cast to the appropriate function pointer type. A custom `NSView` subclass (`BITGFXView`) is registered at runtime via `objc_allocateClassPair`, and its `drawRect:` method calls `CGImageCreate` + `CGContextDrawImage` to blit the pixel buffer.

**Immediate-mode UI.** Widgets like `bg_draw_button` and `bg_draw_textbox` carry no hidden global state between calls beyond what you pass in. `bg_draw_button` does a live hit-test each frame. `bg_draw_textbox` requires a `BgTextbox` struct per instance, which you own — zero-initialise it once and pass a pointer every frame.

**Click-edge detection.** `bg__mouse_click` is set to `1` inside the `ButtonPress` event handler and cleared to `0` at the start of the next `bg_poll_events` call. This means any number of textboxes or widgets drawn in the same frame all see the same authoritative click event, avoiding the common bug where only the first widget drawn per frame responds to a click.

**Key queue.** A 64-slot ring buffer stores key events as they arrive. `bg_next_key` pops one event per call, making it easy to drain the queue in a loop without missing keystrokes on fast-typing frames. The queue silently drops events when full.

---

## License

MIT
