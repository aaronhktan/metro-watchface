#include <pebble.h>
#include "comm.h"
#include "src/c/settings.h"

static void comm_inbox_received(DictionaryIterator *iter, void *context) {
	Tuple *line_colour_t = dict_find(iter, MESSAGE_KEY_lineColour);
	if (line_colour_t) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "A line colour %d was received!", (int)line_colour_t->value->int32);
		GColor line_colour = GColorFromHEX(line_colour_t->value->int32);
		settings_set_line_colour(line_colour);
	}
	settings_save_settings();
}

void comm_init() {
	app_message_register_inbox_received(comm_inbox_received);
	app_message_open(128, 128);
}