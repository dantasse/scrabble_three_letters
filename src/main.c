#include <pebble.h>
#include <string.h>

Window *my_window;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *word_layer;
TextLayer *defn_layer;

char** words; // array of words.
int NUM_WORDS_IN_LIST = 105;

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  char* current_time = "00:00";
  strftime(current_time, 6 /*maxsize*/, "%l:%M", tick_time);
  text_layer_set_text(time_layer, current_time);

  char* current_date = "00/00/00";
  // ugh there's no strftime thing for "month without leading zero",
  // rolling my own...
  snprintf(current_date, 9 /*maxsize*/, "%d/%d/%d", tick_time->tm_mon +1,
           tick_time->tm_mday, tick_time->tm_year % 100);
  
  text_layer_set_text(date_layer, current_date);
  
  char* current_word = "ZZ";
  int word_num = (tick_time->tm_hour * 60 + tick_time->tm_min) % NUM_WORDS_IN_LIST;
  strncpy(current_word, words[word_num], 2);
  text_layer_set_text(word_layer, current_word);
  text_layer_set_text(defn_layer, words[word_num] + 4); // skip 4 char ahead
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

char** get_words(char* buffer) {
  const char delim[2] = "\n";
  words = malloc(sizeof(char*) * NUM_WORDS_IN_LIST);
  // TODO if we want to make this scale to beyond just these 105 words,
  // maybe keep reallocing this instead of just mallocing it once? Meh.
  
  int n_words = 0;
  char* token = strtok(buffer, delim);
  while(token != NULL) {
    n_words++;
    words[n_words-1] = malloc(sizeof(char)*strlen(token));
    strcpy(words[n_words-1], token);
    token = strtok(NULL, delim);
  }
  return words;
}

void handle_init(void) {
  my_window = window_create();
  
  // Load the text from the twoletters.txt file.
  ResHandle rh = resource_get_handle(RESOURCE_ID_TWO_LETTERS);
  size_t res_size = resource_size(rh) + 1;
  uint8_t *all_text_buffer_uint = (uint8_t*) malloc(res_size);
  resource_load(rh, all_text_buffer_uint, res_size);
  char* all_text_buffer = (char*) all_text_buffer_uint;
  words = get_words(all_text_buffer);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  time_layer = text_layer_create(GRect(0, 0, 50, 48));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorBlack);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(time_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(time_layer));

  date_layer = text_layer_create(GRect(50, 0, 92, 48)); // end 2px from the end, just looks nicer
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorBlack);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(date_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(date_layer));

  word_layer = text_layer_create(GRect(0, 48, 144, 56));
  text_layer_set_background_color(word_layer, GColorClear);
  text_layer_set_text_color(word_layer, GColorBlack);
  text_layer_set_font(word_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(word_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(word_layer));
  
  defn_layer = text_layer_create(GRect(0, 104, 144, 64));
  text_layer_set_background_color(defn_layer, GColorClear);
  text_layer_set_text_color(defn_layer, GColorBlack);
  text_layer_set_font(defn_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(defn_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(defn_layer));

  // Show the Window on the watch, with animated=true
  window_stack_push(my_window, true);
  update_time();

}

void handle_deinit(void) {
  text_layer_destroy(time_layer);
  text_layer_destroy(word_layer);
  text_layer_destroy(defn_layer);
  for(int i = 0; i < NUM_WORDS_IN_LIST; i++) {
    free(words[i]);
  }
  free(words);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
