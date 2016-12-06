#include <pebble.h>

static Window *s_main_window;
static Layer *s_window_layer, *s_foreground_layer, *s_background_layer, *s_train_layer;
static char s_time_text[6] = "00:00", s_battery_text[5] = "100%";
// s_heart_rate_text[8] = "200 BPM";
static PropertyAnimation *animation_in, *animation_out;
static bool s_animating;

static void end_train_animation() {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "The train animation has stopped.");
	s_animating = false;
}

static void train_animation_out() {
	GRect bounds = layer_get_bounds(s_window_layer);
	GRect from_frame = GRect(bounds.size.w / 2 - 30, bounds.size.h * 0.75 - 20, 60, 40);
	GRect to_frame = GRect(bounds.size.w + 60, bounds.size.h * 0.75 - 20, 60, 40);
	animation_out = property_animation_create_layer_frame(s_train_layer, &from_frame, &to_frame);
	animation_set_duration((Animation*)animation_out, 1500);
	animation_set_delay((Animation*)animation_out, 1000);
	animation_set_curve((Animation*)animation_out, AnimationCurveEaseIn);
	animation_set_handlers((Animation*)animation_out, (AnimationHandlers) {
		.started = NULL,
		.stopped = (void*)end_train_animation
	}, NULL);
	animation_schedule((Animation*)animation_out);
}

static void train_animation_in() {
	if (!s_animating) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Train is moving in.");
		GRect bounds = layer_get_bounds(s_window_layer);
		GRect from_frame = GRect(-60, bounds.size.h * 0.75 - 20, 60, 40);
		GRect to_frame = GRect(bounds.size.w / 2 - 30, bounds.size.h * 0.75 - 20, 60, 40);
		animation_in = property_animation_create_layer_frame(s_train_layer, &from_frame, &to_frame);
		animation_set_duration((Animation*)animation_in, 1500);
		animation_set_delay((Animation*)animation_out, 1000);
		animation_set_curve((Animation*)animation_in, AnimationCurveEaseOut);
		animation_set_handlers((Animation*)animation_in, (AnimationHandlers) {
			.started = NULL,
			.stopped = (void*)train_animation_out
		}, NULL);
		s_animating = true;
		animation_schedule((Animation*)animation_in);
	}
}

static void train_update_proc(Layer *s_train_layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(s_train_layer);
	
	graphics_context_set_fill_color(ctx, GColorYellow);
	
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void background_update_proc(Layer *s_background_layer, GContext *ctx) {
	// Sets bounds of window
	GRect bounds = layer_get_bounds(s_window_layer);
	
	// Set colour to the closest thing to sand
	graphics_context_set_fill_color(ctx, GColorWhite);
	// Draw background color
	PBL_IF_RECT_ELSE(graphics_fill_rect(ctx, bounds, 0, GCornerNone), graphics_fill_circle(ctx, grect_center_point(&bounds), bounds.size.w));
	
	// Draw the line
	graphics_context_set_fill_color(ctx, GColorYellow);
	graphics_fill_rect(ctx, GRect(0, bounds.size.h * 0.75 - 10, bounds.size.w, 20), 0, GCornerNone);
	
	// Draw the station
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h * 0.75), bounds.size.w / 10);
	graphics_context_set_stroke_width(ctx, 5);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h * 0.75), bounds.size.w / 10);
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
										 GRect((bounds.size.w - time_text_bounds.w) / 2, bounds.size.h / 4, time_text_bounds.w, time_text_bounds.h),
										 GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	
	// Draw battery text
	GSize battery_text_bounds = graphics_text_layout_get_content_size("100%", fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS),
																																 GRect(0, 0, bounds.size.w, bounds.size.h),
																																 GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_battery_text, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LECO_20)), 
										 GRect((bounds.size.w - battery_text_bounds.w) / 2, bounds.size.h / 4 - battery_text_bounds.h / 2 + 8, battery_text_bounds.w, battery_text_bounds.h),
										 GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	
// 	#if PBL_API_EXISTS(health_service_peek_current_value)
// 		// Draw heart rate text
// 		if (health_service_peek_current_value(HealthMetricHeartRateBPM) != 0) {
// 		GSize heart_rate_text_bounds = graphics_text_layout_get_content_size("200 BPM", fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS),
// 																																	 GRect(0, 0, bounds.size.w, bounds.size.h),
// 																																	 GTextOverflowModeWordWrap, GTextAlignmentCenter);
// 		graphics_draw_text(ctx, s_heart_rate_text, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LECO_20)), 
// 											 GRect((bounds.size.w - heart_rate_text_bounds.w) / 2, bounds.size.h / 2 + time_text_bounds.h / 2, heart_rate_text_bounds.w, heart_rate_text_bounds.h),
// 											 GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
// 		}
// 	#endif
}

static void update_ui() {
	time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  strftime(s_time_text, sizeof(s_time_text), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

//   #if PBL_API_EXISTS(health_service_peek_current_value)
//     HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
// 		snprintf(s_heart_rate_text, sizeof(s_heart_rate_text), "%lu BPM", (uint32_t) value);
//   #endif

  BatteryChargeState battery_info = battery_state_service_peek();
  snprintf(s_battery_text, sizeof(s_battery_text), "%d%%", battery_info.charge_percent);
	layer_mark_dirty(s_foreground_layer);
	
	train_animation_in();
}

static void initialize_ui() {
	GRect bounds = layer_get_bounds(s_window_layer);
	
	s_background_layer = layer_create(bounds);
	layer_set_update_proc(s_background_layer, background_update_proc);
	
	s_foreground_layer = layer_create(bounds);
	layer_set_update_proc(s_foreground_layer, foreground_update_proc);

	s_train_layer = layer_create(GRect(-60, bounds.size.h * 0.75 - 20, 60, 40));
	layer_set_update_proc(s_train_layer, train_update_proc);
	
	layer_add_child(window_get_root_layer(s_main_window), s_background_layer);
	layer_add_child(window_get_root_layer(s_main_window), s_foreground_layer);
	layer_add_child(window_get_root_layer(s_main_window), s_train_layer);
	layer_mark_dirty(s_background_layer);
	layer_mark_dirty(s_foreground_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_ui();
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