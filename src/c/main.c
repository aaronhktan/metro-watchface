#include <pebble.h>

#define LECO_20 RESOURCE_ID_FONT_LECO_20
#define TRAIN_HEIGHT 30
#define TRAIN_WIDTH 45
#define TRAIN_DELAY 500
#define TRAIN_DURATION 1500
#define PASSENGER_TYPES 3

static Window *s_main_window;
static Layer *s_window_layer, *s_foreground_layer, *s_background_layer, *s_train_layer;
static int s_minutes, s_shape[6] = {4, 4, 4, 4, 4, 4}, s_shape_on_train[6] = {4, 4, 4, 4, 4, 4}, s_station = 4, s_number_of_passengers_waiting, s_number_of_passengers_on_train = 0, s_number_of_passengers;
static char s_time_text[6] = "00:00", s_battery_text[5] = "100%";
// s_heart_rate_text[8] = "200 BPM";
static PropertyAnimation *animation_in, *animation_out;
static bool s_animating, s_pick_up_passengers = false;
static GFont s_leco_font;
static AppTimer *s_passenger_timer;

// Reset variables after train animation is done
static void end_train_animation() {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "The train animation has stopped.");
	s_animating = false;
	s_pick_up_passengers = false;
	s_number_of_passengers_on_train = 0;
	animation_destroy((Animation*)animation_out);
}

// PropertyAnimation to move the train from station to out of screen
static void train_animation_out() {
	s_pick_up_passengers = true;
	layer_mark_dirty(s_foreground_layer);
	GRect bounds = layer_get_bounds(s_window_layer);
	GRect from_frame = GRect(bounds.size.w / 2 - 0.5 * TRAIN_WIDTH, bounds.size.h * 0.75 - 0.5 * TRAIN_HEIGHT, TRAIN_WIDTH, TRAIN_HEIGHT);
	GRect to_frame = GRect(bounds.size.w + 60, bounds.size.h * 0.75 - 0.5 * TRAIN_HEIGHT, TRAIN_WIDTH, TRAIN_HEIGHT);
	animation_out = property_animation_create_layer_frame(s_train_layer, &from_frame, &to_frame);
	animation_set_duration((Animation*)animation_out, TRAIN_DURATION);
	if (s_number_of_passengers_on_train == 0) {
		animation_set_delay((Animation*)animation_out, 0);
		animation_set_curve((Animation*)animation_out, AnimationCurveLinear);
	} else {
		animation_set_delay((Animation*)animation_out, s_number_of_passengers_waiting * TRAIN_DELAY);
		animation_set_curve((Animation*)animation_out, AnimationCurveEaseIn);
	}
	animation_set_handlers((Animation*)animation_out, (AnimationHandlers) {
		.started = NULL,
		.stopped = (void*)end_train_animation
	}, NULL);
	animation_schedule((Animation*)animation_out);
}

// PropertyAnimation for train layer to move it from offscreen to the station
static void train_animation_in() {
	if (!s_animating) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Train is moving in.");
		GRect bounds = layer_get_bounds(s_window_layer);
		GRect from_frame = GRect(-60, bounds.size.h * 0.75 - 0.5 * TRAIN_HEIGHT, TRAIN_WIDTH, TRAIN_HEIGHT);
		GRect to_frame = GRect(bounds.size.w / 2 - 0.5 * TRAIN_WIDTH, bounds.size.h * 0.75 - 0.5 * TRAIN_HEIGHT, TRAIN_WIDTH, TRAIN_HEIGHT);
		animation_in = property_animation_create_layer_frame(s_train_layer, &from_frame, &to_frame);
		animation_set_duration((Animation*)animation_in, TRAIN_DURATION);
		animation_set_delay((Animation*)animation_in, TRAIN_DELAY);
		if (s_number_of_passengers_waiting == 0) {
			animation_set_curve((Animation*)animation_in, AnimationCurveLinear);
		} else {
			animation_set_curve((Animation*)animation_in, AnimationCurveEaseOut);
		}
		animation_set_handlers((Animation*)animation_in, (AnimationHandlers) {
			.started = NULL,
			.stopped = (void*)train_animation_out
		}, NULL);
		s_animating = true;
		animation_schedule((Animation*)animation_in);
	}
}

