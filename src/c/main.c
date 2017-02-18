#include <pebble.h>
#include "src/c/graphics.h"
#include "src/c/comm.h"
#include "src/c/settings.h"

#define LECO_20 RESOURCE_ID_FONT_LECO_20
#define TRAIN_HEIGHT 30
#define TRAIN_WIDTH 45
#define TRAIN_DELAY 500
#define TRAIN_DURATION 1500
#define PASSENGER_TYPES 8
#define MAX_NUMBER_OF_PASSENGERS 5

static Window *s_main_window;
static Layer *s_window_layer, *s_foreground_layer, *s_background_layer, *s_train_layer;
static GRect bounds;
static int s_minutes, s_shape[6] = {4, 4, 4, 4, 4, 4}, s_shape_on_train[6] = {4, 4, 4, 4, 4, 4}, s_station = 15, s_number_of_passengers_waiting, s_number_of_passengers_on_train = 0;
static char s_time_text[6] = "00:00", s_battery_text[5] = "100%";
// s_heart_rate_text[8] = "200 BPM";
static PropertyAnimation *animation_in, *animation_out;
static bool s_animating, s_train_at_station;
static GFont s_leco_font;
static AppTimer *s_passenger_timer;

/********************************************************************************************************************************* Layer update procedures */

// The foreground is the time, battery, station, and passengers
static void foreground_update_proc(Layer *s_foreground_layer, GContext *ctx) {
	graphics_draw_foreground(ctx, s_window_layer, s_leco_font, s_time_text, s_battery_text, s_shape, s_number_of_passengers_waiting);
}

// Background includes the line and the sand
static void background_update_proc(Layer *s_background_layer, GContext *ctx) {
	graphics_draw_background(ctx, s_window_layer);
}

// The train layer includes the train and all the passengers on the train
static void train_update_proc(Layer *s_train_layer, GContext *ctx) {
	graphics_draw_train(ctx, s_train_layer, s_train_at_station, s_number_of_passengers_on_train, s_shape_on_train);
}

/********************************************************************************************************************************* Text procedures */

// Set strings for time and battery
static void set_time_and_battery() {
	// Set time and minutes
	time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
	strftime(s_time_text, sizeof(s_time_text), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
	s_minutes = tick_time->tm_min % 5;
	
	// Set battery
  BatteryChargeState battery_info = battery_state_service_peek();
  snprintf(s_battery_text, sizeof(s_battery_text), "%d%%", battery_info.charge_percent);
	
	// Set heart rate
	//   #if PBL_API_EXISTS(health_service_peek_current_value)
//     HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
// 		snprintf(s_heart_rate_text, sizeof(s_heart_rate_text), "%lu BPM", (uint32_t) value);
//   #endif
	
	// Set steps
}

// Set the number of passengers
static void set_number_of_passengers() {
	// Set number of passengers based on time
	if (s_minutes == 0) {
		s_number_of_passengers_waiting = 5;
	} else {
		s_number_of_passengers_waiting = s_minutes;
	}
}

// Assign passenger shapes
static void generate_all_shapes() {
	for (int i = 0; i < MAX_NUMBER_OF_PASSENGERS; i++) {
		s_shape[i] = rand() % PASSENGER_TYPES;
		while(s_shape[i] == s_station) { // Prevent generating passenger shape that is the same as the station
			s_shape[i] = rand() % PASSENGER_TYPES;
		}
		APP_LOG(APP_LOG_LEVEL_DEBUG, "The random number generated is %d.", s_shape[i]);
		s_shape_on_train[i] = s_shape[i];
	}
}

/********************************************************************************************************************************* Animation procedures */

// Reset variables after train animation is done
static void end_train_animation() {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "The train animation has stopped.");
	s_animating = false;
	s_number_of_passengers_on_train = 0;
	animation_destroy((Animation*)animation_out);
	generate_all_shapes();
}

// PropertyAnimation to move the train from station to out of screen
static void train_animation_out() {
	layer_mark_dirty(s_foreground_layer);
	s_train_at_station = true;
	GRect from_frame = GRect(bounds.size.w / 2 - 0.5 * TRAIN_WIDTH, bounds.size.h * 2 / 3 - 0.5 * TRAIN_HEIGHT, TRAIN_WIDTH, TRAIN_HEIGHT);
	GRect to_frame = GRect(bounds.size.w + 60, bounds.size.h * 2 / 3 - 0.5 * TRAIN_HEIGHT, TRAIN_WIDTH, TRAIN_HEIGHT);
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
		.stopped = (void*)end_train_animation // Reset everything when animation is complete
	}, NULL);
	animation_schedule((Animation*)animation_out);
}

