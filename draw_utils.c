#include "app.h"

HFONT create_app_font(int height, int weight, const wchar_t *face) {
    return CreateFontW(height, 0, 0, 0, weight, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, face);
}

void init_theme_objects(void) {
    const wchar_t *face_display = g_theme == THEME_LIGHT ? FONT_DISPLAY_LIGHT : FONT_DISPLAY_DARK;
    const wchar_t *face_body = g_theme == THEME_LIGHT ? FONT_BODY_LIGHT : FONT_BODY_DARK;
    int display_wt = g_theme == THEME_LIGHT ? FW_NORMAL : FW_SEMIBOLD;
    if (!g_font_display) g_font_display = create_app_font(FONT_SIZE_DISPLAY, display_wt, face_display);
    if (!g_font_title) g_font_title = create_app_font(FONT_SIZE_TITLE, FW_SEMIBOLD, face_body);
    if (!g_font_body) g_font_body = create_app_font(FONT_SIZE_BODY, FW_NORMAL, face_body);
    if (!g_font_body_bold) g_font_body_bold = create_app_font(FONT_SIZE_BODY, FW_SEMIBOLD, face_body);
    if (!g_font_caption) g_font_caption = create_app_font(FONT_SIZE_CAPTION, FW_NORMAL, face_body);
    if (!g_brush_bg_deep) g_brush_bg_deep = CreateSolidBrush(TC_BG_DEEPEST);
    if (!g_brush_bg_surface) g_brush_bg_surface = CreateSolidBrush(TC_BG_SURFACE);
    if (!g_brush_bg_card) g_brush_bg_card = CreateSolidBrush(TC_BG_CARD);
}

void destroy_theme_objects(void) {
    if (g_font_display) DeleteObject(g_font_display);
    if (g_font_title) DeleteObject(g_font_title);
    if (g_font_body) DeleteObject(g_font_body);
    if (g_font_body_bold) DeleteObject(g_font_body_bold);
    if (g_font_caption) DeleteObject(g_font_caption);
    if (g_brush_bg_deep) DeleteObject(g_brush_bg_deep);
    if (g_brush_bg_surface) DeleteObject(g_brush_bg_surface);
    if (g_brush_bg_card) DeleteObject(g_brush_bg_card);
    g_font_display = g_font_title = g_font_body = g_font_body_bold = g_font_caption = NULL;
    g_brush_bg_deep = g_brush_bg_surface = g_brush_bg_card = NULL;
}

float clamp01f(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

float ease_out_cubicf(float t) {
    float x = 1.0f - clamp01f(t);
    return 1.0f - x * x * x;
}

float ease_out_backf(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    float x = clamp01f(t) - 1.0f;
    return 1.0f + c3 * x * x * x + c1 * x * x;
}

COLORREF mix_color(COLORREF a, COLORREF b, float t) {
    float u = clamp01f(t);
    int ar = GetRValue(a), ag = GetGValue(a), ab = GetBValue(a);
    int br = GetRValue(b), bg = GetGValue(b), bb = GetBValue(b);
    int rr = (int)(ar + (br - ar) * u);
    int rg = (int)(ag + (bg - ag) * u);
    int rb = (int)(ab + (bb - ab) * u);
    return RGB(rr, rg, rb);
}

void fill_vertical_gradient(HDC hdc, const RECT *rc, COLORREF top, COLORREF bottom) {
    TRIVERTEX vert[2];
    GRADIENT_RECT grad = {0, 1};
    vert[0].x = rc->left;
    vert[0].y = rc->top;
    vert[0].Red = (COLOR16)(GetRValue(top) << 8);
    vert[0].Green = (COLOR16)(GetGValue(top) << 8);
    vert[0].Blue = (COLOR16)(GetBValue(top) << 8);
    vert[0].Alpha = 0xFF00;
    vert[1].x = rc->right;
    vert[1].y = rc->bottom;
    vert[1].Red = (COLOR16)(GetRValue(bottom) << 8);
    vert[1].Green = (COLOR16)(GetGValue(bottom) << 8);
    vert[1].Blue = (COLOR16)(GetBValue(bottom) << 8);
    vert[1].Alpha = 0xFF00;
    GradientFill(hdc, vert, 2, &grad, 1, GRADIENT_FILL_RECT_V);
}

void fill_round_gradient(HDC hdc, const RECT *rc, COLORREF top, COLORREF bottom, int radius) {
    int saved = SaveDC(hdc);
    HRGN clip = CreateRoundRectRgn(rc->left, rc->top, rc->right, rc->bottom, radius * 2, radius * 2);
    SelectClipRgn(hdc, clip);
    fill_vertical_gradient(hdc, rc, top, bottom);
    RestoreDC(hdc, saved);
    DeleteObject(clip);
}

void draw_rounded_rect(HDC hdc, const RECT *rc, COLORREF fill, COLORREF border, int radius) {
    HBRUSH br = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ old_br = SelectObject(hdc, br);
    HGDIOBJ old_pen = SelectObject(hdc, pen);
    RoundRect(hdc, rc->left, rc->top, rc->right, rc->bottom, radius * 2, radius * 2);

    SelectObject(hdc, old_br);
    SelectObject(hdc, old_pen);
    DeleteObject(br);
    DeleteObject(pen);

    if ((rc->right - rc->left) > 20 && (rc->bottom - rc->top) > 20) {
        HPEN hi = CreatePen(PS_SOLID, 1, mix_color(fill, TC_WHITE, 0.10f));
        HGDIOBJ old_hi_pen = SelectObject(hdc, hi);
        HGDIOBJ old_hi_br = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, rc->left + 1, rc->top + 1, rc->right - 1, rc->bottom - 1, (radius - 1) * 2, (radius - 1) * 2);
        SelectObject(hdc, old_hi_pen);
        SelectObject(hdc, old_hi_br);
        DeleteObject(hi);
    }
}

