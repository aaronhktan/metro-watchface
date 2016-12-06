#include <pebble.h>

static Window *s_main_window;
static Layer *s_window_layer, *s_foreground_layer, *s_background_layer, *s_train_layer;
static int s_train;
static char s_time_text[6] = "00:00", s_battery_text[5] = "100%", s_heart_rate_text[8] = "200 BPM";

static void background_update_proc(Layer *s_background_layer, GContext *ctx) {
}

static void train_update(Animation *anim, AnimationProgress dist_normalized) {
}

static void foreground_update_proc(Layer *s_foreground_layer, GContext *ctx) {
	// Set bounds of window
	GRect bounds = layer_get_bounds(s_window_layer);
	
	// Set colour to black
	graphics_context_set_text_color(ctx, GColorBlack);
	
	// Draw time text
	GSize time_text_bounds = graphics_text_layout_get_content_size("24:00", fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS),
																																 GRect(0, 0, bounds.size.w, bounds.size.h),
																																 GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_time_text, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS), 
										 GRect((bounds.size.w - time_text_bounds.w) / 2, bounds.size.h / 2 - time_text_bounds.h / 2, time_text_bounds.w, time_text_bounds.h),
										 GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	
	// Draw battery text
	GSize battery_text_bounds = graphics_text_layout_get_content_size("100%", fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS),
																																 GRect(0, 0, bounds.size.w, bounds.size.h),
																																 GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_battery_text, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LECO_20)), 
										 GRect((bounds.size.w - battery_text_bounds.w) / 2, bounds.size.h / 2 - battery_text_bounds.h / 2 - time_text_bounds.h / 2 + 8, battery_text_bounds.w, battery_text_bounds.h),
										 GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	
	// Draw heart rate text
	GSize heart_rate_text_bounds = graphics_text_layout_get_content_size("200 BPM", fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS),
																																 GRect(0, 0, bounds.size.w, bounds.size.h),
																																 GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_heart_rate_text, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LECO_20)), 
										 GRect((bounds.size.w - heart_rate_text_bounds.w) / 2, bounds.size.h / 2 + time_text_bounds.h / 2, heart_rate_text_bounds.w, heart_rate_text_bounds.h),
										 GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

static void update_ui() {
	time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  strftime(s_time_text, sizeof(s_time_text), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  #if PBL_API_EXISTS(health_service_peek_current_value)
    HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
		snprintf(s_heart_rate_text, sizeof(s_heart_rate_text), "%lu BPM", (uint32_t) value);
  #endif

  BatteryChargeState battery_info = battery_state_service_peek();

  snprintf(s_battery_text, sizeof(s_battery_text), "%d%%", battery_info.charge_percent);
	
	layer_mark_dirty(s_foreground_layer);
}

static void initialize_ui() {
	GRect bounds = layer_get_bounds(s_window_layer);
	
	s_foreground_layer = layer_create(bounds);
	layer_set_update_proc(s_foreground_layer, foreground_update_proc);
	
	layer_add_child(window_get_root_layer(s_main_window), s_foreground_layer);
	layer_mark_dirty(s_foreground_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
//   update_ui();
}

static void main_window_load(Window *window) {
	s_window_layer = window_get_root_layer(window);
	initialize_ui();
	update_ui();
}

static void main_window_unload(Window *window) {
}

static void init() {
	s_main_window = window_create();
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	window_stack_push(s_main_window, true);
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}