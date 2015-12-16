#include "pebble.h"
#include "fctx/fctx.h"
#include "fctx/ffont.h"

#include "autoconfig.h"

static Window* window;
static Layer* layer;
static FFont* font;
static FPoint ffcenter;

static AppTimer *upTimer = NULL;
static AppTimer *lightTimer = NULL;
static bool wasUp = false;

static Animation* startSecondAnimation;
static Animation* mainSecondAnimation;
static Animation* endSecondAnimation;
static Animation* fullSecondAnimation;

static AnimationImplementation implementation;

static uint32_t secondStartAngle = 0;
static uint32_t secondEndAngle = 0;
static uint32_t secondAngle = 0;

static int hours = 0;
static int minutes = 0;
static int seconds = 0;
static char txt_minutes[3] = "00";
static char txt_hours[3]   = "00";
static char txt_date[10]   = "000000000";

typedef enum {
  BG_COLOR = 0,
  HOUR_COLOR,
  MINUTE_COLOR,
  SECOND_COLOR,
  NUM_COLORS
} Colors;

const uint8_t THEME_WHITE[NUM_COLORS] = {GColorWhiteARGB8, GColorLightGrayARGB8, GColorBlackARGB8, GColorRedARGB8 };
const uint8_t THEME_BLACK[NUM_COLORS] = {GColorBlackARGB8, GColorLightGrayARGB8, GColorWhiteARGB8, GColorRedARGB8 };
const uint8_t THEME_BLUE[NUM_COLORS]  = {GColorVividCeruleanARGB8, GColorDukeBlueARGB8, GColorBlueMoonARGB8, GColorRedARGB8 };
const uint8_t THEME_RED[NUM_COLORS]   = {GColorRedARGB8, GColorBlackARGB8, GColorLightGrayARGB8, GColorGreenARGB8 };
const uint8_t THEME_PINK[NUM_COLORS]  = {GColorMagentaARGB8, GColorImperialPurpleARGB8, GColorBulgarianRoseARGB8, GColorDarkCandyAppleRedARGB8 };

static GColor colors[NUM_COLORS];

#define HOUR_WITDH  18
#define HOUR_HEIGHT 57

#define MINUTE_WITDH        16
#define MINUTE_HEIGHT_UP    78
#define MINUTE_HEIGHT_DOWN  20

static FPoint HOURS_PATH_INFO[4];
static FPoint MINUTES_PATH_INFO[4];

