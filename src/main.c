#include "pebble.h"
  
#define KEY_HERO1 5
#define KEY_HERO2 6
#define KEY_HERO3 7
#define KEY_HERO4 8
  
#define KEY_BLUETOOTH 9
  
#define TOTAL_BT_DIGITS 2
static GBitmap *bt_digits_images[TOTAL_BT_DIGITS];
static BitmapLayer *bt_digits_layers[TOTAL_BT_DIGITS];
  
#define TOTAL_HERO_DIGITS 5
static GBitmap *hero_digits_images[TOTAL_HERO_DIGITS];
static BitmapLayer *hero_digits_layers[TOTAL_HERO_DIGITS];

Window *s_main_window;

static TextLayer *steps_layer;
static TextLayer *s_time_layer;
TextLayer *mana_layer;
static TextLayer *battery_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

#define NUM_LEVEL_PKEY 0
#define NUM_LEVEL_DEFAULT 1 
static int level = NUM_LEVEL_DEFAULT;
static TextLayer *lvl_layer;

/*
 * Variables for Step Counting
 */
// Total Steps (TS)
#define TS 1
// Total Steps Default (TSD)
#define TSD 1
 
// value to auto adjust step acceptance 
const int PED_ADJUST = 2;

// Less increases sensitivity
int X_DELTA = 35; 
int Y_DELTA, Z_DELTA = 185;
int YZ_DELTA_MIN = 175;
int YZ_DELTA_MAX = 195; 


int X_DELTA_TEMP, Y_DELTA_TEMP, Z_DELTA_TEMP = 0;
int lastX, lastY, lastZ, currX, currY, currZ = 0;

long stepGoal = 220000;
long pedometerCount = 0;
long lastPedometerCount = 0;

bool did_pebble_vibrate = false;
bool validX, validY, validZ = false;
bool startedSession = false;
static bool initiate_watchface = true;

// configuration values
uint32_t secondsTillStepsUpdate = 0;
uint32_t stepsUpdateInterval = 1; // in seconds;

// AppMessage Keys
#define UPDATE_INTERVAL 2
  
const int BT_WHITE_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_Moogle_Indicator_WHITE,
  RESOURCE_ID_Chocobo_Indicator_WHITE};

const int BT_BLACK_IMAGE_RESOURCE_IDS[] = {
 	RESOURCE_ID_Moogle_Indicator_BLACK,
  RESOURCE_ID_Chocobo_Indicator_BLACK};

const int HERO_IMAGE_RESOURCE_IDS[] = {
RESOURCE_ID_Bard,
RESOURCE_ID_BlackMage,
RESOURCE_ID_Conjurer,
RESOURCE_ID_Dragoon,
RESOURCE_ID_Fighter,
RESOURCE_ID_Geomancer, 
RESOURCE_ID_Hunter,
RESOURCE_ID_Karateka, 
RESOURCE_ID_Knight, 
RESOURCE_ID_Monk, 
RESOURCE_ID_MysticKnight,
RESOURCE_ID_Ninja, 
RESOURCE_ID_OnionKnight, 
RESOURCE_ID_RedMage, 
RESOURCE_ID_Sage, 
RESOURCE_ID_Scholar, 
RESOURCE_ID_Shaman, 
RESOURCE_ID_Summoner, 
RESOURCE_ID_Thief, 
RESOURCE_ID_Viking, 
RESOURCE_ID_Warlock, 
RESOURCE_ID_WhiteMage
};


//Bitmap Container
static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;

 	*bmp_image = gbitmap_create_with_resource(resource_id);
 	GRect frame = (GRect) {
   	.origin = origin,
   	.size = (*bmp_image)->bounds.size
};
 	bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
 	layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);

    if (old_image != NULL) {
 	gbitmap_destroy(old_image);
}
}
  
static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100/100";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "Regen");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d/100", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}
  

// Bluetooth Connectivity
static void handle_bluetooth(bool connected) {
  
  int indicator = persist_read_int(KEY_BLUETOOTH);
  
  if(indicator == 0){
if (connected) {
// Normal Mode 
if (initiate_watchface) {
   	layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[0]), true);
   	layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[1]), true);
}
// On disconnection vibrate twice
else {
    layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[0]), true);
   	layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[1]), true);
