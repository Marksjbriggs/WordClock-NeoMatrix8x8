/*
  Adapted from phil b's TIMESQUARE WATCH code.

  Only here if you feel like it!

*/

// Moon display: shows APPROXIMATE phase of moon.  This is not a medical
// device -- do not rely on this data if you are prone to lycanthropy.
// The phase shown is based on time elapsed since a known new moon, an
// approach that the Arduino can easily process but may be limited by a
// few flaws: the "known new moon" time is based on UTC, not local time,
// and may be a few hours off; the lunar period is assumed uniform and
// is rounded to whole seconds, and thus may drift over very long periods;
// unknown long-term accuracy of RTClib (e.g. DST & leap seconds ignored);
// uses 32-bit unixtime() and thus likely innacurate in years 2038+.
// This is for fun, not Real Science(tm).

// Used by various display modes for smooth fade-out before sleep
const uint8_t PROGMEM
fade[] =
{
  0,  1,  1,  2,  4,  5,  8, 10, 13, 17, 22, 27, 32, 39, 46,
  53, 62, 71, 82, 93, 105, 117, 131, 146, 161, 178, 196, 214, 234, 255
};

static const uint8_t PROGMEM
phases[] = {
  0x3B, 0x1F, 0x01, 0x00, 0x3E, 0x26, 0x03, 0x00, 0x3F, 0x2E, 0x06, 0x00, 0x42, 0x3E, 0x07,
  0x00, 0x47, 0x56, 0x07, 0x00, 0x54, 0x7A, 0x07, 0x00, 0x81, 0x88, 0x07, 0x00, 0xCC, 0x88,
  0x07, 0x00, 0xF2, 0x88, 0x07, 0x00, 0xF2, 0x84, 0x06, 0x00, 0xF0, 0x77, 0x03, 0x00, 0xED,
  0x63, 0x01, 0x00, 0xE3, 0x42, 0x01, 0x00, 0xCE, 0x27, 0x01, 0x00, 0x96, 0x1E, 0x01, 0x00,
  0x56, 0x1E, 0x01, 0x00, 0x00, 0x0B, 0x36, 0x01, 0x00, 0x0B, 0x46, 0x06, 0x00, 0x0B, 0x60,
  0x07, 0x00, 0x0B, 0x98, 0x07, 0x00, 0x0D, 0xCC, 0x07, 0x00, 0x46, 0xE8, 0x07, 0x01, 0xFC,
  0xE8, 0x07, 0x7A, 0xFF, 0xE8, 0x07, 0xFF, 0xFF, 0xE8, 0x08, 0xFF, 0xFF, 0xE1, 0x05, 0xFF,
  0xFF, 0xC5, 0x02, 0xFF, 0xFF, 0x89, 0x01, 0xFF, 0xF5, 0x47, 0x01, 0xFF, 0x82, 0x36, 0x01,
  0xEB, 0x0D, 0x36, 0x01, 0x1F, 0x0B, 0x36, 0x01, 0x00, 0x00, 0x0B, 0x1F, 0x00, 0x00, 0x0B,
  0x47, 0x00, 0x00, 0x0B, 0x6D, 0x00, 0x00, 0x1D, 0x89, 0x00, 0x00, 0x4D, 0x88, 0x00, 0x06,
  0xFF, 0x88, 0x00, 0xC3, 0xFF, 0x88, 0x64, 0xFF, 0xFF, 0x88, 0xFF, 0xFF, 0xFF, 0x88, 0xFF,
  0xFF, 0xFF, 0x72, 0xFF, 0xFF, 0xFF, 0x4B, 0xFF, 0xFF, 0xF5, 0x22, 0xFF, 0xFF, 0x78, 0x1F,
  0xFF, 0xB8, 0x0B, 0x1F, 0xFF, 0x04, 0x0B, 0x1F, 0x26, 0x00, 0x0B, 0x1F, 0x00, 0x00, 0x00,
  0x3B, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0xA4, 0x00, 0x00, 0x01, 0xF5, 0x00, 0x00,
  0x21, 0xF5, 0x00, 0x00, 0xFF, 0xF7, 0x00, 0xA0, 0xFF, 0xF5, 0x5C, 0xFF, 0xFF, 0xF5, 0xFF,
  0xFF, 0xFF, 0xF5, 0xFF, 0xFF, 0xFF, 0xDC, 0xFF, 0xFF, 0xFF, 0xA2, 0xFF, 0xFF, 0xFF, 0x59,
  0xFF, 0xFF, 0x7A, 0x3B, 0xFF, 0xF5, 0x00, 0x3C, 0xFF, 0x0C, 0x00, 0x3C, 0x2A, 0x00, 0x00,
  0x3C, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0xA4, 0x00, 0x00,
  0x01, 0xF5, 0x00, 0x00, 0x20, 0xF5, 0x00, 0x00, 0xFF, 0xF5, 0x00, 0xA0, 0xFF, 0xF5, 0x5D,
  0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xFF, 0xF5, 0xFF, 0xFF, 0xFF, 0xDC, 0xFF, 0xFF, 0xFF, 0xA0,
  0xFF, 0xFF, 0xFF, 0x58, 0xFF, 0xFF, 0x7A, 0x3B, 0xFF, 0xF5, 0x00, 0x3C, 0xFF, 0x0C, 0x00,
  0x3C, 0x2B, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x0B, 0x1F, 0x00, 0x00, 0x0B, 0x48, 0x00, 0x00,
  0x0B, 0x6D, 0x00, 0x00, 0x1D, 0x88, 0x00, 0x00, 0x4E, 0x88, 0x00, 0x06, 0xFF, 0x88, 0x00,
  0xC3, 0xFF, 0x88, 0x66, 0xFF, 0xFF, 0x88, 0xFF, 0xFF, 0xFF, 0x89, 0xFF, 0xFF, 0xFF, 0x73,
  0xFF, 0xFF, 0xFF, 0x4B, 0xFF, 0xFF, 0xF5, 0x22, 0xFF, 0xFF, 0x77, 0x1F, 0xFF, 0xB8, 0x0B,
  0x1F, 0xFF, 0x04, 0x0B, 0x1F, 0x26, 0x00, 0x0B, 0x1F, 0x00, 0x0B, 0x36, 0x01, 0x00, 0x0B,
  0x46, 0x06, 0x00, 0x0B, 0x5E, 0x07, 0x00, 0x0B, 0x9A, 0x07, 0x00, 0x0E, 0xCC, 0x07, 0x00,
  0x46, 0xE8, 0x07, 0x01, 0xFC, 0xE6, 0x07, 0x7C, 0xFF, 0xE8, 0x07, 0xFF, 0xFF, 0xE8, 0x07,
  0xFF, 0xFF, 0xE1, 0x05, 0xFF, 0xFF, 0xC7, 0x02, 0xFF, 0xFF, 0x88, 0x01, 0xFF, 0xF7, 0x48,
  0x01, 0xFF, 0x84, 0x36, 0x01, 0xEB, 0x0D, 0x36, 0x01, 0x1F, 0x0B, 0x36, 0x01, 0x3B, 0x1F,
  0x01, 0x00, 0x3F, 0x27, 0x03, 0x00, 0x3F, 0x2D, 0x06, 0x00, 0x41, 0x3E, 0x07, 0x00, 0x47,
  0x55, 0x07, 0x00, 0x52, 0x7A, 0x07, 0x00, 0x7F, 0x88, 0x07, 0x00, 0xCA, 0x88, 0x07, 0x00,
  0xF2, 0x88, 0x07, 0x00, 0xF5, 0x82, 0x06, 0x00, 0xF2, 0x77, 0x03, 0x00, 0xED, 0x63, 0x01,
  0x00, 0xE6, 0x42, 0x01, 0x00, 0xCE, 0x27, 0x01, 0x00, 0x96, 0x1F, 0x01, 0x00, 0x56, 0x1F,
  0x01, 0x00
}
,
leftHalf[]  = { 
  0, 0,  0,  0,  0,  0,  0,  0, 15, 14, 13, 12, 11, 10, 9,
  8, 8,  8,  8,  8,  8,  8,  8,  7,  6,  5,  4,  3,  2, 1
}
,
rightHalf[] = { 
  0, 1,  2,  3,  4,  5,  6,  7,  8,  8,  8,  8,  8,  8, 8,
  8, 9, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0, 0
};

