#include "pebble.h"
#include "fctx/fctx.h"
#include "fctx/ffont.h"

static Window* window;
static Layer* layer;
static FFont* font;
static FPoint ffcenter;

static int hours = 0;
static int minutes = 0;
static int seconds = 0;
static char txt_minutes[3] = "00";
static char txt_hours[3]   = "00";
static char txt_date[10]   = "000000000";

#define BG_COLOR GColorDarkGray
#define HOUR_COLOR GColorLightGray
#define MINUTE_COLOR GColorBlack
#define SECOND_COLOR GColorVividCerulean

#define HOUR_WITDH  18
#define HOUR_HEIGHT 57

#define MINUTE_WITDH        16
#define MINUTE_HEIGHT_UP    78
#define MINUTE_HEIGHT_DOWN  20

static FPoint HOURS_PATH_INFO[4];
static FPoint MINUTES_PATH_INFO[4];

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  hours = tick_time->tm_hour;
  minutes = tick_time->tm_min;
  seconds = tick_time->tm_sec;
  strftime(txt_date,    sizeof(txt_date),     "%d %a",  tick_time);
  snprintf(txt_hours,   sizeof(txt_hours),    "%02d",   hours);
  snprintf(txt_minutes, sizeof(txt_minutes),  "%02d",   minutes);
  if(layer)
    layer_mark_dirty(layer);
}

static void update_layer(Layer* layer, GContext* ctx) {

    GRect bounds = layer_get_bounds(layer);
    GPoint center = grect_center_point(&bounds);

    graphics_context_set_stroke_color(ctx, GColorWhite);
    for(uint8_t i=0; i<60; i++){
      GPoint start_point  = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(5)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(i * 360 / 60));
      GPoint end_point    = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(8)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(i * 360 / 60));
      graphics_context_set_stroke_width(ctx, i % 15 == 0 ? 3 : 1);
      graphics_draw_line(ctx, start_point, end_point);
    }

    graphics_draw_text(ctx, txt_date, fonts_get_system_font(FONT_KEY_GOTHIC_14), (GRect){.origin={0, 125}, .size=bounds.size}, 0, GTextAlignmentCenter, 0);

    GPoint seconds_start_point = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(60)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE((seconds + 30)  * 360 / 60));
    GPoint seconds_end_point   = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(10)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(seconds  * 360 / 60));

    FContext fctx;
    fctx_init_context(&fctx, ctx);
    fctx_set_color_bias(&fctx, 0);

    // draw minutes hand

    // draw shadow
    ffcenter.x = INT_TO_FIXED(center.x - 1);
    ffcenter.y = INT_TO_FIXED(center.y - 1);
    fctx_set_fill_color(&fctx, GColorWhite);

    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_plot_circle(&fctx, &ffcenter, INT_TO_FIXED(13));
    fctx_end_fill(&fctx);

    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_set_rotation(&fctx, DEG_TO_TRIGANGLE((minutes * 60 + seconds) * 360 / (60 * 60)));
    fctx_draw_path(&fctx, MINUTES_PATH_INFO, 4);
    fctx_end_fill(&fctx);

    // draw hand
    ffcenter.x = INT_TO_FIXED(center.x);
    ffcenter.y = INT_TO_FIXED(center.y);
    fctx_set_fill_color(&fctx, MINUTE_COLOR);

    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_plot_circle(&fctx, &ffcenter, INT_TO_FIXED(13));
    fctx_end_fill(&fctx);

    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_set_rotation(&fctx, DEG_TO_TRIGANGLE((minutes * 60 + seconds) * 360 / (60 * 60)));
    fctx_draw_path(&fctx, MINUTES_PATH_INFO, 4);
    fctx_end_fill(&fctx);

    // draw hours hand

    // draw shadow
    ffcenter.x = INT_TO_FIXED(center.x + 1);
    ffcenter.y = INT_TO_FIXED(center.y + 1);
    fctx_set_fill_color(&fctx, GColorBlack);

    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_plot_circle(&fctx, &ffcenter, INT_TO_FIXED(HOUR_WITDH/2));
    fctx_end_fill(&fctx);

    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_set_rotation(&fctx, DEG_TO_TRIGANGLE(((hours % 12) * 60 + minutes) * 360 / (12*60)));
    fctx_draw_path(&fctx, HOURS_PATH_INFO, 4);
    fctx_end_fill(&fctx);

    // draw hand
    ffcenter.x = INT_TO_FIXED(center.x);
    ffcenter.y = INT_TO_FIXED(center.y);
    fctx_set_fill_color(&fctx, HOUR_COLOR);

    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_plot_circle(&fctx, &ffcenter, INT_TO_FIXED(HOUR_WITDH/2));
    fctx_end_fill(&fctx);

    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_set_rotation(&fctx, DEG_TO_TRIGANGLE(((hours % 12) * 60 + minutes) * 360 / (12*60)));
    fctx_draw_path(&fctx, HOURS_PATH_INFO, 4);
    fctx_end_fill(&fctx);

    // Draw hours/minutes text

    GPoint minutes_text_point = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(20)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE((minutes * 60 + seconds) * 360 / (60 * 60)));
    ffcenter.x = INT_TO_FIXED(minutes_text_point.x);
    ffcenter.y = INT_TO_FIXED(minutes_text_point.y);

    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_set_fill_color(&fctx, GColorWhite);
    fctx_set_text_size(&fctx, font, 15);
    fctx_set_rotation(&fctx, DEG_TO_TRIGANGLE((minutes * 60 + seconds) * 360 / (60 * 60)  + (minutes < 30 ? - 90 : 90)));
    fctx_draw_string(&fctx, txt_minutes, font, GTextAlignmentCenter, FTextAnchorMiddle);
    fctx_end_fill(&fctx);

    GPoint hours_text_point = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(42)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(((hours % 12) * 60 + minutes) * 360 / (12*60)));
    ffcenter.x = INT_TO_FIXED(hours_text_point.x);
    ffcenter.y = INT_TO_FIXED(hours_text_point.y);

    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_set_fill_color(&fctx, GColorBlack);
    fctx_set_text_size(&fctx, font, 18);
    fctx_set_rotation(&fctx, DEG_TO_TRIGANGLE(((hours % 12) * 60 + minutes) * 360 / (12*60) + (hours % 12 < 6 ? - 90 : 90)));
    fctx_draw_string(&fctx, txt_hours, font, GTextAlignmentCenter, FTextAnchorMiddle);
    fctx_end_fill(&fctx);

    fctx_deinit_context(&fctx);

    // draw seconds hand

    seconds_start_point.y += 1;
    seconds_end_point.y   += 1;

    graphics_context_set_stroke_width(ctx, 3);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_line(ctx, seconds_start_point, seconds_end_point);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, center, 4);

    seconds_start_point.y -= 1;
    seconds_end_point.y   -= 1;

    graphics_context_set_stroke_width(ctx, 3);
    graphics_context_set_stroke_color(ctx, SECOND_COLOR);
    graphics_draw_line(ctx, seconds_start_point, seconds_end_point);
    graphics_context_set_fill_color(ctx, SECOND_COLOR);
    graphics_fill_circle(ctx, center, 4);

    graphics_context_set_fill_color(ctx, BG_COLOR);
    graphics_fill_circle(ctx, center, 1);
}