// The train layer includes the train and all the passengers on the train
static void train_update_proc(Layer *s_train_layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(s_train_layer);
	
	graphics_context_set_fill_color(ctx, GColorYellow); // Set train colour to yellow
	
	graphics_fill_rect(ctx, bounds, 0, GCornerNone); // Fill the rectangle with yellow
	
	if (s_pick_up_passengers) { // This means that the train is at the station and the redraw should add passengers
		graphics_context_set_fill_color(ctx, GColorRed);
		switch(s_number_of_passengers_on_train) {
			case 5:
				switch(s_shape_on_train[4]) {
					case 0:
						graphics_fill_circle(ctx, GPoint(bounds.size.w * 5 / 6, bounds.size.h / 4), bounds.size.h / 4);
						break;
					case 1:
						graphics_fill_rect(ctx, GRect(bounds.size.w * 2 / 3 + bounds.size.w / 20, + bounds.size.w / 20, bounds.size.w / 3 - bounds.size.w / 10, bounds.size.h / 2 - bounds.size.w / 10), 0, GCornerNone);
						break;
					case 2: ;
						const GPathInfo TRIANGLE_PATH_INFO_5 = {
							.num_points = 3,
							.points = (GPoint []) {{bounds.size.w * 2 / 3, 0}, {bounds.size.w, bounds.size.h / 4}, {bounds.size.w * 2 / 3, bounds.size.h / 2}}
						};
						GPath *s_triangle_path_5 = gpath_create(&TRIANGLE_PATH_INFO_5);
						gpath_draw_filled(ctx, s_triangle_path_5);
						gpath_destroy(s_triangle_path_5);
						break;
				}
			case 4:
				switch(s_shape_on_train[3]) {
					case 0:
						graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h * 3 / 4), bounds.size.h / 4);
						break;
					case 1:
						graphics_fill_rect(ctx, GRect(bounds.size.w / 3 + bounds.size.w / 20, bounds.size.h / 2 + bounds.size.w / 20, bounds.size.w / 3 - bounds.size.w / 10, bounds.size.h / 2 - bounds.size.w / 10), 0, GCornerNone);
						break;
					case 2: ;
						const GPathInfo TRIANGLE_PATH_INFO_4 = {
							.num_points = 3,
							.points = (GPoint []) {{bounds.size.w / 3, bounds.size.h / 2}, {bounds.size.w * 2 / 3, bounds.size.h * 0.75}, {bounds.size.w / 3, bounds.size.h}}
						};
						GPath *s_triangle_path_4 = gpath_create(&TRIANGLE_PATH_INFO_4);
						gpath_draw_filled(ctx, s_triangle_path_4);
						gpath_destroy(s_triangle_path_4);
						break;
				}
			case 3:
				switch(s_shape_on_train[2]) {
					case 0:
						graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 4), bounds.size.h / 4);
						break;
					case 1:
						graphics_fill_rect(ctx, GRect(bounds.size.w / 3 + bounds.size.w / 20, bounds.size.w / 20, bounds.size.w / 3 - bounds.size.w / 10, bounds.size.h / 2 - bounds.size.w / 10), 0, GCornerNone);
						break;
					case 2: ;
						const GPathInfo TRIANGLE_PATH_INFO_3 = {
							.num_points = 3,
							.points = (GPoint []) {{bounds.size.w / 3, 0}, {bounds.size.w * 2 / 3, bounds.size.h / 4}, {bounds.size.w / 3, bounds.size.h / 2}}
						};
						GPath *s_triangle_path_3 = gpath_create(&TRIANGLE_PATH_INFO_3);
						gpath_draw_filled(ctx, s_triangle_path_3);
						gpath_destroy(s_triangle_path_3);
						break;
				}
			case 2:
				switch(s_shape_on_train[1]) {
					case 0:
						graphics_fill_circle(ctx, GPoint(bounds.size.w / 6, bounds.size.h * 3 / 4), bounds.size.h / 4);
						break;
					case 1:
						graphics_fill_rect(ctx, GRect(bounds.size.w / 20, bounds.size.h / 2 + bounds.size.w / 20, bounds.size.w / 3 - bounds.size.w / 10, bounds.size.h / 2 - bounds.size.w / 10), 0, GCornerNone);
						break;
					case 2: ;
						const GPathInfo TRIANGLE_PATH_INFO_2 = {
							.num_points = 3,
							.points = (GPoint []) {{0, bounds.size.h / 2}, {bounds.size.w / 3, bounds.size.h * 0.75}, {0, bounds.size.h}}
						};
						GPath *s_triangle_path_2 = gpath_create(&TRIANGLE_PATH_INFO_2);
						gpath_draw_filled(ctx, s_triangle_path_2);
						gpath_destroy(s_triangle_path_2);
						break;
				}
			case 1:
					switch(s_shape_on_train[0]) {
						case 0:
							graphics_fill_circle(ctx, GPoint(bounds.size.w / 6, bounds.size.h / 4), bounds.size.h / 4);
							break;
						case 1:
							graphics_fill_rect(ctx, GRect(bounds.size.w / 20, bounds.size.w / 20, bounds.size.w / 3 - bounds.size.w / 10, bounds.size.h / 2 - bounds.size.w / 10), 0, GCornerNone);
							break;
						case 2: ;
							const GPathInfo TRIANGLE_PATH_INFO_1 = {
										.num_points = 3,
										.points = (GPoint []) {{0, 0}, {bounds.size.w / 3, bounds.size.h / 4}, {0, bounds.size.h / 2}}
									};
									GPath *s_triangle_path_1 = gpath_create(&TRIANGLE_PATH_INFO_1);
									gpath_draw_filled(ctx, s_triangle_path_1);
									gpath_destroy(s_triangle_path_1);
							break;
						}
		}
	}
}

