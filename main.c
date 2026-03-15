/*
 * main.c  —  PNG Image Editor  (bitgfx.h)
 *
 * Features:
 *   - New canvas (width × height)
 *   - Open PNG  (libpng)
 *   - Save PNG  (libpng)
 *   - Brush, Eraser, Eyedropper tools
 *   - Brush size + opacity sliders
 *   - RGB colour textboxes + 20-colour palette
 *   - Pan  (middle-mouse drag)
 *   - Zoom (+ / - buttons, or scroll implied by buttons)
 *   - RGBA canvas with checkerboard transparency preview
 *   - Undo (up to 32 levels)
 *
 * Build — Linux:
 *   gcc -O2 -o editor main.c -lX11 -lm -I/usr/include/libpng16 -lpng
 *
 * Build — macOS (Homebrew libpng):
 *   LIBPNG=$(brew --prefix libpng)
 *   clang -O2 -o editor main.c \
 *       -framework Cocoa -framework CoreGraphics -lobjc -lm \
 *       -I$LIBPNG/include -L$LIBPNG/lib -lpng
 */

#define BITGFX_IMPLEMENTATION
#include "bitgfx.h"
/* png.h location varies by platform/install. Use pkg-config in your
   build command (see below) to get the right -I flag automatically.  */
#ifdef __has_include
#  if __has_include(<png.h>)
#    include <png.h>
#  elif __has_include(<libpng16/png.h>)
#    include <libpng16/png.h>
#  elif __has_include(<libpng/png.h>)
#    include <libpng/png.h>
#  else
#    error "libpng not found. Install libpng-dev (Linux) or libpng (Homebrew/MacPorts)."
#  endif
#else
#  include <png.h>
#endif
#include <math.h>

/* =========================================================================
 * Window layout
 * ========================================================================= */
#define WIN_W   1200
#define WIN_H    750
#define PANEL_W  260   /* left tool panel */
#define VIEW_X   PANEL_W
#define VIEW_W   (WIN_W - PANEL_W)
#define VIEW_H   WIN_H

/* =========================================================================
 * Canvas (RGBA, dynamically allocated)
 * ========================================================================= */
static uint8_t *canvas     = NULL;   /* row-major RGBA */
static int      canvas_w   = 0;
static int      canvas_h   = 0;

#define CANVAS_PIXEL(x,y)  (canvas + ((y)*canvas_w + (x))*4)

static void canvas_alloc(int w, int h) {
    free(canvas);
    canvas   = (uint8_t*)calloc((size_t)(w * h * 4), 1);
    canvas_w = w;
    canvas_h = h;
}

static void canvas_fill_white(void) {
    for (int i = 0; i < canvas_w * canvas_h; i++) {
        canvas[i*4+0] = 255;
        canvas[i*4+1] = 255;
        canvas[i*4+2] = 255;
        canvas[i*4+3] = 255;
    }
}

/* =========================================================================
 * Undo stack
 * ========================================================================= */
#define UNDO_LEVELS 32
static uint8_t *undo_stack[UNDO_LEVELS];
static int      undo_top = 0;   /* next write position */
static int      undo_count = 0;

static void undo_push(void) {
    if (!canvas) return;
    size_t sz = (size_t)(canvas_w * canvas_h * 4);
    free(undo_stack[undo_top]);
    undo_stack[undo_top] = (uint8_t*)malloc(sz);
    if (undo_stack[undo_top]) memcpy(undo_stack[undo_top], canvas, sz);
    undo_top = (undo_top + 1) % UNDO_LEVELS;
    if (undo_count < UNDO_LEVELS) undo_count++;
}

static void undo_pop(void) {
    if (undo_count == 0 || !canvas) return;
    undo_top = (undo_top + UNDO_LEVELS - 1) % UNDO_LEVELS;
    if (undo_stack[undo_top]) {
        memcpy(canvas, undo_stack[undo_top], (size_t)(canvas_w*canvas_h*4));
        free(undo_stack[undo_top]);
        undo_stack[undo_top] = NULL;
    }
    undo_count--;
}

