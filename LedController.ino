//
// Biltema LED controller 46-3384/46-3385
// 0 = H=0.5  L=5.5
// 1 = H=1.0  L=5.0
//
// That is a 6us cycle time, ~166KHz
//
// This can be simulated with SPI frequency = 4Mhz
// where each bit is 24 SPI bits = 3*8 bits
// 0 = 11000000|00000000|00000000
// 1 = 11110000|00000000|00000000
//

#include <SPI.h>

#define NUM_LEDS   (20+30)       // Starter=20 Extension=30 LEDS
#define RESET_US   240           // Not sure about this.
#define FREQUENCY  4000000
#define LCD_BITS(x)  ((x))   // (~(x))
// Bits should really be like in the comment above but they get stretched out
// a bit, but this shorter bits works when stretched.
// the cycle needes to be >= 6us but less than RESET_US (a bit unkown)
#define ZERO 0x80         // 1us   first third of ZERO bit
#define ONE  0xC0         // 0.5us first thrid of ONE bit

#define BLACK   0x000000
#define RED     0xFF0000
#define GREEN   0x00FF00
#define BLUE    0x0000FF
#define YELLOW  (RED+GREEN)
#define MAGENTA (RED+BLUE)
#define CYAN    (GREEN+BLUE)
#define WHITE   0xFFFFFF

#define ON WHITE
#define OFF BLACK

typedef uint32_t rgb_t;

#define rgb_red(x)   (((x)>>16) & 0xff)
#define rgb_green(x) (((x)>>8) & 0xff)
#define rgb_blue(x)  ((x) & 0xff)

rgb_t rgb_data[NUM_LEDS];  // rgb led data

static uint16_t step = 0;

static rgb_t make_rgb(uint8_t r, uint8_t g, uint8_t b)
{
  rgb_t color;
  color = ((rgb_t)r << 16) | ((rgb_t)g << 8) | (rgb_t)b;
  return color; 
}

static rgb_t get_pixel(int x)
{
    return rgb_data[x];
}

static void put_pixel(int x, rgb_t color)
{
    rgb_data[x] = color;
}

static void fill_pixels(int x, rgb_t color, int n)
{
    rgb_t* ptr = &rgb_data[x];
    while(n--) {
	    *ptr++ = color;
    }
}

static uint8_t spi_buf[24];

// transfer a byte bit by bit, each bit is 3 bytes to use SPI
// send hi to low bit order
void transfer_byte(uint8_t b)
{
    int i = 0;

    // load bits MSByte (ISP must also send MSBFirst)
    for (i = 0; i < 24; i += 3) {
      uint8_t x = (b & 0x80) ? ONE : ZERO;
      spi_buf[i] = LCD_BITS(x);
      b <<= 1;
    }
    SPI.transfer(spi_buf, sizeof(spi_buf));
}

// transfer order is probably BRG! and MSBFIRST
void transfer_pixel(rgb_t pixel)
{
    transfer_byte(pixel);
    transfer_byte(pixel >> 16);
    transfer_byte(pixel >> 8);
}

void transfer_pixels(int x, int n)
{
    rgb_t* ptr = &rgb_data[x];
    while(n--) {
	    transfer_pixel(*ptr++);
    }
}

void refresh_pixels()
{
    transfer_pixels(0, NUM_LEDS);
    delayMicroseconds(RESET_US);
}


void setup(void)
{
    Serial.begin (9600);
    Serial.println ("LedController started");

    digitalWrite(SS, HIGH);  // ensure SS stays high for now
    // Put SCK, MOSI, SS pins into output mode
    // also put SCK, MOSI into LOW state, and SS into HIGH state.
    // Then put SPI hardware into Master mode and turn SPI on
    SPI.begin ();

    // Should be 4MHz for LED to get 0.5+5.5us resolution
    SPI.beginTransaction(SPISettings(FREQUENCY, MSBFIRST, SPI_MODE3));

    // spi_buf mostly contains zero's so fill i from start
    for (int i = 0; i < sizeof(spi_buf); i++)
      spi_buf[i] = LCD_BITS(0);
}

static uint8_t scale8(uint8_t color, uint8_t scale)
{
  return (color*scale) >> 8;
}

static rgb_t fade(rgb_t color, uint8_t f)
{
  uint8_t r, g, b;

  r = scale8(rgb_red(color), f);
  g = scale8(rgb_green(color), f);
  b = scale8(rgb_blue(color), f);
  color = make_rgb(r,g,b);
  return color;
}

void step_color_demo()
{
    static const rgb_t palette[8] = 
    { BLACK, RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW, WHITE };
    uint8_t ci = (step >> 8) & 0x7;
    uint8_t f = step & 0xff;  // 0..255 fade
    rgb_t color = palette[ci];
    color = fade(color, f);
    
    fill_pixels(0, color, NUM_LEDS);
}

void move_pixel_demo()
{
    fill_pixels(0, BLUE, NUM_LEDS);
    put_pixel(step % NUM_LEDS, RED);
}

#define HALF_LEN 10 // (NUM_LEDS/2)
#define GUIDE_LEN 2

void lantern_demo()
{
  int half = HALF_LEN-GUIDE_LEN-1;
  int front = NUM_LEDS/2;
  int ss = (step>>1) % half;
  int len = (step & 1) ? GUIDE_LEN+1 : GUIDE_LEN;
  
  fill_pixels(0, OFF, NUM_LEDS);
  fill_pixels(front + ss, GREEN, len);
  fill_pixels(front - ss - (len-1), RED, len);
}

void loop(void)
{
    // enable Slave Select
    digitalWrite(SS, LOW);    // SS is pin 10

    // lantern_demo();
    // move_pixel_demo();
    step_color_demo();
    
    step++;
    // Serial.println(step);

    refresh_pixels();
    
    // disable Slave Select
    digitalWrite(SS, HIGH);
    
    delay(10);
}