vibes_double_pulse();
}
}
else {
  
// If started in disconnection display Moogle, no vibration
if (initiate_watchface) {
  layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[0]), false);
bitmap_layer_set_compositing_mode(bt_digits_layers[0], GCompOpOr);
set_container_image(&bt_digits_images[0], bt_digits_layers[0], BT_WHITE_IMAGE_RESOURCE_IDS[0], GPoint(114, 117));
  layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[1]), false);
    bitmap_layer_set_compositing_mode(bt_digits_layers[1], GCompOpClear);
set_container_image(&bt_digits_images[1], bt_digits_layers[1], BT_BLACK_IMAGE_RESOURCE_IDS[0], GPoint(114, 117));
}
  
// On disconnection display Moogle and vibrate three times
else {
  layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[0]), false);
bitmap_layer_set_compositing_mode(bt_digits_layers[0], GCompOpOr);
set_container_image(&bt_digits_images[0], bt_digits_layers[0], BT_WHITE_IMAGE_RESOURCE_IDS[0], GPoint(114, 117));
  layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[1]), false);
    bitmap_layer_set_compositing_mode(bt_digits_layers[1], GCompOpClear);
set_container_image(&bt_digits_images[1], bt_digits_layers[1], BT_BLACK_IMAGE_RESOURCE_IDS[0], GPoint(114, 117));

vibes_enqueue_custom_pattern( (VibePattern) {
   	.durations = (uint32_t []) {100, 100, 100, 100, 100},
   	.num_segments = 5
} );
}}
  }
  else if(indicator == 1){
    
  if (connected) {
// Normal Mode 
if (initiate_watchface) {
   	layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[0]), true);
   	layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[1]), true);
}
// On disconnection vibrate twice
else {
    layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[0]), true);
   	layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[1]), true);
vibes_double_pulse();
}
}
else {
  
// If started in disconnection display Moogle, no vibration
if (initiate_watchface) {
  layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[0]), false);
bitmap_layer_set_compositing_mode(bt_digits_layers[0], GCompOpOr);
set_container_image(&bt_digits_images[0], bt_digits_layers[0], BT_WHITE_IMAGE_RESOURCE_IDS[1], GPoint(114, 117));
  layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[1]), false);
    bitmap_layer_set_compositing_mode(bt_digits_layers[1], GCompOpClear);
set_container_image(&bt_digits_images[1], bt_digits_layers[1], BT_BLACK_IMAGE_RESOURCE_IDS[1], GPoint(114, 117));
}
  
// On disconnection display Moogle and vibrate three times
else {
  layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[0]), false);
bitmap_layer_set_compositing_mode(bt_digits_layers[0], GCompOpOr);
set_container_image(&bt_digits_images[0], bt_digits_layers[0], BT_WHITE_IMAGE_RESOURCE_IDS[1], GPoint(114, 117));
  layer_set_hidden(bitmap_layer_get_layer(bt_digits_layers[1]), false);
    bitmap_layer_set_compositing_mode(bt_digits_layers[1], GCompOpClear);
set_container_image(&bt_digits_images[1], bt_digits_layers[1], BT_BLACK_IMAGE_RESOURCE_IDS[1], GPoint(114, 117));

vibes_enqueue_custom_pattern( (VibePattern) {
   	.durations = (uint32_t []) {100, 100, 100, 100, 100},
   	.num_segments = 5
} );
}}
    
    
  }

}

  
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a buffer
  static char buffer[] = "00:00";
  static char date_text[] = "00.00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  // Only update the date when it's changed.
  strftime(date_text, sizeof(date_text), "%m.%d", tick_time);
  text_layer_set_text(mana_layer, date_text);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  
  handle_battery(battery_state_service_peek());
}

static void update_level() {
  
  static char lvl_text[] = "123";
  snprintf(lvl_text, sizeof(lvl_text), "%u", level);
  text_layer_set_text(lvl_layer, lvl_text);
    
}