static void handle_time_tick(struct tm *tick_time, TimeUnits units_changed) {
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

  graphics_context_set_stroke_color(ctx, gcolor_legible_over(colors[BG_COLOR]));
  graphics_context_set_text_color(ctx, gcolor_legible_over(colors[BG_COLOR]));
  for(uint8_t i=0; i<60; i++){
    GPoint start_point  = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(5)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(i * 360 / 60));
    GPoint end_point    = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(8)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(i * 360 / 60));
    graphics_context_set_stroke_width(ctx, i % 15 == 0 ? 3 : 1);
    graphics_draw_line(ctx, start_point, end_point);
  }

  if(getShow_date()){
    graphics_draw_text(ctx, txt_date, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect){.origin={0, 125}, .size=bounds.size}, 0, GTextAlignmentCenter, 0);
  }
  
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
  fctx_set_fill_color(&fctx, colors[MINUTE_COLOR]);

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
  fctx_set_fill_color(&fctx, colors[HOUR_COLOR]);

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
  if(getShow_numbers()){
    GPoint minutes_text_point = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(20)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE((minutes * 60 + seconds) * 360 / (60 * 60)));
    ffcenter.x = INT_TO_FIXED(minutes_text_point.x);
    ffcenter.y = INT_TO_FIXED(minutes_text_point.y);
  
    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_set_fill_color(&fctx, gcolor_legible_over(colors[MINUTE_COLOR]));
    fctx_set_text_size(&fctx, font, 15);
    fctx_set_rotation(&fctx, DEG_TO_TRIGANGLE((minutes * 60 + seconds) * 360 / (60 * 60)  + (minutes < 30 ? - 90 : 90)));
    fctx_draw_string(&fctx, txt_minutes, font, GTextAlignmentCenter, FTextAnchorMiddle);
    fctx_end_fill(&fctx);
  
    GPoint hours_text_point = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(42)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(((hours % 12) * 60 + minutes) * 360 / (12*60)));
    ffcenter.x = INT_TO_FIXED(hours_text_point.x);
    ffcenter.y = INT_TO_FIXED(hours_text_point.y);
  
    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, ffcenter);
    fctx_set_fill_color(&fctx, gcolor_legible_over(colors[HOUR_COLOR]));
    fctx_set_text_size(&fctx, font, 18);
    fctx_set_rotation(&fctx, DEG_TO_TRIGANGLE(((hours % 12) * 60 + minutes) * 360 / (12*60) + (hours % 12 < 6 ? - 90 : 90)));
    fctx_draw_string(&fctx, txt_hours, font, GTextAlignmentCenter, FTextAnchorMiddle);
    fctx_end_fill(&fctx);
  }

  fctx_deinit_context(&fctx);

  // draw seconds hand
  if(getSecond_duration() != SECOND_DURATION_NO){
    if(getSecond_duration() == SECOND_DURATION_ALWAYS){
      secondAngle = DEG_TO_TRIGANGLE(seconds  * 360 / 60);
    }

    GPoint seconds_start_point = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(60)), GOvalScaleModeFitCircle, secondAngle + TRIG_MAX_ANGLE/2);
    GPoint seconds_end_point   = gpoint_from_polar(grect_inset(bounds, GEdgeInsets1(10)), GOvalScaleModeFitCircle, secondAngle);

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
    graphics_context_set_stroke_color(ctx, colors[SECOND_COLOR]);
    graphics_draw_line(ctx, seconds_start_point, seconds_end_point);
    graphics_context_set_fill_color(ctx, colors[SECOND_COLOR]);
    graphics_fill_circle(ctx, center, 4);
  }

  graphics_context_set_fill_color(ctx, colors[BG_COLOR]);
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

static void lightoff_cb(void *ctx){
  lightTimer = NULL;
  light_enable(false);
}

static void lighton(){
  uint32_t timeout = 0;
  switch(getBacklight_duration()){
    case BACKLIGHT_DURATION_3  : timeout = 3000; break;
    case BACKLIGHT_DURATION_5  : timeout = 5000; break;
    case BACKLIGHT_DURATION_10 : timeout = 10000; break;
    case BACKLIGHT_DURATION_NO : return;
    default : timeout = 3000; break;
  }
  light_enable(true);
  if(lightTimer){
    app_timer_cancel(lightTimer);
  }
  lightTimer = app_timer_register(timeout, lightoff_cb, NULL);
}

static void animationUpdate(struct Animation *animation, const AnimationProgress time_normalized){
  if(animation == startSecondAnimation){
    secondAngle = secondStartAngle * time_normalized / ANIMATION_NORMALIZED_MAX;
  }
  else if(animation == endSecondAnimation){
    secondAngle = secondEndAngle + (TRIG_MAX_ANGLE - secondEndAngle) * time_normalized / ANIMATION_NORMALIZED_MAX;
  }
  else if(animation == mainSecondAnimation){
    secondAngle = secondStartAngle + (secondEndAngle - secondStartAngle) * time_normalized / ANIMATION_NORMALIZED_MAX;
  }

  layer_mark_dirty(layer);
}

void animation_stopped(Animation *animation, bool finished, void *data) {
  fullSecondAnimation = NULL;
}

