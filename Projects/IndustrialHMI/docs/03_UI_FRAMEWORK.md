# Phase 3: UI Framework Implementation

## 3.1 Graphics Primitives

### Color Definitions

```c
// ui_colors.h
#ifndef UI_COLORS_H
#define UI_COLORS_H

#include <stdint.h>

// RGB565 color format (16-bit)
#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3))

// Basic colors
#define COLOR_BLACK         0x0000
#define COLOR_WHITE         0xFFFF
#define COLOR_RED           0xF800
#define COLOR_GREEN         0x07E0
#define COLOR_BLUE          0x001F
#define COLOR_YELLOW        0xFFE0
#define COLOR_CYAN          0x07FF
#define COLOR_MAGENTA       0xF81F
#define COLOR_ORANGE        0xFD20
#define COLOR_GRAY          0x8410
#define COLOR_DARKGRAY      0x4208
#define COLOR_LIGHTGRAY     0xC618

// Industrial theme colors
#define COLOR_ALARM_RED     0xF800
#define COLOR_ALARM_YELLOW  0xFEA0
#define COLOR_OK_GREEN      0x07E0
#define COLOR_BACKGROUND    0x0000
#define COLOR_PANEL         0x2104
#define COLOR_BORDER        0x4208
#define COLOR_TEXT          0xFFFF
#define COLOR_TEXT_DIM      0x8410
#define COLOR_ACCENT        0x04FF
#define COLOR_BUTTON        0x2945
#define COLOR_BUTTON_PRESS  0x4A69

#endif // UI_COLORS_H
```

### Graphics API

```c
// ui_graphics.h
#ifndef UI_GRAPHICS_H
#define UI_GRAPHICS_H

#include "ui_colors.h"

// Basic drawing functions
void GFX_Init(void);
void GFX_Clear(uint16_t color);
void GFX_DrawPixel(int16_t x, int16_t y, uint16_t color);
void GFX_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void GFX_DrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void GFX_DrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void GFX_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void GFX_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void GFX_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void GFX_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void GFX_DrawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
void GFX_FillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
void GFX_DrawArc(int16_t x, int16_t y, int16_t r, int16_t startAngle, int16_t endAngle, uint16_t color);
void GFX_FillArc(int16_t x, int16_t y, int16_t r, int16_t startAngle, int16_t endAngle, uint16_t color);
void GFX_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void GFX_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

// Text functions
void GFX_SetFont(const uint8_t* font);
void GFX_SetTextColor(uint16_t fg, uint16_t bg);
void GFX_SetTextSize(uint8_t size);
void GFX_DrawChar(int16_t x, int16_t y, char c);
void GFX_DrawString(int16_t x, int16_t y, const char* str);
void GFX_DrawStringCentered(int16_t x, int16_t y, int16_t w, const char* str);
int16_t GFX_GetTextWidth(const char* str);
int16_t GFX_GetTextHeight(void);

// Image functions
void GFX_DrawBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h);
void GFX_DrawIcon(int16_t x, int16_t y, uint8_t iconId);

// Buffer management (for double buffering)
void GFX_SwapBuffers(void);
void GFX_SetDrawBuffer(uint8_t buffer);

#endif // UI_GRAPHICS_H
```

---

## 3.2 Widget System

### Base Widget Structure