static void window_load(Window *me) {
  
  window_set_background_color(me, GColorBlack);
  
   // Background image
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_WHITE);
	s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpOr);
	layer_add_child(window_get_root_layer(me), bitmap_layer_get_layer(s_background_layer));
  
  //Check for saved option
  int hero1 = persist_read_int(KEY_HERO1);
  int hero2 = persist_read_int(KEY_HERO2);
  int hero3 = persist_read_int(KEY_HERO3);
  int hero4 = persist_read_int(KEY_HERO4);


  //Hero1========================================================================================================
  if(hero1 == 0)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[0], GPoint(5, 15)); 
  }
  else if(hero1 == 1)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[1], GPoint(5, 15)); 
  }
  else if(hero1 == 2)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[2], GPoint(5, 15)); 
  }
  else if(hero1 == 3)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[3], GPoint(5, 15)); 
  }

  else if(hero1 == 4)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[4], GPoint(5, 15)); 
  }

  else if(hero1 == 5)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[5], GPoint(5, 15)); 
  }

  else if(hero1 == 6)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[6], GPoint(5, 15)); 
  }

  else if(hero1 == 7)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[7], GPoint(5, 15)); 
  }

  else if(hero1 == 8)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[8], GPoint(5, 15)); 
  }

  else if(hero1 == 9)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[9], GPoint(5, 15)); 
  }

  else if(hero1 == 10)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[10], GPoint(5, 15)); 
  }

  else if(hero1 == 11)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[11], GPoint(5, 15)); 
  }

  else if(hero1 == 12)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[12], GPoint(5, 15)); 
  }

  else if(hero1 == 13)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[13], GPoint(5, 15)); 
  }

  else if(hero1 == 14)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[14], GPoint(5, 15)); 
  }

  else if(hero1 == 15)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[15], GPoint(5, 15)); 
  }

  else if(hero1 == 16)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[16], GPoint(5, 15)); 
  }

  else if(hero1 == 17)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[17], GPoint(5, 15)); 
  }

  else if(hero1 == 18)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[18], GPoint(5, 15)); 
  }

  else if(hero1 == 19)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[19], GPoint(5, 15)); 
  }
  else if(hero1 == 20)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[20], GPoint(5, 15)); 
  }

  else if(hero1 == 21)
  {
   set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[21], GPoint(5, 15)); 
  }
  
  //Hero2=========================================================================================================
 if(hero2 == 0)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[0], GPoint(5, 53)); 
  }
  else if(hero2 == 1)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[1], GPoint(5, 53)); 
  }
  else if(hero2 == 2)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[2], GPoint(5, 53)); 
  }
  else if(hero2 == 3)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[3], GPoint(5, 53)); 
  }

  else if(hero2 == 4)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[4], GPoint(5, 53)); 
  }

  else if(hero2 == 5)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[5], GPoint(5, 53)); 
  }

  else if(hero2 == 6)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[6], GPoint(5, 53)); 
  }

  else if(hero2 == 7)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[7], GPoint(5, 53)); 
  }

  else if(hero2 == 8)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[8], GPoint(5, 53)); 
  }

  else if(hero2 == 9)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[9], GPoint(5, 53)); 
  }

  else if(hero2 == 10)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[10], GPoint(5, 53)); 
  }

  else if(hero2 == 11)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[11], GPoint(5, 53)); 
  }

  else if(hero2 == 12)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[12], GPoint(5, 53)); 
  }

  else if(hero2 == 13)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[13], GPoint(5, 53)); 
  }
  else if(hero2 == 14)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[14], GPoint(5, 53)); 
  }

  else if(hero2 == 15)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[15], GPoint(5, 53)); 
  }

  else if(hero2 == 16)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[16], GPoint(5, 53)); 
  }

  else if(hero2 == 17)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[17], GPoint(5, 53)); 
  }

  else if(hero2 == 18)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[18], GPoint(5, 53)); 
  }

  else if(hero2 == 19)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[19], GPoint(5, 53)); 
  }
  else if(hero2 == 20)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[20], GPoint(5, 53)); 
  }

  else if(hero2 == 21)
  {
   set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[21], GPoint(5, 53)); 
  }
  
  //HERO3==========================================================================================================

  if(hero3 == 0)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[0], GPoint(5, 93)); 
  }
  else if(hero3 == 1)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[1], GPoint(5, 93)); 
  }
  else if(hero3 == 2)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[2], GPoint(5, 93)); 
  }
  else if(hero3 == 3)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[3], GPoint(5, 93)); 
  }

  else if(hero3 == 4)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[4], GPoint(5, 93)); 
  }

  else if(hero3 == 5)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[5], GPoint(5, 93)); 
  }

  else if(hero3 == 6)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[6], GPoint(5, 93)); 
  }

  else if(hero3 == 7)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[7], GPoint(5, 93)); 
  }

  else if(hero3 == 8)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[8], GPoint(5, 93)); 
  }

  else if(hero3 == 9)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[9], GPoint(5, 93)); 
  }

  else if(hero3 == 10)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[10], GPoint(5, 93)); 
  }

  else if(hero3 == 11)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[11], GPoint(5, 93)); 
  }

  else if(hero3 == 12)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[12], GPoint(5, 93)); 
  }

  else if(hero3 == 13)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[13], GPoint(5, 93)); 
  }

  else if(hero3 == 14)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[14], GPoint(5, 93)); 
  }

  else if(hero3 == 15)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[15], GPoint(5, 93)); 
  }

  else if(hero3 == 16)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[16], GPoint(5, 93)); 
  }

  else if(hero3 == 17)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[17], GPoint(5, 93)); 
  }

  else if(hero3 == 18)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[18], GPoint(5, 93)); 
  }

  else if(hero3 == 19)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[19], GPoint(5, 93)); 
  }
  else if(hero3 == 20)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[20], GPoint(5, 93)); 
  }

  else if(hero3 == 21)
  {
   set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[21], GPoint(5, 93)); 
  }
  
  //Hero4==========================================================================================================

  if(hero4 == 0)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[0], GPoint(5, 132)); 
  }
  else if(hero4 == 1)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[1], GPoint(5, 132)); 
  }
  else if(hero4 == 2)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[2], GPoint(5, 132)); 
  }
  else if(hero4 == 3)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[3], GPoint(5, 132)); 
  }

  else if(hero4 == 4)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[4], GPoint(5, 132)); 
  }

  else if(hero4 == 5)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[5], GPoint(5, 132)); 
  }

  else if(hero4 == 6)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[6], GPoint(5, 132)); 
  }

  else if(hero4 == 7)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[7], GPoint(5, 132)); 
  }

  else if(hero4 == 8)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[8], GPoint(5, 132)); 
  }

  else if(hero4 == 9)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[9], GPoint(5, 132)); 
  }

  else if(hero4 == 10)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[10], GPoint(5, 132)); 
  }

  else if(hero4 == 11)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[11], GPoint(5, 132)); 
  }

  else if(hero4 == 12)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[12], GPoint(5, 132)); 
  }

  else if(hero4 == 13)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[13], GPoint(5, 132)); 
  }

  else if(hero4 == 14)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[14], GPoint(5, 132)); 
  }

  else if(hero4 == 15)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[15], GPoint(5, 132)); 
  }

  else if(hero4 == 16)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[16], GPoint(5, 132)); 
  }

  else if(hero4 == 17)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[17], GPoint(5, 132)); 
  }

  else if(hero4 == 18)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[18], GPoint(5, 132)); 
  }

  else if(hero4 == 19)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[19], GPoint(5, 132)); 
  }
  else if(hero4 == 20)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[20], GPoint(5, 132)); 
  }

  else if(hero4 == 21)
  {
   set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[21], GPoint(5, 132)); 
  }
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(40, 135, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");  
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(me), text_layer_get_layer(s_time_layer));
  
  // Create Hit Points TextLayer
  battery_layer = text_layer_create(GRect(36, 99, 139, 34));
  text_layer_set_text_color(battery_layer, GColorWhite);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentLeft);
  text_layer_set_text(battery_layer, "100/100");
  layer_add_child(window_get_root_layer(me), text_layer_get_layer(battery_layer));
	 
  // Steps setup
	steps_layer = text_layer_create(GRect(36, 56, 144, 30));
	text_layer_set_text_color(steps_layer, GColorWhite);	
	text_layer_set_background_color(steps_layer, GColorClear);
	text_layer_set_font(steps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(steps_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(me), text_layer_get_layer(steps_layer));
  
   // Create Level TextLayer
  lvl_layer = text_layer_create(GRect(36, 20, 144, 34));
  text_layer_set_text_color(lvl_layer, GColorWhite);
  text_layer_set_background_color(lvl_layer, GColorClear);
  text_layer_set_font(lvl_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(lvl_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(me), text_layer_get_layer(lvl_layer));	
  
   // Create Mana TextLayer
  mana_layer = text_layer_create(GRect(36, 136, 144, 30));
  text_layer_set_text_color(mana_layer, GColorWhite);
  text_layer_set_background_color(mana_layer, GColorClear);
  layer_add_child(window_get_root_layer(me), text_layer_get_layer(mana_layer));
  text_layer_set_font(mana_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(mana_layer, GTextAlignmentLeft);
  
   update_time();
   update_level();
   battery_state_service_subscribe(&handle_battery);  
}

static void window_unload(Window *window) {
  
  //Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  //Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  //Destroy TextLayers
  text_layer_destroy(steps_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(mana_layer);
  
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  if(units_changed & SECOND_UNIT){
    secondsTillStepsUpdate++;
  }
}

static void in_recv_handler(DictionaryIterator *iterator, void *context)
{
  //Get Tuple
  Tuple *t = dict_read_first(iterator);
  while(t != NULL)
  {
    switch(t->key)
    {
    case KEY_HERO1:
      
      if(strcmp(t->value->cstring, "Bard") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[0], GPoint(5, 15));
        persist_write_int(KEY_HERO1, 0);
      }
      else if(strcmp(t->value->cstring, "BlackMage") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[1], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 1);
      }
      else if(strcmp(t->value->cstring, "Conjurer") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[2], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 2);
      }
      else if(strcmp(t->value->cstring, "Dragoon") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[3], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 3);
      }

      else if(strcmp(t->value->cstring, "Fighter") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[4], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 4);
      }

      else if(strcmp(t->value->cstring, "Geomancer") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[5], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 5);
      }

      else if(strcmp(t->value->cstring, "Hunter") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[6], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 6);
      }

      else if(strcmp(t->value->cstring, "Karateka") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[7], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 7);
      }

      else if(strcmp(t->value->cstring, "Knight") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[8], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 8);
      }

      else if(strcmp(t->value->cstring, "Monk") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[9], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 9);
      }

      else if(strcmp(t->value->cstring, "MysticKnight") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[10], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 10);
      }

      else if(strcmp(t->value->cstring, "Ninja") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[11], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 11);
      }

      else if(strcmp(t->value->cstring, "OnionKnight") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[12], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 12);
      }

      else if(strcmp(t->value->cstring, "RedMage") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[13], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 13);
      }

      else if(strcmp(t->value->cstring, "Sage") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[14], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 14);
      }

      else if(strcmp(t->value->cstring, "Scholar") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[15], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 15);
      }

      else if(strcmp(t->value->cstring, "Shaman") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[16], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 16);
      }

      else if(strcmp(t->value->cstring, "Summoner") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[17], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 17);
      }

      else if(strcmp(t->value->cstring, "Thief") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[18], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 18);
      }

      else if(strcmp(t->value->cstring, "Viking") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[19], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 19);
      }

      else if(strcmp(t->value->cstring, "Warlock") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[20], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 20);
      }

      else if(strcmp(t->value->cstring, "WhiteMage") == 0)
      {
        set_container_image(&hero_digits_images[1], hero_digits_layers[1], HERO_IMAGE_RESOURCE_IDS[21], GPoint(5, 15)); 
        persist_write_int(KEY_HERO1, 21);
      }

      break;

    case KEY_HERO2:
      
      if(strcmp(t->value->cstring, "Bard") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[0], GPoint(5, 53));
        persist_write_int(KEY_HERO2, 0);
      }
      else if(strcmp(t->value->cstring, "BlackMage") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[1], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 1);
      }
      else if(strcmp(t->value->cstring, "Conjurer") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[2], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 2);
      }
      else if(strcmp(t->value->cstring, "Dragoon") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[3], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 3);
      }

      else if(strcmp(t->value->cstring, "Fighter") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[4], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 4);
      }

      else if(strcmp(t->value->cstring, "Geomancer") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[5], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 5);
      }

      else if(strcmp(t->value->cstring, "Hunter") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[6], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 6);
      }

      else if(strcmp(t->value->cstring, "Karateka") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[7], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 7);
      }

      else if(strcmp(t->value->cstring, "Knight") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[8], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 8);
      }

      else if(strcmp(t->value->cstring, "Monk") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[9], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 9);
      }

      else if(strcmp(t->value->cstring, "MysticKnight") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[10], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 10);
      }

      else if(strcmp(t->value->cstring, "Ninja") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[11], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 11);
      }

      else if(strcmp(t->value->cstring, "OnionKnight") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[12], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 12);
      }

      else if(strcmp(t->value->cstring, "RedMage") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[13], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 13);
      }

      else if(strcmp(t->value->cstring, "Sage") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[14], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 14);
      }

      else if(strcmp(t->value->cstring, "Scholar") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[15], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 15);
      }

      else if(strcmp(t->value->cstring, "Shaman") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[16], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 16);
      }

      else if(strcmp(t->value->cstring, "Summoner") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[17], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 17);
      }

      else if(strcmp(t->value->cstring, "Thief") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[18], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 18);
      }

      else if(strcmp(t->value->cstring, "Viking") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[19], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 19);
      }

      else if(strcmp(t->value->cstring, "Warlock") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[20], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 20);
      }

      else if(strcmp(t->value->cstring, "WhiteMage") == 0)
      {
        set_container_image(&hero_digits_images[2], hero_digits_layers[2], HERO_IMAGE_RESOURCE_IDS[21], GPoint(5, 53)); 
        persist_write_int(KEY_HERO2, 21);
      }

      break;



    case KEY_HERO3:
      
      if(strcmp(t->value->cstring, "Bard") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[0], GPoint(5, 93));
        persist_write_int(KEY_HERO3, 0);
      }
      else if(strcmp(t->value->cstring, "BlackMage") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[1], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 1);
      }
      else if(strcmp(t->value->cstring, "Conjurer") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[2], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 2);
      }
      else if(strcmp(t->value->cstring, "Dragoon") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[3], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 3);
      }

      else if(strcmp(t->value->cstring, "Fighter") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[4], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 4);
      }

      else if(strcmp(t->value->cstring, "Geomancer") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[5], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 5);
      }

      else if(strcmp(t->value->cstring, "Hunter") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[6], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 6);
      }

      else if(strcmp(t->value->cstring, "Karateka") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[7], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 7);
      }

      else if(strcmp(t->value->cstring, "Knight") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[8], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 8);
      }

      else if(strcmp(t->value->cstring, "Monk") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[9], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 9);
      }

      else if(strcmp(t->value->cstring, "MysticKnight") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[10], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 10);
      }

      else if(strcmp(t->value->cstring, "Ninja") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[11], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 11);
      }

      else if(strcmp(t->value->cstring, "OnionKnight") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[12], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 12);
      }

      else if(strcmp(t->value->cstring, "RedMage") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[13], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 13);
      }

      else if(strcmp(t->value->cstring, "Sage") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[14], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 14);
      }

      else if(strcmp(t->value->cstring, "Scholar") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[15], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 15);
      }

      else if(strcmp(t->value->cstring, "Shaman") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[16], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 16);
      }

      else if(strcmp(t->value->cstring, "Summoner") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[17], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 17);
      }

      else if(strcmp(t->value->cstring, "Thief") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[18], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 18);
      }

      else if(strcmp(t->value->cstring, "Viking") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[19], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 19);
      }

      else if(strcmp(t->value->cstring, "Warlock") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[20], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 20);
      }

      else if(strcmp(t->value->cstring, "WhiteMage") == 0)
      {
        set_container_image(&hero_digits_images[3], hero_digits_layers[3], HERO_IMAGE_RESOURCE_IDS[21], GPoint(5, 93)); 
        persist_write_int(KEY_HERO3, 21);
      }

      break;

      
        
    case KEY_HERO4:
      
      if(strcmp(t->value->cstring, "Bard") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[0], GPoint(5, 132));
        persist_write_int(KEY_HERO4, 0);
      }
      else if(strcmp(t->value->cstring, "BlackMage") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[1], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 1);
      }
      else if(strcmp(t->value->cstring, "Conjurer") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[2], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 2);
      }
      else if(strcmp(t->value->cstring, "Dragoon") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[3], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 3);
      }

      else if(strcmp(t->value->cstring, "Fighter") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[4], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 4);
      }

      else if(strcmp(t->value->cstring, "Geomancer") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[5], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 5);
      }

      else if(strcmp(t->value->cstring, "Hunter") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[6], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 6);
      }

      else if(strcmp(t->value->cstring, "Karateka") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[7], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 7);
      }

      else if(strcmp(t->value->cstring, "Knight") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[8], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 8);
      }

      else if(strcmp(t->value->cstring, "Monk") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[9], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 9);
      }

      else if(strcmp(t->value->cstring, "MysticKnight") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[10], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 10);
      }

      else if(strcmp(t->value->cstring, "Ninja") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[11], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 11);
      }

      else if(strcmp(t->value->cstring, "OnionKnight") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[12], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 12);
      }

      else if(strcmp(t->value->cstring, "RedMage") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[13], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 13);
      }

      else if(strcmp(t->value->cstring, "Sage") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[14], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 14);
      }

      else if(strcmp(t->value->cstring, "Scholar") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[15], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 15);
      }

      else if(strcmp(t->value->cstring, "Shaman") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[16], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 16);
      }

      else if(strcmp(t->value->cstring, "Summoner") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[17], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 17);
      }

      else if(strcmp(t->value->cstring, "Thief") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[18], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 18);
      }

      else if(strcmp(t->value->cstring, "Viking") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[19], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 19);
      }

      else if(strcmp(t->value->cstring, "Warlock") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[20], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 20);
      }

      else if(strcmp(t->value->cstring, "WhiteMage") == 0)
      {
        set_container_image(&hero_digits_images[4], hero_digits_layers[4], HERO_IMAGE_RESOURCE_IDS[21], GPoint(5, 132)); 
        persist_write_int(KEY_HERO4, 21);
      }

      break;
      
      case KEY_BLUETOOTH:
      
      if(strcmp(t->value->cstring, "Moogle") == 0)
      {
        persist_write_int(KEY_BLUETOOTH, 0);
      }
      else if(strcmp(t->value->cstring, "Chocobo") == 0)
      {
         persist_write_int(KEY_BLUETOOTH, 1);
      }
            }
        // Look for next item
         t = dict_read_next(iterator);
  }
}

