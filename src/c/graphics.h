#pragma once

void graphics_draw_foreground(GContext *ctx, Layer *s_window_layer, GFont s_leco_font, char* s_time_text, char* s_battery_text, int s_shape[], int s_number_of_passengers_waiting);
void graphics_draw_background(GContext *ctx, Layer *s_window_layer);
void graphics_draw_train(GContext *ctx, Layer *s_train_layer, bool s_train_at_station, int s_number_of_passengers_on_train, int s_shape_on_train[]);