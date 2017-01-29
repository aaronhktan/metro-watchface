#include <pebble.h>
#include "graphics.h"
#include "src/c/settings.h"

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
	graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h * 2 / 3), bounds.size.w / 10);
	graphics_context_set_stroke_width(ctx, 7);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h * 2 / 3), bounds.size.w / 10);
	
	// Draw the passengers
	graphics_context_set_fill_color(ctx, GColorBlack);
	for (int i = 0; i < s_number_of_passengers_waiting; i++) {
		int s_shape_width = bounds.size.w / 11; // This is the width that every shape takes up
		#if PBL_DISPLAY_WIDTH == 144
			int s_distance_between = 2;
		#elif PBL_DISPLAY_WIDTH == 180
			int s_distance_between = 2;
		#elif PBL_PLATFORM_EMERY
			int s_distance_between = 3;
		#else
			int s_distance_between = 2;
		#endif
		int s_vertical_offset = bounds.size.w / 10;
		int s_horizontal_center = bounds.size.w / 2;
		int s_vertical_center = bounds.size.h * 2 / 3;
		int s_station_radius = bounds.size.w / 7;
		// 			APP_LOG(APP_LOG_LEVEL_DEBUG, "The random number is %d.", s_shape[i]);
		switch(s_shape[i]) {
			case 0: // This is a circle (x is center, add radius of station, add half of circle, then add space between shapes) (y is center of line - vertical offset + half of radius)
				graphics_fill_circle(ctx, GPoint(s_horizontal_center + s_station_radius + s_shape_width / 2 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width / 2), s_shape_width / 2);
				break;
			case 1: // This is a square (GRect is x, y, width, height) (x coordinate is center of screen + radius of station + space between)
				graphics_fill_rect(ctx, GRect(s_horizontal_center + s_station_radius + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset, s_shape_width, s_shape_width), 0, GCornerNone);
				break;
			case 2: ; // This is a triangle
				const GPathInfo TRIANGLE_PATH_INFO = { // (middle top, left bottom, right bottom)
					.num_points = 3,
					.points = (GPoint []) {{s_horizontal_center + s_station_radius + s_shape_width / 2 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset - 1},
					{s_horizontal_center + s_station_radius + i * (s_shape_width + s_distance_between) - s_distance_between / 2, s_vertical_center - s_vertical_offset - 1 + s_shape_width},
					{s_horizontal_center + s_station_radius + i * (s_shape_width + s_distance_between) + s_shape_width + s_distance_between / 2, s_vertical_center - s_vertical_offset - 1 + s_shape_width}}
				};
				GPath *s_triangle_path = gpath_create(&TRIANGLE_PATH_INFO);
				gpath_draw_filled(ctx, s_triangle_path);
				gpath_destroy(s_triangle_path);
				break;
			case 3: ; // This is a star
				const GPathInfo STAR_PATH_INFO = { // top, then clockwise
					.num_points = 10,
					.points = (GPoint []) {{s_horizontal_center + s_station_radius + s_shape_width / 2 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset - 2},
																{s_horizontal_center + s_station_radius + s_shape_width * 6 / 8 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width * 2 / 5 - 2},
																{s_horizontal_center + s_station_radius + s_shape_width + 1 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width * 2 / 5 - 2},
																{s_horizontal_center + s_station_radius + 1 + s_shape_width * 6 / 8 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width * 6 / 10},
																{s_horizontal_center + s_station_radius + 1 + s_shape_width * 15 / 16 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width + 1},
																{s_horizontal_center + s_station_radius + s_shape_width / 2 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width * 4 / 5},
																{s_horizontal_center + s_station_radius + s_shape_width * 1 / 16 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width + 1},
																{s_horizontal_center + s_station_radius - 1 + s_shape_width * 2 / 8 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width * 6 / 10},
																{s_horizontal_center + s_station_radius - 1 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width * 2 / 5 - 1},
																{s_horizontal_center + s_station_radius - 1 + s_shape_width * 2 / 8 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width * 2 / 5 - 1}}
				};
				GPath *s_star_path = gpath_create(&STAR_PATH_INFO);
				gpath_draw_filled(ctx, s_star_path);
				gpath_destroy(s_star_path);
				break;
			case 4: ; // This is a pentagon
				const GPathInfo PENTAGON_PATH_INFO = {
					.num_points = 5,
					.points = (GPoint []) {{s_horizontal_center + s_station_radius + s_shape_width / 2 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset - 2},
																{s_horizontal_center + s_station_radius + s_shape_width + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width * 7 / 20},
																{s_horizontal_center + s_station_radius + s_shape_width * 9 / 10 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width - 1},
																{s_horizontal_center + s_station_radius + s_shape_width * 1 / 10 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width - 1},
																{s_horizontal_center + s_station_radius - 1 + i * (s_shape_width + s_distance_between), s_vertical_center - s_vertical_offset + s_shape_width * 7 / 20}}
				};
				GPath *s_pentagon_path = gpath_create(&PENTAGON_PATH_INFO);
				gpath_draw_filled(ctx, s_pentagon_path);
				gpath_destroy(s_pentagon_path);
				break;
			case 5: ; // This is a diamond
				break;
			case 6: ; // This is a cross
				break;
			case 7: ; // This is a teardrop
				break;
			case 8: ; // This is a gem
				break;
			case 9: ; // This is an oval
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
	graphics_context_set_fill_color(ctx, settings_get_line_colour());
	graphics_fill_rect(ctx, GRect(0, bounds.size.h * 2 / 3 - 10, bounds.size.w, 20), 0, GCornerNone);
}

void graphics_draw_train(GContext *ctx, Layer *s_train_layer, bool s_train_at_station, int s_number_of_passengers_on_train, int s_shape_on_train[]) {
	GRect bounds = layer_get_bounds(s_train_layer);
	
	graphics_context_set_fill_color(ctx, settings_get_line_colour()); // Set train colour to colour set by user
	
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