```c
// ui_widget.h
#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <stdint.h>
#include <stdbool.h>

// Forward declaration
typedef struct UI_Widget UI_Widget_t;

// Widget types
typedef enum {
    WIDGET_NONE = 0,
    WIDGET_LABEL,
    WIDGET_BUTTON,
    WIDGET_GAUGE,
    WIDGET_CHART,
    WIDGET_LED,
    WIDGET_SLIDER,
    WIDGET_CHECKBOX,
    WIDGET_TEXTBOX,
    WIDGET_NUMPAD,
    WIDGET_KEYBOARD,
    WIDGET_IMAGE,
    WIDGET_PROGRESS,
    WIDGET_CONTAINER
} WidgetType_t;

// Widget states
typedef enum {
    WIDGET_STATE_NORMAL = 0,
    WIDGET_STATE_PRESSED,
    WIDGET_STATE_FOCUSED,
    WIDGET_STATE_DISABLED
} WidgetState_t;

// Event types
typedef enum {
    EVENT_NONE = 0,
    EVENT_PRESS,
    EVENT_RELEASE,
    EVENT_CLICK,
    EVENT_LONG_PRESS,
    EVENT_VALUE_CHANGED,
    EVENT_FOCUS_IN,
    EVENT_FOCUS_OUT
} EventType_t;

// Callback types
typedef void (*WidgetDrawFunc)(UI_Widget_t* widget);
typedef bool (*WidgetEventFunc)(UI_Widget_t* widget, EventType_t event, void* param);
typedef void (*WidgetUpdateFunc)(UI_Widget_t* widget);

// Base widget structure
struct UI_Widget {
    uint16_t id;                    // Unique widget ID
    WidgetType_t type;              // Widget type
    
    // Geometry
    int16_t x, y;                   // Position
    int16_t width, height;          // Size
    
    // State
    WidgetState_t state;
    bool visible;
    bool enabled;
    bool needsRedraw;
    
    // Style
    uint16_t bgColor;
    uint16_t fgColor;
    uint16_t borderColor;
    uint8_t borderWidth;
    uint8_t cornerRadius;
    
    // Data binding
    uint16_t tagId;                 // Bound data tag ID
    
    // Callbacks
    WidgetDrawFunc draw;
    WidgetEventFunc onEvent;
    WidgetUpdateFunc update;
    
    // User data
    void* userData;
    
    // Linked list
    UI_Widget_t* parent;
    UI_Widget_t* next;
    UI_Widget_t* children;          // For containers
};

// Widget API
UI_Widget_t* Widget_Create(WidgetType_t type, int16_t x, int16_t y, int16_t w, int16_t h);
void Widget_Destroy(UI_Widget_t* widget);
void Widget_SetVisible(UI_Widget_t* widget, bool visible);
void Widget_SetEnabled(UI_Widget_t* widget, bool enabled);
void Widget_SetPosition(UI_Widget_t* widget, int16_t x, int16_t y);
void Widget_SetSize(UI_Widget_t* widget, int16_t w, int16_t h);
void Widget_SetColors(UI_Widget_t* widget, uint16_t fg, uint16_t bg);
void Widget_BindTag(UI_Widget_t* widget, uint16_t tagId);
void Widget_Invalidate(UI_Widget_t* widget);
void Widget_Draw(UI_Widget_t* widget);
bool Widget_HandleTouch(UI_Widget_t* widget, int16_t x, int16_t y, bool pressed);

#endif // UI_WIDGET_H
```

---

## 3.3 Specific Widgets

### Button Widget

```c
// widget_button.h
typedef struct {
    UI_Widget_t base;
    char text[32];
    const uint8_t* icon;
    uint8_t iconId;
    bool toggle;
    bool toggled;
    void (*onClick)(UI_Widget_t* widget);
} Widget_Button_t;

Widget_Button_t* Widget_Button_Create(int16_t x, int16_t y, int16_t w, int16_t h, const char* text);
void Widget_Button_SetText(Widget_Button_t* btn, const char* text);
void Widget_Button_SetIcon(Widget_Button_t* btn, uint8_t iconId);
void Widget_Button_SetToggle(Widget_Button_t* btn, bool toggle);
void Widget_Button_SetCallback(Widget_Button_t* btn, void (*callback)(UI_Widget_t*));
```

### Gauge Widget