void init() {

   s_main_window = window_create();
   window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  level = persist_exists(NUM_LEVEL_PKEY) ? persist_read_int(NUM_LEVEL_PKEY) : NUM_LEVEL_DEFAULT;
  
  Layer *window_layer = window_get_root_layer(s_main_window);
  
  GRect dummy_frame = { {0, 0}, {0, 0} };
  
  for (int i = 0; i < TOTAL_HERO_DIGITS; ++i) {
   		hero_digits_layers[i] = bitmap_layer_create(dummy_frame);
   		layer_add_child(window_layer, bitmap_layer_get_layer(hero_digits_layers[i]));
	}
  
  app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
   app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  //Create Bluetooth layer
 	for (int i = 0; i < TOTAL_BT_DIGITS; ++i) {
   	bt_digits_layers[i] = bitmap_layer_create(dummy_frame);
   	layer_add_child(window_layer, bitmap_layer_get_layer(bt_digits_layers[i]));
  }
  
   handle_bluetooth(bluetooth_connection_service_peek());
   initiate_watchface = false;
  
   // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Window
  window_stack_push(s_main_window, true /* Animated */);
}



/*
 * Step Counter (Code basicly from https://github.com/jathusanT/pebble_pedometer)
 */



void autoCorrectZ(){
	if (Z_DELTA > YZ_DELTA_MAX){
		Z_DELTA = YZ_DELTA_MAX; 
	} else if (Z_DELTA < YZ_DELTA_MIN){
		Z_DELTA = YZ_DELTA_MIN;
	}
}