void draw_round_border(HDC hdc, const RECT *rc, COLORREF border, int radius, int pen_width) {
    HPEN pen = CreatePen(PS_SOLID, pen_width, border);
    HGDIOBJ old_pen = SelectObject(hdc, pen);
    HGDIOBJ old_br = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, rc->left, rc->top, rc->right, rc->bottom, radius * 2, radius * 2);
    SelectObject(hdc, old_pen);
    SelectObject(hdc, old_br);
    DeleteObject(pen);
}

void truncate_preview(wchar_t *dest, size_t cch_dest, const wchar_t *src, size_t max_chars) {
    size_t len;
    if (!dest || cch_dest == 0) return;
    if (!src) src = L"";
    wcsncpy_s(dest, cch_dest, src, _TRUNCATE);
    len = wcslen(dest);
    if (len > max_chars && max_chars > 3 && max_chars < cch_dest) {
        dest[max_chars - 3] = L'.';
        dest[max_chars - 2] = L'.';
        dest[max_chars - 1] = L'.';
        dest[max_chars] = 0;
    }
}

int button_is_primary(int id) {
    return id == BTN_PIN_CLIPBOARD || id == BTN_COPY_PIN || id == BTN_COPY_HISTORY || id == BTN_PIN_HISTORY;
}

void draw_button(const DRAWITEMSTRUCT *di) {
    RECT rc = di->rcItem;
    int id = di->CtlID;
    int primary = button_is_primary(id);
    COLORREF fill_top, fill_bottom, text, border;
    wchar_t text_buf[64];
    int radius = RADIUS_MD;
    int disabled = (di->itemState & ODS_DISABLED) != 0;
    int pressed = (di->itemState & ODS_SELECTED) != 0;
    int hot = (di->itemState & ODS_HOTLIGHT) != 0;

    if (primary) {
        fill_top = pressed ? TC_ACCENT_DEEP : TC_ACCENT;
        fill_bottom = pressed ? mix_color(TC_ACCENT_DEEP, TC_BG_ELEVATED, 0.3f) : mix_color(TC_ACCENT, TC_BG_ELEVATED, 0.2f);
        text = TC_WHITE;
        border = pressed ? TC_ACCENT_HOVER : mix_color(TC_ACCENT, TC_ACCENT_HOVER, 0.5f);
    } else {
        fill_top = pressed ? TC_BG_ELEVATED : TC_BG_OVERLAY;
        fill_bottom = pressed ? TC_BG_CARD : TC_BG_ELEVATED;
        text = hot ? TC_WHITE : TC_TEXT_PRIMARY;
        border = hot ? TC_BORDER_FOCUS : TC_BORDER_DEFAULT;
    }

    if (disabled) {
        fill_top = TC_BG_ELEVATED;
        fill_bottom = TC_BG_CARD;
        text = TC_TEXT_DISABLED;
        border = TC_BORDER_DEFAULT;
    }

    if (!pressed && !disabled) {
        RECT shadow = rc;
        OffsetRect(&shadow, 0, 1);
        draw_rounded_rect(di->hDC, &shadow, TC_BG_DEEP, TC_BG_DEEP, radius);
    }
    fill_round_gradient(di->hDC, &rc, fill_top, fill_bottom, radius);
    draw_round_border(di->hDC, &rc, border, radius, 1);

    if (!disabled) {
        COLORREF hi = primary ? TC_ACCENT_GLOW : mix_color(TC_TEXT_TERTIARY, TC_BG_OVERLAY, 0.5f);
        HPEN highlight = CreatePen(PS_SOLID, 1, hi);
        HGDIOBJ old_pen = SelectObject(di->hDC, highlight);
        MoveToEx(di->hDC, rc.left + radius, rc.top + 1, NULL);
        LineTo(di->hDC, rc.right - radius, rc.top + 1);
        SelectObject(di->hDC, old_pen);
        DeleteObject(highlight);
    }

    GetWindowTextW(di->hwndItem, text_buf, ARRAYSIZE(text_buf));
    SetBkMode(di->hDC, TRANSPARENT);
    SetTextColor(di->hDC, text);
    SelectObject(di->hDC, g_font_body_bold);
    DrawTextW(di->hDC, text_buf, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    if (di->itemState & ODS_FOCUS) {
        RECT focus = rc;
        InflateRect(&focus, -3, -3);
        draw_round_border(di->hDC, &focus, TC_BORDER_FOCUS, radius - 2, 2);
    }
}