/* =========================================================================
 * PNG I/O  (libpng)
 * ========================================================================= */
static char status_msg[512] = "New 800x600 canvas. Open a PNG or start drawing.";

static int load_png(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        snprintf(status_msg, sizeof(status_msg), "Cannot open: %s", path);
        return 0;
    }
    uint8_t sig[8];
    if (fread(sig, 1, 8, fp) != 8 || png_sig_cmp(sig, 0, 8)) {
        snprintf(status_msg, sizeof(status_msg), "Not a PNG: %s", path);
        fclose(fp); return 0;
    }
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop   info = png_create_info_struct(png);
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        snprintf(status_msg, sizeof(status_msg), "PNG read error: %s", path);
        return 0;
    }
    png_init_io(png, fp);
    png_set_sig_bytes(png, 8);
    png_read_info(png, info);

    int w = (int)png_get_image_width(png, info);
    int h = (int)png_get_image_height(png, info);
    png_byte color = png_get_color_type(png, info);
    png_byte depth = png_get_bit_depth(png, info);

    if (depth == 16) png_set_strip_16(png);
    if (color == PNG_COLOR_TYPE_PALETTE)    png_set_palette_to_rgb(png);
    if (color == PNG_COLOR_TYPE_GRAY && depth < 8) png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
    if (color == PNG_COLOR_TYPE_RGB  ||
        color == PNG_COLOR_TYPE_GRAY ||
        color == PNG_COLOR_TYPE_PALETTE) png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if (color == PNG_COLOR_TYPE_GRAY ||
        color == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png);
    png_read_update_info(png, info);

    canvas_alloc(w, h);
    uint8_t **rows = (uint8_t**)malloc((size_t)h * sizeof(uint8_t*));
    for (int y = 0; y < h; y++) rows[y] = CANVAS_PIXEL(0, y);
    png_read_image(png, rows);
    free(rows);
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
    snprintf(status_msg, sizeof(status_msg), "Opened: %s  (%dx%d)", path, w, h);
    return 1;
}

static int save_png(const char *path) {
    if (!canvas) return 0;
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        snprintf(status_msg, sizeof(status_msg), "Cannot write: %s", path);
        return 0;
    }
    png_structp png  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop   info = png_create_info_struct(png);
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        snprintf(status_msg, sizeof(status_msg), "PNG write error: %s", path);
        return 0;
    }
    png_init_io(png, fp);
    png_set_IHDR(png, info, (png_uint_32)canvas_w, (png_uint_32)canvas_h,
                 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    uint8_t **rows = (uint8_t**)malloc((size_t)canvas_h * sizeof(uint8_t*));
    for (int y = 0; y < canvas_h; y++) rows[y] = CANVAS_PIXEL(0, y);
    png_write_image(png, rows);
    free(rows);
    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    snprintf(status_msg, sizeof(status_msg), "Saved: %s", path);
    return 1;
}

/* =========================================================================
 * Painting
 * ========================================================================= */
static void paint_pixel(int x, int y,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (x < 0 || x >= canvas_w || y < 0 || y >= canvas_h) return;
    uint8_t *d = CANVAS_PIXEL(x, y);
    uint32_t ia = 255 - a;
    d[0] = (uint8_t)((r*a + d[0]*ia) / 255);
    d[1] = (uint8_t)((g*a + d[1]*ia) / 255);
    d[2] = (uint8_t)((b*a + d[2]*ia) / 255);
    d[3] = (uint8_t)(a + (uint32_t)d[3]*ia / 255);
}

static void erase_pixel(int x, int y) {
    if (x < 0 || x >= canvas_w || y < 0 || y >= canvas_h) return;
    memset(CANVAS_PIXEL(x, y), 0, 4);
}