```c
// widget_gauge.h
typedef enum {
    GAUGE_STYLE_ARC,        // Arc/dial gauge
    GAUGE_STYLE_LINEAR_H,   // Horizontal bar
    GAUGE_STYLE_LINEAR_V,   // Vertical bar
    GAUGE_STYLE_NUMERIC     // Numeric display only
} GaugeStyle_t;

typedef struct {
    UI_Widget_t base;
    GaugeStyle_t style;
    float value;
    float minValue;
    float maxValue;
    float warningLow;
    float warningHigh;
    float alarmLow;
    float alarmHigh;
    char label[16];
    char unit[8];
    uint8_t decimals;
    bool showValue;
    bool showLabel;
    uint16_t colorNormal;
    uint16_t colorWarning;
    uint16_t colorAlarm;
} Widget_Gauge_t;

Widget_Gauge_t* Widget_Gauge_Create(int16_t x, int16_t y, int16_t w, int16_t h);
void Widget_Gauge_SetStyle(Widget_Gauge_t* gauge, GaugeStyle_t style);
void Widget_Gauge_SetValue(Widget_Gauge_t* gauge, float value);
void Widget_Gauge_SetRange(Widget_Gauge_t* gauge, float min, float max);
void Widget_Gauge_SetAlarmLimits(Widget_Gauge_t* gauge, float alarmLow, float alarmHigh);
void Widget_Gauge_SetWarningLimits(Widget_Gauge_t* gauge, float warnLow, float warnHigh);
void Widget_Gauge_SetLabel(Widget_Gauge_t* gauge, const char* label);
void Widget_Gauge_SetUnit(Widget_Gauge_t* gauge, const char* unit);
void Widget_Gauge_SetDecimals(Widget_Gauge_t* gauge, uint8_t decimals);
```

### Chart Widget

```c
// widget_chart.h
typedef enum {
    CHART_TYPE_LINE,
    CHART_TYPE_BAR,
    CHART_TYPE_AREA
} ChartType_t;

typedef struct {
    float* data;
    uint16_t count;
    uint16_t maxCount;
    uint16_t color;
    const char* name;
} ChartSeries_t;

typedef struct {
    UI_Widget_t base;
    ChartType_t type;
    ChartSeries_t series[4];    // Up to 4 series
    uint8_t seriesCount;
    float minY, maxY;
    bool autoScale;
    bool showGrid;
    bool showLegend;
    uint16_t gridColor;
    char titleX[16];
    char titleY[16];
} Widget_Chart_t;

Widget_Chart_t* Widget_Chart_Create(int16_t x, int16_t y, int16_t w, int16_t h);
void Widget_Chart_SetType(Widget_Chart_t* chart, ChartType_t type);
uint8_t Widget_Chart_AddSeries(Widget_Chart_t* chart, const char* name, uint16_t color);
void Widget_Chart_AddPoint(Widget_Chart_t* chart, uint8_t series, float value);
void Widget_Chart_SetYRange(Widget_Chart_t* chart, float min, float max);
void Widget_Chart_Clear(Widget_Chart_t* chart);
```

### LED Indicator Widget

```c
// widget_led.h
typedef enum {
    LED_SHAPE_CIRCLE,
    LED_SHAPE_SQUARE,
    LED_SHAPE_RECT
} LEDShape_t;

typedef struct {
    UI_Widget_t base;
    LEDShape_t shape;
    bool state;
    bool blinking;
    uint16_t blinkRate;         // ms
    uint16_t colorOn;
    uint16_t colorOff;
    char label[16];
} Widget_LED_t;

Widget_LED_t* Widget_LED_Create(int16_t x, int16_t y, int16_t size);
void Widget_LED_SetState(Widget_LED_t* led, bool state);
void Widget_LED_SetBlinking(Widget_LED_t* led, bool blink, uint16_t rate);
void Widget_LED_SetColors(Widget_LED_t* led, uint16_t colorOn, uint16_t colorOff);
void Widget_LED_SetLabel(Widget_LED_t* led, const char* label);
```

### Slider Widget

```c
// widget_slider.h
typedef struct {
    UI_Widget_t base;
    float value;
    float minValue;
    float maxValue;
    float step;
    bool horizontal;
    bool showValue;
    uint16_t trackColor;
    uint16_t fillColor;
    uint16_t knobColor;
    void (*onValueChanged)(UI_Widget_t* widget, float value);
} Widget_Slider_t;

Widget_Slider_t* Widget_Slider_Create(int16_t x, int16_t y, int16_t w, int16_t h, bool horizontal);
void Widget_Slider_SetValue(Widget_Slider_t* slider, float value);
void Widget_Slider_SetRange(Widget_Slider_t* slider, float min, float max);
void Widget_Slider_SetStep(Widget_Slider_t* slider, float step);
void Widget_Slider_SetCallback(Widget_Slider_t* slider, void (*callback)(UI_Widget_t*, float));
```

### Numeric Keypad Widget