static void add_passenger_to_train() {
	if (s_number_of_passengers_waiting > 0) { // There are still passengers at the station
		s_passenger_timer = app_timer_register(TRAIN_DELAY, add_passenger_to_train, NULL); // Wait for some time before taking next passenger
		--s_number_of_passengers_waiting; // Remove one passenger from the number of passengers left at the station
		APP_LOG(APP_LOG_LEVEL_DEBUG, "The number of passengers waiting is %d.", s_number_of_passengers_waiting);
		s_number_of_passengers_on_train++; // Add one passenger to the number of passengers on the train
		APP_LOG(APP_LOG_LEVEL_DEBUG, "The number of passengers on the train is %d.", s_number_of_passengers_on_train);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "The number of passengers in total is %d.", s_number_of_passengers);
		layer_mark_dirty(s_train_layer);
		for (int i = 1; i <= s_number_of_passengers_waiting; i++) { // Move all the waiting passengers down one
			s_shape[i - 1] = s_shape[i];
		}
	}
}

// Background includes the line and the sand
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
}

// The foreground is the time, battery, station, and passengers
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
		GSize battery_text_bounds = graphics_text_layout_get_content_size("100%", s_leco_font,
																																	 GRect(0, 0, bounds.size.w, bounds.size.h),
																																	 GTextOverflowModeWordWrap, GTextAlignmentCenter);
		graphics_draw_text(ctx, s_battery_text, s_leco_font, 
											 GRect((bounds.size.w - battery_text_bounds.w) / 2, bounds.size.h / 4 - battery_text_bounds.h + 8, battery_text_bounds.w, battery_text_bounds.h),
											 GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

		// Draw the station
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h * 0.75), bounds.size.w / 10);
		graphics_context_set_stroke_width(ctx, 5);
		graphics_context_set_stroke_color(ctx, GColorBlack);
		graphics_draw_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h * 0.75), bounds.size.w / 10);

		// Draw the passengers
		graphics_context_set_fill_color(ctx, GColorBlack);
		for (int i = 0; i < s_number_of_passengers_waiting; i++) {
			int s_shape_width = 2 * bounds.size.w / 20;
// 			APP_LOG(APP_LOG_LEVEL_DEBUG, "The random number is %d.", s_shape[i]);
			switch(s_shape[i]) {
				case 0: // This is a circle
					graphics_fill_circle(ctx, GPoint(bounds.size.w / 2 + bounds.size.w / 20 + 2 * bounds.size.w / 20 + i * (s_shape_width + bounds.size.w / 40), bounds.size.h * 0.75 - 11), bounds.size.h / 30);
					break;
				case 1: // This is a square
					graphics_fill_rect(ctx, GRect(bounds.size.w / 2 + bounds.size.w / 15 + bounds.size.w / 20 + i *(s_shape_width + bounds.size.w / 40), bounds.size.h * 0.75 - 16, bounds.size.h / 15, bounds.size.h / 15), 0, GCornerNone);
					break;
				case 2: ; // This is a triangle
					const GPathInfo TRIANGLE_PATH_INFO = {
						.num_points = 3,
						.points = (GPoint []) {{bounds.size.w / 2 + bounds.size.w / 15 + 2 * bounds.size.w / 20 + i * (s_shape_width + bounds.size.w / 40), bounds.size.h * 0.75 - bounds.size.w / 10 - 5},
																		{bounds.size.w / 2 + bounds.size.w / 15 + 3 * bounds.size.w / 20 + i * (s_shape_width + bounds.size.w / 40), bounds.size.h * 0.75 - bounds.size.w / 10 + bounds.size.w / 20},
																		{bounds.size.w / 2 + bounds.size.w / 15 + bounds.size.w / 20 + i * (s_shape_width + bounds.size.w / 40), bounds.size.h * 0.75 - bounds.size.w / 10 + bounds.size.w / 20}}
					};
					GPath *s_triangle_path = gpath_create(&TRIANGLE_PATH_INFO);
					gpath_draw_filled(ctx, s_triangle_path);
					gpath_destroy(s_triangle_path);
					break;
			}
		}
	}
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
// }