static void paint_circle(int cx, int cy, int r,
                          uint8_t pr, uint8_t pg, uint8_t pb, uint8_t pa,
                          int erasing)
{
    for (int dy = -r; dy <= r; dy++)
        for (int dx = -r; dx <= r; dx++)
            if (dx*dx + dy*dy <= r*r) {
                if (erasing) erase_pixel(cx+dx, cy+dy);
                else         paint_pixel(cx+dx, cy+dy, pr, pg, pb, pa);
            }
}

static void paint_line(int x0, int y0, int x1, int y1, int r,
                        uint8_t pr, uint8_t pg, uint8_t pb, uint8_t pa,
                        int erasing)
{
    int dx=abs(x1-x0), dy=abs(y1-y0);
    int sx=x0<x1?1:-1, sy=y0<y1?1:-1, err=dx-dy;
    for(;;) {
        paint_circle(x0,y0,r,pr,pg,pb,pa,erasing);
        if(x0==x1&&y0==y1) break;
        int e2=err*2;
        if(e2>-dy){err-=dy;x0+=sx;}
        if(e2< dx){err+=dx;y0+=sy;}
    }
}

/* =========================================================================
 * UI helpers
 * ========================================================================= */
static int clamp_i(int v, int lo, int hi) {
    return v<lo?lo:(v>hi?hi:v);
}

static void sep(int y) {
    bg_draw_line(6, y, PANEL_W-6, y, 50, 50, 70);
}

static void sec(int *py, const char *t) {
    *py += 4; sep(*py); *py += 6;
    bg_draw_text(8, *py, t, 130, 165, 215);
    *py += 13;
}

static int slider(int x, int y, int w, int h,
                  int val, int lo, int hi,
                  const char *lbl,
                  uint8_t fr, uint8_t fg, uint8_t fb)
{
    bg_fill_rect(x, y, w, h, 32, 32, 48);
    bg_draw_rect(x, y, w, h, 65, 65, 90);
    int fill = (hi>lo) ? (val-lo)*(w-2)/(hi-lo) : 0;
    bg_fill_rect(x+1, y+1, fill, h-2, fr, fg, fb);
    char buf[32]; snprintf(buf,sizeof(buf),"%s %d",lbl,val);
    bg_draw_text(x+4, y+(h-8)/2, buf, 220, 220, 230);
    int mx,my; bg_get_mouse_pos(&mx,&my);
    if (mx>=x&&mx<x+w&&my>=y&&my<y+h&&bg_is_mouse_down(0)) {
        int nv=lo+(mx-x)*(hi-lo)/(w-1);
        if(nv<lo) nv=lo;
        if(nv>hi) nv=hi;
        return nv;
    }
    return val;
}

/* =========================================================================
 * Viewport (pan + zoom)
 * ========================================================================= */
static float  zoom      = 1.0f;
static int    pan_x     = 0;    /* canvas pixel at view top-left */
static int    pan_y     = 0;

/* Convert view pixel → canvas pixel */
static int view_to_canvas_x(int vx) {
    return (int)((vx - VIEW_X) / zoom) + pan_x;
}
static int view_to_canvas_y(int vy) {
    return (int)(vy / zoom) + pan_y;
}

/* Convert canvas pixel → view pixel */
static int canvas_to_view_x(int cx) {
    return (int)((cx - pan_x) * zoom) + VIEW_X;
}
static int canvas_to_view_y(int cy) {
    return (int)((cy - pan_y) * zoom);
}

static void clamp_pan(void) {
    if (!canvas) return;
    int max_px = (int)(canvas_w - VIEW_W / zoom);
    int max_py = (int)(canvas_h - VIEW_H / zoom);
    if (max_px < 0) max_px = 0;
    if (max_py < 0) max_py = 0;
    if (pan_x < 0)       pan_x = 0;
    if (pan_y < 0)       pan_y = 0;
    if (pan_x > max_px)  pan_x = max_px;
    if (pan_y > max_py)  pan_y = max_py;
}