static void window_load(Window *window) {
  Layer* window_layer = window_get_root_layer(window);
  GRect window_frame = layer_get_frame(window_layer);

  font = ffont_create_from_resource(RESOURCE_ID_DIN_FONT);

  HOURS_PATH_INFO[0] = FPointI(-HOUR_WITDH / 2, 0);
  HOURS_PATH_INFO[1] = FPointI(-HOUR_WITDH / 2, -HOUR_HEIGHT);
  HOURS_PATH_INFO[2] = FPointI(HOUR_WITDH / 2, -HOUR_HEIGHT);
  HOURS_PATH_INFO[3] = FPointI(HOUR_WITDH / 2, 0);
  
  MINUTES_PATH_INFO[0] = FPointI(-MINUTE_WITDH / 2, MINUTE_HEIGHT_DOWN);
  MINUTES_PATH_INFO[1] = FPointI(-MINUTE_WITDH / 2, -MINUTE_HEIGHT_UP);
  MINUTES_PATH_INFO[2] = FPointI(MINUTE_WITDH / 2, -MINUTE_HEIGHT_UP);
  MINUTES_PATH_INFO[3] = FPointI(MINUTE_WITDH / 2, MINUTE_HEIGHT_DOWN);

  layer = layer_create(window_frame);
  layer_set_update_proc(layer, update_layer);
  layer_add_child(window_layer, layer);
}

static void window_unload(Window *window) {
  window_destroy(window);
  layer_destroy(layer);
  ffont_destroy(font);
}

static void init() {
    setlocale(LC_ALL, "");
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_background_color(window, BG_COLOR);
    window_stack_push(window, true);
    
    tick_timer_service_subscribe(SECOND_UNIT, handle_minute_tick);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    handle_minute_tick(t, SECOND_UNIT);
}

static void deinit() {
}

int main() {
    init();
    app_event_loop();
    deinit();
}