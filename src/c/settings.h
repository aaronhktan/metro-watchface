#pragma once

#define SETTINGS_KEY 1
#define SETTINGS_VERSION_KEY 2

// Define settings struct
typedef struct ClaySettings {
	GColor line_colour;
} ClaySettings;

void settings_init();
void settings_save_settings();

void settings_set_line_colour(GColor value);
GColor settings_get_line_colour();