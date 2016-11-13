#include <pebble.h>

static Window *mainWindow;

static GBitmap *bmOverlay;
static GBitmap *bmBatt;
static GBitmap *bmBatt0;
static GBitmap *bmBatt1;
static GBitmap *bmBatt2;
static GBitmap *bmBatt3;
static GBitmap *bmBatt4;

static Layer *layerTime;

static BitmapLayer *bLayerOverlay;
static BitmapLayer *bLayerBatt;

static TextLayer *tLayerDate1;
static TextLayer *tLayerDate2;

GFont fontSquareM; 

static GColor colorHour;
static GColor colorTens;
static GColor colorOnes;

static int hourX[] = { 66, 81, 81, 81, 81, 66, 51, 36, 36, 36, 36, 51 };
static int hourY[] = { 44, 44, 59, 74, 89, 89, 89, 89, 74, 59, 44, 44 };
static int tensX[] = { 34, 70, 106, 106, 106 };
static int tensY[] = { 0, 0, 0, 36, 72 };
static int onesX[] = { 112, 84, 56, 28, 0, 0, 0, 0, 0 };
static int onesY[] = { 112, 112, 112, 112, 112, 84, 56, 28, 0 };

static void batt_layer_update() {
  BatteryChargeState charge = battery_state_service_peek();
  int battLvl = charge.charge_percent;
  
  if (battLvl > 90) {
    bitmap_layer_set_bitmap(bLayerBatt, bmBatt0);
  }
  else if (battLvl > 70) {
    bitmap_layer_set_bitmap(bLayerBatt, bmBatt1);
  }
  else if (battLvl > 40) {
    bitmap_layer_set_bitmap(bLayerBatt, bmBatt2);
  }
  else if (battLvl > 10) {
    bitmap_layer_set_bitmap(bLayerBatt, bmBatt3);
  }
  else {
    bitmap_layer_set_bitmap(bLayerBatt, bmBatt4);
  }
}

static void time_layer_update(Layer *layer, GContext *context) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int hour = tick_time->tm_hour % 12;
  if (hour == 0) { hour = 12; }
  int tens = tick_time->tm_min / 10;
  int ones = tick_time->tm_min % 10;

  static char dayOfMo[] = ".....";
  static char dayOfYr[] = "...";
  
  graphics_context_set_fill_color(context, colorHour);      
  for (int i = 0; i < hour; i++) {
    graphics_fill_rect(context, GRect(hourX[i], hourY[i], 15, 15), 0, GCornerNone);
  }
  graphics_context_set_fill_color(context, GColorWhite);      
  for (int i = hour; i < 12; i++) {
    graphics_fill_rect(context, GRect(hourX[i] + 5, hourY[i] + 5, 5, 5), 0, GCornerNone);
  }
  graphics_context_set_fill_color(context, colorTens);      
  for (int i = 0; i < tens; i++) {
    graphics_fill_rect(context, GRect(tensX[i], tensY[i], 36, 36), 0, GCornerNone);
  }
  graphics_context_set_fill_color(context, GColorWhite);      
  for (int i = tens; i < 5; i++) {
    graphics_fill_rect(context, GRect(tensX[i] + 12, tensY[i] + 12, 12, 12), 0, GCornerNone);
  }
  graphics_context_set_fill_color(context, colorOnes);      
  for (int i = 0; i < ones; i++) {
    graphics_fill_rect(context, GRect(onesX[i], onesY[i], 28, 28), 0, GCornerNone);
  }
  graphics_context_set_fill_color(context, GColorWhite);      
  for (int i = ones; i < 9; i++) {
    graphics_fill_rect(context, GRect(onesX[i] + 10, onesY[i] + 10, 8, 8), 0, GCornerNone);
  }
  
  strftime(dayOfMo, sizeof("....."), "%m.%d", tick_time);
  strftime(dayOfYr, sizeof("..."), "%j", tick_time);
  
  text_layer_set_text(tLayerDate1, dayOfMo);
  text_layer_set_text(tLayerDate2, dayOfYr);
  
  batt_layer_update();
}

static void time_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(layerTime);
}

static void bluetooth_handler(bool btConn) {
  if (btConn) vibes_short_pulse();
  else vibes_long_pulse();
}

static void battery_handler(BatteryChargeState charge) {
  batt_layer_update();
}

