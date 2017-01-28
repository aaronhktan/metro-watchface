#include <pebble.h>
#include "settings.h"

ClaySettings settings;

void settings_init() {
	settings.line_colour = GColorYellow;
	if (persist_exists(SETTINGS_KEY)) {
		persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
	}
}

void settings_save_settings() {
	persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
	persist_write_int(SETTINGS_VERSION_KEY, 1);
}

void settings_set_line_colour(GColor value) {
	settings.line_colour = value;
}

GColor settings_get_line_colour() {
	return settings.line_colour;
}