static void showseconds(){
  if(!fullSecondAnimation && getSecond_duration() != SECOND_DURATION_ALWAYS && getSecond_duration() != SECOND_DURATION_NO){
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    secondStartAngle = DEG_TO_TRIGANGLE(t->tm_sec * 360 / 60);

    switch(getSecond_duration()){
      case SECOND_DURATION_5:  secondEndAngle = DEG_TO_TRIGANGLE((t->tm_sec + 5) * 360 / 60); break;
      case SECOND_DURATION_10: secondEndAngle = DEG_TO_TRIGANGLE((t->tm_sec + 10) * 360 / 60); break;
      case SECOND_DURATION_15: secondEndAngle = DEG_TO_TRIGANGLE((t->tm_sec + 15) * 360 / 60); break;
      case SECOND_DURATION_30: secondEndAngle = DEG_TO_TRIGANGLE((t->tm_sec + 30) * 360 / 60); break;
      default: secondEndAngle = secondStartAngle;
    }


    startSecondAnimation = animation_create();
    animation_set_duration(startSecondAnimation, 1000);
    animation_set_curve(startSecondAnimation, AnimationCurveEaseIn);
    animation_set_implementation(startSecondAnimation, &implementation);

    mainSecondAnimation = animation_create();
    switch(getSecond_duration()){
      case SECOND_DURATION_5:  animation_set_duration(mainSecondAnimation, 5000); break;
      case SECOND_DURATION_10: animation_set_duration(mainSecondAnimation, 10000); break;
      case SECOND_DURATION_15: animation_set_duration(mainSecondAnimation, 15000); break;
      case SECOND_DURATION_30: animation_set_duration(mainSecondAnimation, 30000); break;
      default: ;
    }
    animation_set_curve(mainSecondAnimation, AnimationCurveLinear);
    animation_set_implementation(mainSecondAnimation, &implementation);

    endSecondAnimation = animation_create();
    animation_set_curve(endSecondAnimation, AnimationCurveEaseIn);
    animation_set_duration(endSecondAnimation, 1000);
    animation_set_implementation(endSecondAnimation, &implementation);

    fullSecondAnimation = animation_sequence_create(startSecondAnimation, mainSecondAnimation, endSecondAnimation, NULL);
    animation_set_handlers(fullSecondAnimation, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler) animation_stopped,
    }, NULL);
    animation_schedule(fullSecondAnimation);
  }
}

static void up_cb(void *ctx){
  upTimer = NULL;
  wasUp = true;
  lighton();
  showseconds();
}

static void accelDataHandler(AccelData *data, uint32_t num_samples){
  if(!wasUp && !upTimer && data->y < -450){
    upTimer = app_timer_register(500, up_cb, NULL);
  }
  else if(data->y > -450) {
    if(upTimer) {
      app_timer_cancel(upTimer);
      upTimer = NULL;
    }
    if(lightTimer){
      app_timer_cancel(lightTimer);
      lightTimer = NULL;
    }
    wasUp = false;
    lightoff_cb(NULL);
  }
}

static void updateSettings(){
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(getSecond_duration() == SECOND_DURATION_ALWAYS ? SECOND_UNIT : MINUTE_UNIT, handle_time_tick); 

  const uint8_t *theme = NULL;
  switch(getColor_theme()){
    case COLOR_THEME_WHITE :  theme = THEME_WHITE;  break;
    case COLOR_THEME_BLACK :  theme = THEME_BLACK;  break;
    case COLOR_THEME_BLUE :   theme = THEME_BLUE;   break;
    case COLOR_THEME_RED :    theme = THEME_RED;    break;
    case COLOR_THEME_PINK :   theme = THEME_PINK;   break;
    default : break;
  }
  for(uint8_t i=0; i<NUM_COLORS; i++){
    colors[i] = (GColor8){.argb=theme[i]};
  }
  window_set_background_color(window, colors[BG_COLOR]);

  accel_data_service_unsubscribe();
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
  accel_data_service_subscribe(1, accelDataHandler);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  autoconfig_in_received_handler(iter, context);
  updateSettings();
}

static void init() {
  autoconfig_init(200, 0);

  app_message_register_inbox_received(in_received_handler);

  implementation.update = animationUpdate;

  setlocale(LC_ALL, "");
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  updateSettings();
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  handle_time_tick(t, SECOND_UNIT);
}

static void deinit() {
  autoconfig_deinit();
  tick_timer_service_unsubscribe();

  if(lightTimer){
    app_timer_cancel(lightTimer);
  }
  if(upTimer){
    app_timer_cancel(upTimer);
  }
}

int main() {
  init();
  app_event_loop();
  deinit();
}