static void main_window_load_handler(Window *window) {
  window_set_background_color(mainWindow, GColorBlack);
  
  bmOverlay = gbitmap_create_with_resource(RESOURCE_ID_IMG_OVERLAY);

  bmBatt = gbitmap_create_with_resource(RESOURCE_ID_IMG_BATT);
  bmBatt0 = gbitmap_create_as_sub_bitmap(bmBatt, GRect(0, 0, 20, 20));
  bmBatt1 = gbitmap_create_as_sub_bitmap(bmBatt, GRect(20, 0, 20, 20));
  bmBatt2 = gbitmap_create_as_sub_bitmap(bmBatt, GRect(40, 0, 20, 20));
  bmBatt3 = gbitmap_create_as_sub_bitmap(bmBatt, GRect(60, 0, 20, 20));
  bmBatt4 = gbitmap_create_as_sub_bitmap(bmBatt, GRect(80, 0, 20, 20));
  
  fontSquareM = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQUARE_24));
  
  layerTime = layer_create(GRect(2, 2, 140, 140));
  layer_set_update_proc(layerTime, time_layer_update);
  
  bLayerOverlay = bitmap_layer_create(GRect(2, 2, 140, 140));
  bitmap_layer_set_background_color(bLayerOverlay, GColorClear);
  bitmap_layer_set_compositing_mode(bLayerOverlay, GCompOpSet);
  bitmap_layer_set_bitmap(bLayerOverlay, bmOverlay);
  
  bLayerBatt = bitmap_layer_create(GRect(58, 66, 20, 20));
  bitmap_layer_set_background_color(bLayerBatt, GColorClear);
  bitmap_layer_set_compositing_mode(bLayerBatt, GCompOpSet);
  bitmap_layer_set_bitmap(bLayerBatt, bmBatt0);
  
  tLayerDate1 = text_layer_create(GRect(2, 142, 94, 24));
  text_layer_set_text_color(tLayerDate1, GColorWhite);
  text_layer_set_background_color(tLayerDate1, GColorClear);
  text_layer_set_text_alignment(tLayerDate1, GTextAlignmentLeft);
  text_layer_set_font(tLayerDate1, fontSquareM);
  text_layer_set_text(tLayerDate1, "00.00");
  
  tLayerDate2 = text_layer_create(GRect(96, 142, 46, 24));
  text_layer_set_text_color(tLayerDate2, GColorWhite);
  text_layer_set_background_color(tLayerDate2, GColorClear);
  text_layer_set_text_alignment(tLayerDate2, GTextAlignmentRight);
  text_layer_set_font(tLayerDate2, fontSquareM);
  text_layer_set_text(tLayerDate2, "000");
  
  layer_add_child(window_get_root_layer(mainWindow), layerTime);
  layer_add_child(window_get_root_layer(mainWindow), bitmap_layer_get_layer(bLayerOverlay));
  layer_add_child(window_get_root_layer(mainWindow), bitmap_layer_get_layer(bLayerBatt));
  layer_add_child(window_get_root_layer(mainWindow), text_layer_get_layer(tLayerDate1));
  layer_add_child(window_get_root_layer(mainWindow), text_layer_get_layer(tLayerDate2));
}

static void main_window_unload_handler(Window *window) {
  gbitmap_destroy(bmOverlay);
  gbitmap_destroy(bmBatt);
  gbitmap_destroy(bmBatt0);
  gbitmap_destroy(bmBatt1);
  gbitmap_destroy(bmBatt2);
  gbitmap_destroy(bmBatt3);
  gbitmap_destroy(bmBatt4);
  fonts_unload_custom_font(fontSquareM);
  layer_destroy(layerTime);
  bitmap_layer_destroy(bLayerOverlay);
  bitmap_layer_destroy(bLayerBatt);
  text_layer_destroy(tLayerDate1);
  text_layer_destroy(tLayerDate2);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple* hrColor = dict_find(iter, MESSAGE_KEY_HOUR);
  if (hrColor) {
    colorHour = GColorFromHEX(hrColor->value->int32);
    persist_write_int(0, hrColor->value->int32);
  }
  Tuple* tnColor = dict_find(iter, MESSAGE_KEY_TENS);
  if (tnColor) {
    colorTens = GColorFromHEX(tnColor->value->int32);
    persist_write_int(1, tnColor->value->int32);
  }
  Tuple* onColor = dict_find(iter, MESSAGE_KEY_ONES);
  if (onColor) {
    colorOnes = GColorFromHEX(onColor->value->int32);
    persist_write_int(2, onColor->value->int32);
  }
  
  layer_mark_dirty(layerTime);
}

void init(void) {
  colorHour = persist_exists(0) ? GColorFromHEX(persist_read_int(0)) : GColorGreen;
  colorTens = persist_exists(1) ? GColorFromHEX(persist_read_int(1)) : GColorBlue;
  colorOnes = persist_exists(2) ? GColorFromHEX(persist_read_int(2)) : GColorRed;
  
  mainWindow = window_create();

  window_set_window_handlers(mainWindow, (WindowHandlers) {
    .load = main_window_load_handler,
    .unload = main_window_unload_handler
  });
  
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(128, 128);
  
  tick_timer_service_subscribe(MINUTE_UNIT, time_handler);
  bluetooth_connection_service_subscribe(bluetooth_handler);
  battery_state_service_subscribe(battery_handler);

  window_stack_push(mainWindow, true);
}

void deinit(void) {
  window_destroy(mainWindow);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
