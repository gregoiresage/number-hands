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
    GRect fill = bounds;

    GPoint center = grect_center_point(&bounds);

    graphics_context_set_stroke_color(ctx, GColorWhite);
    for(uint8_t i=0; i<60; i++){
      GPoint start_point  = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(5)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(i * 360 / 60));
      GPoint end_point    = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(7)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(i * 360 / 60));
      graphics_context_set_stroke_width(ctx, i % 15 == 0 ? 3 : 1);
      graphics_draw_line(ctx, start_point, end_point);
    }

    graphics_draw_text(ctx, txt_date, fonts_get_system_font(FONT_KEY_GOTHIC_14), (GRect){.origin={0, 125}, .size=bounds.size}, 0, GTextAlignmentCenter, 0);

    GPoint seconds_start_point = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(60)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE((seconds + 30)  * 360 / 60));
    GPoint seconds_end_point   = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(10)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(seconds  * 360 / 60));

    GPoint minutes_start_point = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(70)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE((minutes + 30) * 360 / 60));
    GPoint minutes_end_point   = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(15)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(minutes * 360 / 60));

    GPoint hours_start_point = center;
    GPoint hours_end_point   = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(40)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(((hours % 12) * 60 + minutes) * 360 / (24*60)));
    
    minutes_start_point.y -= 1;
    minutes_end_point.y   -= 1;
    minutes_start_point.x -= 1;
    minutes_end_point.x   -= 1;
    center.y -= 1;
    center.x -= 1;

    graphics_context_set_stroke_width(ctx, 15);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx, minutes_start_point, minutes_end_point);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, center, 13);

    minutes_start_point.y += 1;
    minutes_end_point.y   += 1;
    minutes_start_point.x += 1;
    minutes_end_point.x   += 1;
    center.y += 1;
    center.x += 1;

    graphics_context_set_stroke_width(ctx, 15);
    graphics_context_set_stroke_color(ctx, MINUTE_COLOR);
    graphics_draw_line(ctx, minutes_start_point, minutes_end_point);
    graphics_context_set_fill_color(ctx, MINUTE_COLOR);
    graphics_fill_circle(ctx, center, 13);


    hours_start_point.y += 1;
    hours_end_point.y   += 1;
    hours_start_point.x += 1;
    hours_end_point.x   += 1;

    graphics_context_set_stroke_width(ctx, 19);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_line(ctx, hours_start_point, hours_end_point);

    hours_start_point.y -= 1;
    hours_end_point.y   -= 1;
    hours_start_point.x -= 1;
    hours_end_point.x   -= 1;

    graphics_context_set_stroke_width(ctx, 19);
    graphics_context_set_stroke_color(ctx, HOUR_COLOR);
    graphics_draw_line(ctx, hours_start_point, hours_end_point);

    FContext fctx;
    fctx_init_context(&fctx, ctx);
    fctx_set_color_bias(&fctx, 0);

    fctx_begin_fill(&fctx);
    GPoint minutes_text_point = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(18)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(minutes * 360 / 60));
    ffcenter.x = INT_TO_FIXED(minutes_text_point.x);
    ffcenter.y = INT_TO_FIXED(minutes_text_point.y);
    fctx_set_offset(&fctx, ffcenter);
    fctx_set_fill_color(&fctx, GColorWhite);
    fctx_set_text_size(&fctx, font, 15);
    fctx_set_rotation(&fctx, DEG_TO_TRIGANGLE(minutes * 360 / 60  + (minutes < 30 ? - 90 : 90)));
    fctx_draw_string(&fctx, txt_minutes, font, GTextAlignmentCenter, FTextAnchorMiddle);
    fctx_end_fill(&fctx);

    fctx_begin_fill(&fctx);
    GPoint hours_text_point = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(42)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(((hours % 12) * 60 + minutes) * 360 / (24*60)));
    ffcenter.x = INT_TO_FIXED(hours_text_point.x);
    ffcenter.y = INT_TO_FIXED(hours_text_point.y);
    fctx_set_offset(&fctx, ffcenter);
    fctx_set_fill_color(&fctx, GColorBlack);
    fctx_set_text_size(&fctx, font, 19);
    fctx_set_rotation(&fctx, DEG_TO_TRIGANGLE(((hours % 12) * 60 + minutes) * 360 / (24*60) + (hours % 12 < 6 ? - 90 : 90)));
    fctx_draw_string(&fctx, txt_hours, font, GTextAlignmentCenter, FTextAnchorMiddle);
    fctx_end_fill(&fctx);

    fctx_deinit_context(&fctx);

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
    graphics_fill_circle(ctx, center, 2);
}

static void window_load(Window *window) {
  Layer* window_layer = window_get_root_layer(window);
  GRect window_frame = layer_get_frame(window_layer);

  font = ffont_create_from_resource(RESOURCE_ID_DIN_FONT);

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