void autoCorrectY(){
	if (Y_DELTA > YZ_DELTA_MAX){
		Y_DELTA = YZ_DELTA_MAX; 
	} else if (Y_DELTA < YZ_DELTA_MIN){
		Y_DELTA = YZ_DELTA_MIN;
	}
}

void pedometer_update() {
	if (startedSession) {
		X_DELTA_TEMP = abs(abs(currX) - abs(lastX));
		if (X_DELTA_TEMP >= X_DELTA) {
			validX = true;
		}
		Y_DELTA_TEMP = abs(abs(currY) - abs(lastY));
		if (Y_DELTA_TEMP >= Y_DELTA) {
			validY = true;
			if (Y_DELTA_TEMP - Y_DELTA > 200){
				autoCorrectY();
				Y_DELTA = (Y_DELTA < YZ_DELTA_MAX) ? Y_DELTA + PED_ADJUST : Y_DELTA;
			} else if (Y_DELTA - Y_DELTA_TEMP > 175){
				autoCorrectY();
				Y_DELTA = (Y_DELTA > YZ_DELTA_MIN) ? Y_DELTA - PED_ADJUST : Y_DELTA;
			}
		}
		Z_DELTA_TEMP = abs(abs(currZ) - abs(lastZ));
		if (Z_DELTA_TEMP >= Z_DELTA) {
			validZ = true;
			if (Z_DELTA_TEMP - Z_DELTA > 200){
				autoCorrectZ();
				Z_DELTA = (Z_DELTA < YZ_DELTA_MAX) ? Z_DELTA + PED_ADJUST : Z_DELTA;
			} else if (Z_DELTA - Z_DELTA_TEMP > 175){
				autoCorrectZ();
				Z_DELTA = (Z_DELTA < YZ_DELTA_MAX) ? Z_DELTA + PED_ADJUST : Z_DELTA;
			}
		}
	} else {
		startedSession = true;
	}
  
}