```c
// widget_numpad.h
typedef struct {
    UI_Widget_t base;
    char value[16];
    uint8_t maxDigits;
    bool allowDecimal;
    bool allowNegative;
    void (*onConfirm)(UI_Widget_t* widget, float value);
    void (*onCancel)(UI_Widget_t* widget);
} Widget_Numpad_t;

Widget_Numpad_t* Widget_Numpad_Create(int16_t x, int16_t y);
void Widget_Numpad_SetValue(Widget_Numpad_t* numpad, float value);
float Widget_Numpad_GetValue(Widget_Numpad_t* numpad);
void Widget_Numpad_SetMaxDigits(Widget_Numpad_t* numpad, uint8_t digits);
void Widget_Numpad_SetCallbacks(Widget_Numpad_t* numpad, 
    void (*onConfirm)(UI_Widget_t*, float),
    void (*onCancel)(UI_Widget_t*));
```

---

## 3.4 Screen Manager

```c
// ui_screen.h
#ifndef UI_SCREEN_H
#define UI_SCREEN_H

#include "ui_widget.h"

#define MAX_SCREENS 16
#define SCREEN_STACK_SIZE 8

typedef struct UI_Screen UI_Screen_t;

typedef void (*ScreenInitFunc)(UI_Screen_t* screen);
typedef void (*ScreenEnterFunc)(UI_Screen_t* screen);
typedef void (*ScreenExitFunc)(UI_Screen_t* screen);
typedef void (*ScreenUpdateFunc)(UI_Screen_t* screen);
typedef void (*ScreenDestroyFunc)(UI_Screen_t* screen);

struct UI_Screen {
    uint8_t id;
    const char* name;
    UI_Widget_t* widgets;           // Linked list of widgets
    uint16_t bgColor;
    bool showHeader;
    bool showFooter;
    
    ScreenInitFunc init;
    ScreenEnterFunc onEnter;
    ScreenExitFunc onExit;
    ScreenUpdateFunc update;
    ScreenDestroyFunc destroy;
    
    void* userData;
};

// Screen Manager API
void ScreenManager_Init(void);
void ScreenManager_RegisterScreen(UI_Screen_t* screen);
void ScreenManager_SetHome(uint8_t screenId);
void ScreenManager_GoTo(uint8_t screenId);
void ScreenManager_Push(uint8_t screenId);
void ScreenManager_Pop(void);
void ScreenManager_GoHome(void);
UI_Screen_t* ScreenManager_GetCurrent(void);
void ScreenManager_Update(void);
void ScreenManager_Draw(void);
void ScreenManager_HandleTouch(int16_t x, int16_t y, bool pressed);

// Screen utilities
void Screen_AddWidget(UI_Screen_t* screen, UI_Widget_t* widget);
void Screen_RemoveWidget(UI_Screen_t* screen, UI_Widget_t* widget);
UI_Widget_t* Screen_FindWidget(UI_Screen_t* screen, uint16_t widgetId);
void Screen_InvalidateAll(UI_Screen_t* screen);

#endif // UI_SCREEN_H
```

---

## 3.5 Touch Handler

```c
// ui_touch.h
#ifndef UI_TOUCH_H
#define UI_TOUCH_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TOUCH_STATE_NONE,
    TOUCH_STATE_PRESSED,
    TOUCH_STATE_HELD,
    TOUCH_STATE_RELEASED,
    TOUCH_STATE_SWIPE_LEFT,
    TOUCH_STATE_SWIPE_RIGHT,
    TOUCH_STATE_SWIPE_UP,
    TOUCH_STATE_SWIPE_DOWN
} TouchState_t;

typedef struct {
    int16_t x;
    int16_t y;
    bool pressed;
    TouchState_t state;
    uint32_t pressTime;
    uint32_t holdTime;
    int16_t startX, startY;
    int16_t deltaX, deltaY;
} TouchData_t;

// Touch configuration
#define TOUCH_DEBOUNCE_MS       50
#define TOUCH_LONG_PRESS_MS     1000
#define TOUCH_SWIPE_THRESHOLD   50

// Touch API
void Touch_Init(void);
void Touch_Update(void);
TouchData_t* Touch_GetData(void);
bool Touch_IsPressed(void);
bool Touch_WasClicked(void);
bool Touch_IsLongPress(void);
TouchState_t Touch_GetGesture(void);

// Calibration
void Touch_StartCalibration(void);
bool Touch_IsCalibrating(void);
void Touch_SaveCalibration(void);
void Touch_LoadCalibration(void);

#endif // UI_TOUCH_H
```