// PropertyAnimation for train layer to move it from offscreen to the station
static void train_animation_in() {
	if (!s_animating) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Train is moving in.");
		GRect from_frame = GRect(-60, bounds.size.h * 2 / 3 - 0.5 * TRAIN_HEIGHT, TRAIN_WIDTH, TRAIN_HEIGHT);
		GRect to_frame = GRect(bounds.size.w / 2 - 0.5 * TRAIN_WIDTH, bounds.size.h * 2 / 3 - 0.5 * TRAIN_HEIGHT, TRAIN_WIDTH, TRAIN_HEIGHT);
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

static void add_passenger_to_train() {
	if (s_number_of_passengers_waiting > 0) { // There are still passengers at the station
		s_passenger_timer = app_timer_register(TRAIN_DELAY, add_passenger_to_train, NULL); // Wait for some time before taking next passenger
		--s_number_of_passengers_waiting; // Remove one passenger from the number of passengers left at the station
		APP_LOG(APP_LOG_LEVEL_DEBUG, "The number of passengers waiting is %d.", s_number_of_passengers_waiting);
		s_number_of_passengers_on_train++; // Add one passenger to the number of passengers on the train
		APP_LOG(APP_LOG_LEVEL_DEBUG, "The number of passengers on the train is %d.", s_number_of_passengers_on_train);
		layer_mark_dirty(s_train_layer);
		for (int i = 1; i <= s_number_of_passengers_waiting; i++) { // Move all the waiting passengers down one
			s_shape[i - 1] = s_shape[i];
		}
	}
}

/********************************************************************************************************************************* UI procedures */

static void update_ui() {
	// Set time and battery
	set_time_and_battery();

	// Set number of passengers waiting only when not animating; when animating, this would redraw passengers as they were being taken onto the train
	if (!s_animating) {
		s_number_of_passengers_waiting++;
	}
	
	// Redraw foreground layer to show time and battery
	layer_mark_dirty(s_foreground_layer);
}

static void initialize_ui() {
	// Create foreground layer, background layer, and train layer
	bounds = layer_get_bounds(s_window_layer);
	
	s_background_layer = layer_create(bounds);
	layer_set_update_proc(s_background_layer, background_update_proc);
	
	s_foreground_layer = layer_create(bounds);
	layer_set_update_proc(s_foreground_layer, foreground_update_proc);

	s_train_layer = layer_create(GRect(-60, bounds.size.h * 0.75 - 0.5 * TRAIN_HEIGHT, TRAIN_WIDTH, TRAIN_HEIGHT));
	layer_set_update_proc(s_train_layer, train_update_proc);
	
	layer_add_child(window_get_root_layer(s_main_window), s_background_layer);
	layer_add_child(window_get_root_layer(s_main_window), s_train_layer);
	layer_add_child(window_get_root_layer(s_main_window), s_foreground_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	// Upon minute change, UI is updated
	update_ui();
	// Animate train coming in every 5 minutes
	if (s_minutes == 0 && !s_animating) {
		train_animation_in();
		s_passenger_timer = app_timer_register(TRAIN_DURATION + TRAIN_DELAY, add_passenger_to_train, NULL);
	}
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
	if (!s_animating) {
		train_animation_in();
		s_passenger_timer = app_timer_register(TRAIN_DURATION + TRAIN_DELAY, add_passenger_to_train, NULL);
	}
}

static void main_window_load(Window *window) {
	// Load and update UI
	s_window_layer = window_get_root_layer(window);
	initialize_ui();
	set_number_of_passengers();
	generate_all_shapes();
	update_ui();
	set_number_of_passengers();
	// Animate train coming in
	train_animation_in();
	s_passenger_timer = app_timer_register(TRAIN_DURATION + TRAIN_DELAY, add_passenger_to_train, NULL);
	// Subscribe to accelerometer shake to animate train coming in
	accel_tap_service_subscribe(accel_tap_handler);
}

static void main_window_unload(Window *window) {
	layer_destroy(s_window_layer);
	layer_destroy(s_foreground_layer);
	layer_destroy(s_background_layer);
	layer_destroy(s_train_layer);
	animation_unschedule_all();
	fonts_unload_custom_font(s_leco_font);
}

static void init() {
	APP_LOG(APP_LOG_LEVEL_INFO, "Running version 0.5.");
	
	comm_init();
	settings_init();
	
	// Create the main window
	s_main_window = window_create();
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	// Initialize randomizer for generating random passenger shapes
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