static void update_ui() {
	// Set time
	time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

	// Put time into string
  strftime(s_time_text, sizeof(s_time_text), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
	s_minutes = tick_time->tm_min % 5;
	if (s_minutes == 0) {
		s_number_of_passengers_waiting = 5;
		for (int i = 0; i < s_number_of_passengers_waiting; i++) {
			s_shape[i] = rand() % PASSENGER_TYPES;
			while(s_shape[i] == s_station) {
				s_shape[i] = rand() % PASSENGER_TYPES;
			}
			s_shape_on_train[i] = s_shape[i];
		}
	} else {
		s_number_of_passengers_waiting = s_minutes;
		if (s_shape[s_number_of_passengers_waiting] == 4) { // 4 means that the shape has not been assigned yet
			s_shape[s_number_of_passengers_waiting] = rand() % PASSENGER_TYPES;
		}
	}
	
	s_number_of_passengers = s_number_of_passengers_waiting - 1;
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "The current minutes is %d.", s_minutes);

//   #if PBL_API_EXISTS(health_service_peek_current_value)
//     HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
// 		snprintf(s_heart_rate_text, sizeof(s_heart_rate_text), "%lu BPM", (uint32_t) value);
//   #endif

	// Set battery
  BatteryChargeState battery_info = battery_state_service_peek();
  snprintf(s_battery_text, sizeof(s_battery_text), "%d%%", battery_info.charge_percent);

	// Redraw foreground layer to show time and battery
// 	if (s_animating) {
		layer_mark_dirty(s_foreground_layer);
// 	} else {
// 		app_timer_register(2 * TRAIN_DURATION + 2 * TRAIN_DELAY, update_ui, NULL);
// 	}
	
	// Animate train coming in every 5 minutes
	if (s_minutes == 0 && !s_animating) {
		train_animation_in();
		s_passenger_timer = app_timer_register(TRAIN_DURATION + TRAIN_DELAY, add_passenger_to_train, NULL);
	}
}