// Time/date of a known new moon (UNIX time) - Dec 7 1999 22:32
#define NEW_MOON 944605920
#define LP       2551443L // Lunar period in seconds

uint8_t phase = 0;

void mode_moon() {

  DateTime time = RTC.now();
  phase = (uint8_t) (((time.unixtime() - NEW_MOON) % LP) / (24L * 3600L));

  //uint8_t  b = 255; //(uint8_t)pgm_read_byte(&fade);
  uint8_t  b = (uint8_t)pgm_read_byte(&fade);

  matrix.fillScreen(0);
  blit(phases, 64, 8, pgm_read_byte(&rightHalf[phase]) * 4 , 0, 4, 0, 4, 8, b );
  matrix.setRotation(2);
  blit(phases, 64, 8, pgm_read_byte(&leftHalf[phase])  * 4 , 0, 4, 0, 4, 8, b );
  matrix.setRotation(0);


  matrix.show();
}


void blit(const uint8_t *img, int iw, int ih, int sx, int sy, int dx, int dy,
int w, int h, uint8_t b) {
  uint16_t b1;
  uint8_t  shift, x, y;

  shift = 16;


  if ((dx >= 8) || ((dx + w - 1) < 0)) return; // Quick X-only clipping

  b1 = (uint16_t)b + 1; // +1 so shift (rather than divide) can be used

  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      byte clr = ((uint8_t)pgm_read_byte(&img[(sy + y) * iw + sx + x]) * b1);
      if (clr > 0 && clr < 70) {
        clr = clr * 40;  // boost dim pixels
      }
      matrix.drawPixel(dx + x, dy + y,
      matrix.Color(0, 0, clr)); // draw in blue, pale white pixels look green and funky...

    }
  }
}


