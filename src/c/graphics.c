#include <pebble.h>
#include "graphics.h"

void graphics_draw_foreground(GContext *ctx, Layer *s_window_layer, GFont s_leco_font, char* s_time_text, char *s_battery_text, int s_shape[], int s_number_of_passengers_waiting) {
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
}

void graphics_draw_background(GContext *ctx, Layer *s_window_layer) {
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

void graphics_draw_train(GContext *ctx, Layer *s_train_layer, bool s_train_at_station, int s_number_of_passengers_on_train, int s_shape_on_train[]) {
		GRect bounds = layer_get_bounds(s_train_layer);
	
	graphics_context_set_fill_color(ctx, GColorYellow); // Set train colour to yellow
	
	graphics_fill_rect(ctx, bounds, 0, GCornerNone); // Fill the rectangle with yellow
	
	if (s_train_at_station) { // This means that the train is at the station and the redraw should add passengers
		graphics_context_set_fill_color(ctx, GColorPastelYellow);
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