---

## 3.6 Theme System

```c
// ui_theme.h
#ifndef UI_THEME_H
#define UI_THEME_H

typedef struct {
    const char* name;
    
    // Background colors
    uint16_t colorBackground;
    uint16_t colorPanel;
    uint16_t colorHeader;
    uint16_t colorFooter;
    
    // Text colors
    uint16_t colorText;
    uint16_t colorTextDim;
    uint16_t colorTextHighlight;
    
    // Button colors
    uint16_t colorButtonNormal;
    uint16_t colorButtonPressed;
    uint16_t colorButtonDisabled;
    uint16_t colorButtonText;
    
    // Status colors
    uint16_t colorOK;
    uint16_t colorWarning;
    uint16_t colorAlarm;
    uint16_t colorInfo;
    
    // Border/accent
    uint16_t colorBorder;
    uint16_t colorAccent;
    
    // Widget specific
    uint16_t colorGaugeNeedle;
    uint16_t colorGaugeScale;
    uint16_t colorChartGrid;
    uint16_t colorSliderTrack;
    uint16_t colorSliderKnob;
    
    // Fonts
    const uint8_t* fontSmall;
    const uint8_t* fontNormal;
    const uint8_t* fontLarge;
    const uint8_t* fontTitle;
    
    // Dimensions
    uint8_t borderRadius;
    uint8_t borderWidth;
    uint8_t headerHeight;
    uint8_t footerHeight;
    uint8_t padding;
    
} UI_Theme_t;

// Theme API
void Theme_Init(void);
void Theme_Set(const UI_Theme_t* theme);
const UI_Theme_t* Theme_Get(void);
const UI_Theme_t* Theme_GetBuiltIn(uint8_t index);
uint8_t Theme_GetCount(void);

// Built-in themes
extern const UI_Theme_t Theme_Industrial;
extern const UI_Theme_t Theme_Modern;
extern const UI_Theme_t Theme_Dark;
extern const UI_Theme_t Theme_Light;

#endif // UI_THEME_H
```

---

## 3.7 Implementation Steps

### Step 1: Graphics Layer (Day 6-7)
```
[ ] Implement GFX_Init() - initialize LTDC
[ ] Implement basic shapes (pixel, line, rect, circle)
[ ] Implement filled shapes
[ ] Implement arc drawing (for gauges)
[ ] Implement text rendering
[ ] Test with simple patterns
```

### Step 2: Widget Base (Day 8-9)
```
[ ] Implement Widget_Create/Destroy
[ ] Implement widget state management
[ ] Implement widget drawing framework
[ ] Implement touch hit testing
[ ] Create widget linked list management
```

### Step 3: Core Widgets (Day 10-11)
```
[ ] Implement Label widget
[ ] Implement Button widget
[ ] Implement LED indicator widget
[ ] Implement Gauge widget (arc style)
[ ] Test widgets with static values
```

### Step 4: Screen Manager (Day 12-13)
```
[ ] Implement screen registration
[ ] Implement screen navigation (push/pop/goto)
[ ] Implement screen transitions
[ ] Create header/footer framework
[ ] Test with multiple screens
```

### Step 5: Touch Integration (Day 14)
```
[ ] Integrate with touchscreen driver
[ ] Implement touch debouncing
[ ] Implement gesture detection
[ ] Wire touch events to widgets
[ ] Full integration test
```

---

## 3.8 Next Steps

1. ✅ UI framework designed
2. ➡️ Proceed to `04_COMMUNICATION.md` for protocol implementation
3. Create source files based on this design
4. Implement graphics layer first
5. Build widget library incrementally

---

## Checklist

- [ ] Graphics primitives implemented
- [ ] Base widget structure working
- [ ] Core widgets functional
- [ ] Screen manager operational
- [ ] Touch handling integrated
- [ ] Theme system working