static void initialize_ui() {
	// Create foreground layer, background layer, and train layer
	GRect bounds = layer_get_bounds(s_window_layer);
	
	s_background_layer = layer_create(bounds);
	layer_set_update_proc(s_background_layer, background_update_proc);
	
	s_foreground_layer = layer_create(bounds);
	layer_set_update_proc(s_foreground_layer, foreground_update_proc);

	s_train_layer = layer_create(GRect(-60, bounds.size.h * 0.75 - 0.5 * TRAIN_HEIGHT, TRAIN_WIDTH, TRAIN_HEIGHT));
	layer_set_update_proc(s_train_layer, train_update_proc);
	
	layer_add_child(window_get_root_layer(s_main_window), s_background_layer);
	layer_add_child(window_get_root_layer(s_main_window), s_train_layer);
	layer_add_child(window_get_root_layer(s_main_window), s_foreground_layer);
	
	time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
	strftime(s_time_text, sizeof(s_time_text), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
	s_minutes = tick_time->tm_min % 5;
	
// 	// Set battery
//   BatteryChargeState battery_info = battery_state_service_peek();
//   snprintf(s_battery_text, sizeof(s_battery_text), "%d%%", battery_info.charge_percent);
	
// 	layer_mark_dirty(s_foreground_layer);
	
	if (s_minutes == 0) {
		s_number_of_passengers_waiting = 5;
	} else {
		s_number_of_passengers_waiting = s_minutes;
	}
	
	// Assign passenger shapes
	for (int i = 0; i < s_number_of_passengers_waiting; i++) {
		s_shape[i] = rand() % PASSENGER_TYPES;
		while(s_shape[i] == s_station) { // Prevent generating passenger shape that is the same as the station
			s_shape[i] = rand() % PASSENGER_TYPES;
		}
		s_shape_on_train[i] = s_shape[i];
	}
	
	s_number_of_passengers = s_number_of_passengers_waiting - 1;
	
	// Animate train coming in
	train_animation_in();
	s_passenger_timer = app_timer_register(TRAIN_DURATION + TRAIN_DELAY, add_passenger_to_train, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	if (!s_animating) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Update was not rescheduled!");
		// Upon minute change, UI is updated
		update_ui();
	} else {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Animation is happening, so the update was rescheduled.");
		app_timer_register(4 * TRAIN_DELAY + 2 * TRAIN_DURATION, update_ui, NULL);
	}
}

static void main_window_load(Window *window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "The main window has been created.");
	// Load and update UI
	s_window_layer = window_get_root_layer(window);
	initialize_ui();
	update_ui();
}

static void main_window_unload(Window *window) {
	// Destory all layers, stop animations, and unload custom font
	layer_destroy(s_window_layer);
	layer_destroy(s_foreground_layer);
	layer_destroy(s_background_layer);
	layer_destroy(s_train_layer);
	animation_unschedule_all();
	fonts_unload_custom_font(s_leco_font);
}

static void init() {
	APP_LOG(APP_LOG_LEVEL_INFO, "Running version 0.5.");
	// Create the main window
	s_main_window = window_create();
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	// Initialize randomizer
	time_t t;
	srand((unsigned) time(&t));
	
	// Load custom font
	s_leco_font = fonts_load_custom_font(resource_get_handle(LECO_20));
	
	// Push window
	window_stack_push(s_main_window, true);
	
	// Subcribe to the clock
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}