void resetUpdate() {
	lastX = currX;
	lastY = currY;
	lastZ = currZ;
	validX = false;
	validY = false;
	validZ = false;
}

void update_ui_callback() {
	if ((validX && validY && !did_pebble_vibrate) || (validX && validZ && !did_pebble_vibrate)) {
		pedometerCount++;

		
    // steps
    if (secondsTillStepsUpdate >= stepsUpdateInterval && pedometerCount != lastPedometerCount ){
  		static char buf[] = "123456890abcdefghijkl";
  		snprintf(buf, sizeof(buf), "%ld", pedometerCount);
  		text_layer_set_text(steps_layer, buf);
      secondsTillStepsUpdate = 0;
      lastPedometerCount = pedometerCount;
    }

	if (pedometerCount % 2000 == 0) {
    level++;
    update_level();
  }
    
	}

	resetUpdate();
}

void accel_data_handler(AccelData *accel_data, uint32_t num_samples) {
  
  uint32_t i;

 			for (i=0;i<num_samples/3;i++){
          uint32_t appo = i*3; 
          AccelData accel = accel_data[appo];
          if (!startedSession) {
        		lastX = accel.x;
        		lastY = accel.y;
        		lastZ = accel.z;
        	} else {
        		currX = (accel_data[appo].x+accel_data[appo+1].x+accel_data[appo+2].x)/3;
        		currY = (accel_data[appo].y+accel_data[appo+1].y+accel_data[appo+2].y)/3;//accel.y;
        		currZ = (accel_data[appo].z+accel_data[appo+1].z+accel_data[appo+2].z)/3;//accel.z;
        	}
        	
        	did_pebble_vibrate = accel.did_vibrate;
        
        	pedometer_update();

  			}
  update_ui_callback();
}


/*
 * Update Screen and settings when configuration changed
 */
void update_from_settings(){

    accel_data_service_subscribe(9, accel_data_handler);
    accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
    static char buf[] = "123456890abcdefghijkl";
    snprintf(buf, sizeof(buf), "%ld", pedometerCount);
	  text_layer_set_text(steps_layer, buf);    
}


void deinit() {
	persist_write_int(TS, pedometerCount); // save steps on exit
  persist_write_int(NUM_LEVEL_PKEY, level);
  accel_data_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
}


int main(void) {
	init();
  bluetooth_connection_service_subscribe(&handle_bluetooth);
	tick_timer_service_subscribe(SECOND_UNIT , &tick_handler);
  
  // register for acellerometer events
  accel_data_service_subscribe(18, accel_data_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
  
  //Get saved data
  pedometerCount = persist_exists(TS) ? persist_read_int(TS) : TSD;
	stepsUpdateInterval = persist_exists(UPDATE_INTERVAL) ? persist_read_int(UPDATE_INTERVAL) : 1 ;
  update_from_settings();
  
  app_event_loop();
	deinit();
}