/* Zoom keeping the cursor position fixed */
static void zoom_at(float new_zoom, int vx, int vy) {
    if (new_zoom < 0.1f) new_zoom = 0.1f;
    if (new_zoom > 32.f) new_zoom = 32.f;
    int cx = view_to_canvas_x(vx);
    int cy = view_to_canvas_y(vy);
    zoom  = new_zoom;
    pan_x = cx - (int)((vx - VIEW_X) / zoom);
    pan_y = cy - (int)(vy / zoom);
    clamp_pan();
}

/* =========================================================================
 * Render canvas into the view area
 * ========================================================================= */
static void render_canvas(void) {
    if (!canvas) return;

    /* Visible canvas rect */
    int c_x0 = pan_x, c_y0 = pan_y;
    int c_x1 = c_x0 + (int)(VIEW_W / zoom) + 1;
    int c_y1 = c_y0 + (int)(VIEW_H / zoom) + 1;
    if (c_x0 < 0) c_x0 = 0;
    if (c_y0 < 0) c_y0 = 0;
    if (c_x1 > canvas_w) c_x1 = canvas_w;
    if (c_y1 > canvas_h) c_y1 = canvas_h;

    int cell = (int)zoom;
    if (cell < 1) cell = 1;

    for (int cy = c_y0; cy < c_y1; cy++) {
        for (int cx = c_x0; cx < c_x1; cx++) {
            int vx = canvas_to_view_x(cx);
            int vy = canvas_to_view_y(cy);
            int vw = canvas_to_view_x(cx+1) - vx;
            int vh = canvas_to_view_y(cy+1) - vy;
            if (vw < 1) vw = 1;
            if (vh < 1) vh = 1;

            uint8_t *p = CANVAS_PIXEL(cx, cy);
            uint8_t r = p[0], g = p[1], b = p[2], a = p[3];

            /* Checkerboard for transparency */
            uint8_t bg_c = ((cx ^ cy) & 1) ? 200 : 160;
            uint8_t dr = (uint8_t)((r*a + bg_c*(255-a))/255);
            uint8_t dg = (uint8_t)((g*a + bg_c*(255-a))/255);
            uint8_t db = (uint8_t)((b*a + bg_c*(255-a))/255);

            bg_fill_rect(vx, vy, vw, vh, dr, dg, db);
        }
    }

    /* Canvas border */
    int bx0 = canvas_to_view_x(0);
    int by0 = canvas_to_view_y(0);
    int bx1 = canvas_to_view_x(canvas_w);
    int by1 = canvas_to_view_y(canvas_h);
    bg_draw_rect(bx0-1, by0-1, bx1-bx0+2, by1-by0+2, 80, 80, 120);

    /* Pixel grid when zoomed in enough */
    if (zoom >= 8.0f) {
        for (int cx = c_x0; cx <= c_x1; cx++) {
            int vx = canvas_to_view_x(cx);
            bg_draw_line(vx, by0, vx, by1, 40, 40, 60);
        }
        for (int cy = c_y0; cy <= c_y1; cy++) {
            int vy = canvas_to_view_y(cy);
            bg_draw_line(bx0, vy, bx1, vy, 40, 40, 60);
        }
    }
}

/* =========================================================================
 * New canvas dialog state
 * ========================================================================= */
static int show_new_dialog = 0;
static BgTextbox tb_new_w, tb_new_h;

/* =========================================================================
 * Quick palette
 * ========================================================================= */
static const uint8_t PAL[20][3] = {
    {  0,  0,  0},{255,255,255},{128,128,128},{ 64, 64, 64},
    {220, 50, 50},{ 50,180, 50},{ 50,100,220},{220,220, 40},
    {  0,255,255},{255,  0,255},{255,128,  0},{128,  0,255},
    {  0,128,  0},{128,  0,  0},{  0,  0,128},{ 64,200,120},
    {200, 80, 60},{255,200,100},{ 80,180,220},{180, 80,200},
};

/* =========================================================================
 * main
 * ========================================================================= */
int main(void) {
    if (!bg_init(WIN_W, WIN_H, "PNG Editor")) {
        fprintf(stderr, "bg_init failed\n"); return 1;
    }

    /* Default canvas */
    canvas_alloc(800, 600);
    canvas_fill_white();

    /* Initial zoom to fit */
    zoom  = fminf((float)VIEW_W / canvas_w, (float)VIEW_H / canvas_h) * 0.9f;
    pan_x = 0; pan_y = 0;

    /* Tool state */
    typedef enum { TOOL_BRUSH, TOOL_ERASER, TOOL_EYEDROP } Tool;
    Tool    tool       = TOOL_BRUSH;
    uint8_t brush_r    = 0, brush_g = 0, brush_b = 0, brush_a = 255;
    int     brush_size = 5;

    /* Textboxes — drawn before key-log */
    static BgTextbox tb_r, tb_g, tb_b, tb_open, tb_save;
    snprintf(tb_r.buf,   sizeof(tb_r.buf),   "0");          tb_r.len   = 1;
    snprintf(tb_g.buf,   sizeof(tb_g.buf),   "0");          tb_g.len   = 1;
    snprintf(tb_b.buf,   sizeof(tb_b.buf),   "0");          tb_b.len   = 1;
    snprintf(tb_open.buf,sizeof(tb_open.buf),"image.png");  tb_open.len= 9;
    snprintf(tb_save.buf,sizeof(tb_save.buf),"output.png"); tb_save.len=10;
    snprintf(tb_new_w.buf,sizeof(tb_new_w.buf),"800");      tb_new_w.len=3;
    snprintf(tb_new_h.buf,sizeof(tb_new_h.buf),"600");      tb_new_h.len=3;

    /* Pan state */
    int panning    = 0;
    int pan_start_mx = 0, pan_start_my = 0;
    int pan_start_px = 0, pan_start_py = 0;

    /* Stroke tracking for smooth lines */
    int prev_cx = -1, prev_cy = -1;
    int stroke_started = 0;

    extern int bg__mouse_click;

    while (!bg_should_close()) {
        bg_poll_events();

        int mx, my;
        bg_get_mouse_pos(&mx, &my);
        int on_view = (mx >= VIEW_X);

        /* ---- Panning (middle mouse) ---- */
        if (bg_is_mouse_down(1) && on_view) {
            if (!panning) {
                panning      = 1;
                pan_start_mx = mx; pan_start_my = my;
                pan_start_px = pan_x; pan_start_py = pan_y;
            } else {
                pan_x = pan_start_px - (int)((mx - pan_start_mx) / zoom);
                pan_y = pan_start_py - (int)((my - pan_start_my) / zoom);
                clamp_pan();
            }
        } else {
            panning = 0;
        }

        /* ---- Keyboard shortcuts ---- */
        {
            uint32_t k;
            while ((k = bg_next_key()) != BG_KEY_NONE) {
                if (k == 'z' || k == 'Z') undo_pop();
                if (k == '+' || k == '=') zoom_at(zoom * 1.25f, (VIEW_X+WIN_W)/2, WIN_H/2);
                if (k == '-')             zoom_at(zoom / 1.25f, (VIEW_X+WIN_W)/2, WIN_H/2);
                if (k == 'b' || k == 'B') tool = TOOL_BRUSH;
                if (k == 'e' || k == 'E') tool = TOOL_ERASER;
                if (k == 'i' || k == 'I') tool = TOOL_EYEDROP;
            }
        }

        /* ---- Panel background ---- */
        bg_fill_rect(0, 0, PANEL_W, WIN_H, 22, 22, 34);
        bg_draw_line(PANEL_W-1, 0, PANEL_W-1, WIN_H, 55, 55, 80);

        int py = 8;
        bg_draw_text(8, py, "PNG  EDITOR", 180, 210, 255); py += 14;
        if (canvas)  {
            char dim[40];
            snprintf(dim,sizeof(dim),"%d x %d px", canvas_w, canvas_h);
            bg_draw_text(8, py, dim, 100, 120, 160);
        }
        py += 14;

        /* ---- File section ---- */
        sec(&py, "FILE");

        bg_draw_text(8, py, "Open:", 150,150,190); py += 11;
        bg_draw_textbox(8, py, PANEL_W-16, 18, &tb_open); py += 22;
        if (bg_draw_button(8, py, PANEL_W-16, 20, "Open PNG")) {
            if (load_png(tb_open.buf)) {
                zoom  = fminf((float)VIEW_W/canvas_w,(float)VIEW_H/canvas_h)*0.9f;
                pan_x = 0; pan_y = 0;
                undo_count = 0;
            }
        }
        py += 26;

        bg_draw_text(8, py, "Save:", 150,150,190); py += 11;
        bg_draw_textbox(8, py, PANEL_W-16, 18, &tb_save); py += 22;
        if (bg_draw_button(8, py, PANEL_W-16, 20, "Save PNG"))
            save_png(tb_save.buf);
        py += 26;

        /* New canvas */
        if (bg_draw_button(8, py, PANEL_W-16, 20, "New Canvas..."))
            show_new_dialog = !show_new_dialog;
        py += 26;

        if (show_new_dialog) {
            bg_draw_text(8, py, "W:", 180,180,200);
            bg_draw_textbox(22, py, 80, 18, &tb_new_w); 
            bg_draw_text(108, py, "H:", 180,180,200);
            bg_draw_textbox(122, py, 80, 18, &tb_new_h);
            py += 24;
            if (bg_draw_button(8, py, PANEL_W-16, 20, "Create")) {
                int nw = clamp_i(atoi(tb_new_w.buf), 1, 8192);
                int nh = clamp_i(atoi(tb_new_h.buf), 1, 8192);
                canvas_alloc(nw, nh);
                canvas_fill_white();
                zoom  = fminf((float)VIEW_W/nw,(float)VIEW_H/nh)*0.9f;
                pan_x = 0; pan_y = 0;
                undo_count = 0;
                show_new_dialog = 0;
                snprintf(status_msg,sizeof(status_msg),"New %dx%d canvas.",nw,nh);
            }
            py += 26;
        }

        /* ---- Tools ---- */
        sec(&py, "TOOLS  (B / E / I)");
        {
            struct { const char *n; Tool t; } tools[] = {
                {"Brush (B)",   TOOL_BRUSH},
                {"Eraser (E)",  TOOL_ERASER},
                {"Eyedrop (I)", TOOL_EYEDROP},
            };
            for (int i = 0; i < 3; i++) {
                if (bg_draw_button(8, py, PANEL_W-16, 20, tools[i].n))
                    tool = tools[i].t;
                if (tool == tools[i].t)
                    bg_draw_rect(8, py, PANEL_W-16, 20, 120, 180, 255);
                py += 24;
            }
        }

        /* ---- Brush controls ---- */
        sec(&py, "BRUSH");
        brush_size = slider(8, py, PANEL_W-16, 18, brush_size, 1, 100,
                            "Size", 100,160,255);
        py += 24;
        brush_a = (uint8_t)slider(8, py, PANEL_W-16, 18, (int)brush_a,
                                  0, 255, "Opacity", 180,100,220);
        py += 24;

        /* ---- Colour ---- */
        sec(&py, "COLOUR");

        /* Textboxes FIRST — consume keys before anything else */
        bg_draw_text(8, py+4, "R", 220,80,80);
        bg_draw_textbox(20, py, PANEL_W-28, 18, &tb_r); py += 22;
        bg_draw_text(8, py+4, "G", 80,220,80);
        bg_draw_textbox(20, py, PANEL_W-28, 18, &tb_g); py += 22;
        bg_draw_text(8, py+4, "B", 80,80,220);
        bg_draw_textbox(20, py, PANEL_W-28, 18, &tb_b); py += 22;

        /* Live update from textboxes */
        brush_r = (uint8_t)clamp_i(atoi(tb_r.buf), 0, 255);
        brush_g = (uint8_t)clamp_i(atoi(tb_g.buf), 0, 255);
        brush_b = (uint8_t)clamp_i(atoi(tb_b.buf), 0, 255);

        /* Colour preview */
        bg_fill_rect(8, py, PANEL_W-16, 16, brush_r, brush_g, brush_b);
        bg_draw_rect(8, py, PANEL_W-16, 16, 90,90,120);
        py += 22;

        /* ---- Palette ---- */
        sec(&py, "PALETTE");
        {
            int cols=5, sw=(PANEL_W-16)/cols;
            for (int i=0;i<20;i++) {
                int sx=8+(i%cols)*sw, sy=py+(i/cols)*(sw+2);
                bg_fill_rect(sx,sy,sw-2,sw-2,PAL[i][0],PAL[i][1],PAL[i][2]);
                if (PAL[i][0]==brush_r&&PAL[i][1]==brush_g&&PAL[i][2]==brush_b)
                    bg_draw_rect(sx,sy,sw-2,sw-2,255,255,255);
                else
                    bg_draw_rect(sx,sy,sw-2,sw-2,48,48,68);
                int hit=mx>=sx&&mx<sx+sw-2&&my>=sy&&my<sy+sw-2;
                if (hit && bg__mouse_click) {
                    brush_r=PAL[i][0]; brush_g=PAL[i][1]; brush_b=PAL[i][2];
                    snprintf(tb_r.buf,sizeof(tb_r.buf),"%d",brush_r); tb_r.len=(int)strlen(tb_r.buf);
                    snprintf(tb_g.buf,sizeof(tb_g.buf),"%d",brush_g); tb_g.len=(int)strlen(tb_g.buf);
                    snprintf(tb_b.buf,sizeof(tb_b.buf),"%d",brush_b); tb_b.len=(int)strlen(tb_b.buf);
                }
            }
            py += (20/cols+1)*(sw+2)+4;
        }

        /* ---- Zoom ---- */
        sec(&py, "VIEW");
        {
            char zbuf[20]; snprintf(zbuf,sizeof(zbuf),"%.0f%%",(double)zoom*100.0);
            bg_draw_text(8, py+6, zbuf, 180,180,200);
            if (bg_draw_button(80,  py, 40, 22, "+"))
                zoom_at(zoom*1.25f,(VIEW_X+WIN_W)/2,WIN_H/2);
            if (bg_draw_button(124, py, 40, 22, "-"))
                zoom_at(zoom/1.25f,(VIEW_X+WIN_W)/2,WIN_H/2);
            if (bg_draw_button(168, py, PANEL_W-176, 22, "Fit")) {
                if (canvas) {
                    zoom  = fminf((float)VIEW_W/canvas_w,(float)VIEW_H/canvas_h)*0.9f;
                    pan_x = 0; pan_y = 0;
                }
            }
            py += 28;
        }

        /* Undo button */
        if (bg_draw_button(8, py, PANEL_W-16, 20, "Undo  (Z)")) undo_pop();
        py += 26;

        /* Status bar */
        bg_draw_line(4, WIN_H-20, PANEL_W-4, WIN_H-20, 50,50,70);
        bg_draw_text(6, WIN_H-14, status_msg, 130,200,130);

        /* ================================================================
         * View area
         * ================================================================ */
        bg_fill_rect(VIEW_X, 0, VIEW_W, VIEW_H, 40, 40, 50);
        render_canvas();

        /* ================================================================
         * Mouse interaction on canvas
         * ================================================================ */
        if (canvas && on_view && !panning) {
            int cx = view_to_canvas_x(mx);
            int cy = view_to_canvas_y(my);
            int on_canvas = (cx>=0 && cx<canvas_w && cy>=0 && cy<canvas_h);

            /* Draw brush cursor */
            if (on_canvas) {
                int cur_r  = canvas_to_view_x(cx + brush_size) - canvas_to_view_x(cx);
                int scr_cx = canvas_to_view_x(cx);
                int scr_cy = canvas_to_view_y(cy);
                if (tool == TOOL_EYEDROP) {
                    /* Crosshair */
                    bg_draw_line(mx-6, my, mx+6, my, 255,255,255);
                    bg_draw_line(mx, my-6, mx, my+6, 255,255,255);
                } else {
                    uint8_t cr = (tool==TOOL_ERASER)?220:brush_r;
                    uint8_t cg = (tool==TOOL_ERASER)?80:brush_g;
                    uint8_t cb = (tool==TOOL_ERASER)?80:brush_b;
                    bg_draw_rect(scr_cx - cur_r, scr_cy - cur_r,
                                 cur_r*2+1, cur_r*2+1, cr,cg,cb);
                }

                /* Pixel info */
                uint8_t *p = CANVAS_PIXEL(cx, cy);
                char info[64];
                snprintf(info,sizeof(info),"[%d,%d] R%d G%d B%d A%d",
                         cx,cy,p[0],p[1],p[2],p[3]);
                bg_draw_text(VIEW_X+4, WIN_H-14, info, 160,180,220);
            }

            /* Paint / erase / pick */
            if (bg_is_mouse_down(0) && on_canvas) {
                if (tool == TOOL_EYEDROP) {
                    uint8_t *p = CANVAS_PIXEL(cx, cy);
                    brush_r=p[0]; brush_g=p[1]; brush_b=p[2];
                    snprintf(tb_r.buf,sizeof(tb_r.buf),"%d",brush_r); tb_r.len=(int)strlen(tb_r.buf);
                    snprintf(tb_g.buf,sizeof(tb_g.buf),"%d",brush_g); tb_g.len=(int)strlen(tb_g.buf);
                    snprintf(tb_b.buf,sizeof(tb_b.buf),"%d",brush_b); tb_b.len=(int)strlen(tb_b.buf);
                    tool = TOOL_BRUSH;
                    snprintf(status_msg,sizeof(status_msg),"Picked: R%d G%d B%d",brush_r,brush_g,brush_b);
                } else {
                    if (!stroke_started) {
                        undo_push();
                        stroke_started = 1;
                    }
                    if (prev_cx >= 0)
                        paint_line(prev_cx, prev_cy, cx, cy, brush_size,
                                   brush_r, brush_g, brush_b, brush_a,
                                   tool==TOOL_ERASER);
                    else
                        paint_circle(cx, cy, brush_size,
                                     brush_r, brush_g, brush_b, brush_a,
                                     tool==TOOL_ERASER);
                    prev_cx = cx; prev_cy = cy;
                }
            } else {
                prev_cx = -1; prev_cy = -1;
                stroke_started = 0;
            }
        } else {
            prev_cx = -1; prev_cy = -1;
            if (!bg_is_mouse_down(0)) stroke_started = 0;
        }

        /* New-canvas dialog overlay */
        if (show_new_dialog) {
            bg_fill_rect(VIEW_X+20, WIN_H/2-40, 300, 80, 30,30,45);
            bg_draw_rect(VIEW_X+20, WIN_H/2-40, 300, 80, 100,100,140);
            bg_draw_text(VIEW_X+30, WIN_H/2-28,
                         "Width / Height set in panel ->", 200,200,220);
        }

        bg_swap_buffers();
    }

    free(canvas);
    bg_terminate();
    return 0;
}