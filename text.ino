
#define PLAYER_1 "P1"
#define PLAYER_2 "P2"





#define ofset_1 9
#define ofset_2 40
// button
#define RESET_BUTTON 32
#define RIGHT_PLUS_BUTTON 2
#define RIGHT_MINUS_BUTTON 22
#define LEFT_PLUS_BUTTON 18
#define LEFT_MINUS_BUTTON 33

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1    

#include <Adafruit_GFX.h>   // Core graphics library
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

bool forward = true;
uint8_t counter_x = 0, counter_total = 1;
uint8_t dot[32]={0}, stop[32]={0}, pos[32]={0}, firework_cnts[32]={0};

uint8_t random_color_code1 = rand();
uint8_t random_color_code2 = rand();
uint8_t random_color_code3 = rand();

unsigned long tag = 0, interval = 60000, firework_cnt = 0;

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;


  // Total number of panels chained one to another
 
//MatrixPanel_I2S_DMA dma_display;
MatrixPanel_I2S_DMA *dma_display = nullptr;
uint16_t color_background = 0;

uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);
uint16_t pixelColumns[18][4];




uint8_t fireworks[8][9]={
{0B000000000,0B000000000,0B000000000,0B000010000,0B000111000,0B000010000,0B000000000,0B000000000,0B000000000},
{0B000000000,0B000000000,0B000010000,0B000111000,0B001101100,0B000111000,0B000010000,0B000000000,0B000000000},
{0B000000000,0B000010000,0B001010100,0B000111000,0B011101110,0B000111000,0B001010100,0B000010000,0B000000000},
{0B000010000,0B010010010,0B001010100,0B000101000,0B111000111,0B000101000,0B001010100,0B010010010,0B000010000},
{0B100010001,0B010010010,0B001000100,0B000000000,0B110000011,0B000000000,0B001000100,0B010010010,0B100010001},
{0B100010001,0B010000010,0B000000000,0B000000000,0B100000001,0B000000000,0B000000000,0B010000010,0B100010001},
{0B100010001,0B000000000,0B000000000,0B000000000,0B100000001,0B000000000,0B000000000,0B000000000,0B100010001},
{0B000000000,0B000000000,0B000000000,0B000000000,0B000000000,0B000000000,0B000000000,0B000000000,0B000000000}
};

void displayFirework(uint8_t offset_x, uint8_t offset_y, uint8_t value) {
  
  // ROWs
  for (int y = 0; y < 9; y++) {

    uint8_t shifter = fireworks[value][y];
    // COLUMNs
    for (int x = 8; x >= 0; x--) {
      if (shifter & 1) dma_display->drawPixel(x+offset_x,y+offset_y, dma_display->color565(random_color_code1,random_color_code2,random_color_code3));    
      shifter = shifter>>1; 
    }
  } 
}
const uint8_t kongtext4pt7bBitmaps[] PROGMEM = {
  0xFF, 0x30, 0x99, 0x90, 0x4B, 0xF4, 0xBF, 0x48, 0x31, 0xF8, 0x1E, 0x07, 
  0xE3, 0x00, 0x62, 0xCC, 0x30, 0xC3, 0x0C, 0xF1, 0x80, 0x38, 0x6C, 0x3B, 
  0x6E, 0xC6, 0x7B, 0x6F, 0x00, 0x7B, 0x6C, 0xC0, 0xCD, 0xB7, 0x80, 0x6D, 
  0x26, 0x0A, 0x22, 0x82, 0x00, 0x30, 0xCF, 0xCC, 0x30, 0x6F, 0x00, 0xFC, 
  0xF0, 0x19, 0x99, 0x88, 0x00, 0x7B, 0x3D, 0xFB, 0xCD, 0xE0, 0x6E, 0x66, 
  0x6F, 0x7B, 0x30, 0xDE, 0xC3, 0xF0, 0x7B, 0x31, 0x83, 0xCD, 0xE0, 0xC3, 
  0x6D, 0xBF, 0x18, 0x60, 0xFE, 0x0F, 0x83, 0xCD, 0xE0, 0x7B, 0x0F, 0xB3, 
  0xCD, 0xE0, 0xFC, 0x61, 0x8C, 0x30, 0xC0, 0x7B, 0x37, 0xB3, 0xCD, 0xE0, 
  0x7B, 0x3C, 0xDF, 0x0D, 0xE0, 0xF3, 0xC0, 0xF3, 0xE0, 0x36, 0xC6, 0x30, 
  0xFF, 0xC1, 0xFF, 0x80, 0xC6, 0x36, 0xC0, 0x7B, 0x30, 0xCE, 0x00, 0xC0, 
  0x7B, 0x3D, 0xF6, 0xC1, 0xF0, 0x7B, 0x3C, 0xFF, 0xCF, 0x30, 0xFB, 0x3F, 
  0xB3, 0xCF, 0xE0, 0x7B, 0x3C, 0x30, 0xCD, 0xE0, 0xFB, 0x3C, 0xF3, 0xCF, 
  0xE0, 0xFF, 0x0F, 0x30, 0xC3, 0xF0, 0xFF, 0x0F, 0x30, 0xC3, 0x00, 0x7B, 
  0x3C, 0x37, 0xCD, 0xE0, 0xCF, 0x3F, 0xF3, 0xCF, 0x30, 0xF6, 0x66, 0x6F, 
  0x7C, 0x61, 0x86, 0xD9, 0xC0, 0xCF, 0x6F, 0x3C, 0xDB, 0x30, 0xC6, 0x31, 
  0x8C, 0x7C, 0xC7, 0xDF, 0xFE, 0xBC, 0x78, 0xC0, 0xCF, 0xBF, 0xFF, 0xDF, 
  0x30, 0x7B, 0x3C, 0xF3, 0xCD, 0xE0, 0xFB, 0x3C, 0xFE, 0xC3, 0x00, 0x7B, 
  0x3C, 0xF3, 0xD9, 0xD0, 0xFB, 0x3C, 0xFE, 0xCF, 0x30, 0x7B, 0x17, 0x83, 
  0xCD, 0xE0, 0xFC, 0xC3, 0x0C, 0x30, 0xC0, 0xCF, 0x3C, 0xF3, 0xCD, 0xE0, 
  0xCF, 0x3C, 0xF3, 0x78, 0xC0, 0xC7, 0x8F, 0x5F, 0xFE, 0xF8, 0xC0, 0xCD, 
  0xE3, 0x1E, 0xCE, 0x10, 0xCF, 0x37, 0x8C, 0x30, 0xC0, 0xFC, 0x63, 0x18, 
  0xC3, 0xF0, 0xFB, 0x6D, 0xC0, 0xC3, 0x0C, 0x30, 0x80, 0xED, 0xB7, 0xC0, 
  0x76, 0xC0, 0xFC, 0xD9, 0x80, 0x7D, 0x9B, 0x36, 0x67, 0x60, 0xC3, 0x0F, 
  0xB3, 0xCF, 0xE0, 0x7B, 0x3C, 0x33, 0x78, 0x0C, 0x37, 0xF3, 0xCD, 0xF0, 
  0x7B, 0x3F, 0xB0, 0x7C, 0x3B, 0x3C, 0xC6, 0x30, 0x7F, 0x3C, 0xDF, 0x0D, 
  0xE0, 0xC3, 0x0F, 0xB3, 0xCF, 0x30, 0xCF, 0xF0, 0x18, 0x06, 0x31, 0xED, 
  0xE0, 0xC3, 0x3F, 0xBC, 0xFB, 0x30, 0xCC, 0xCC, 0xC7, 0xED, 0xAF, 0x5E, 
  0xBD, 0x60, 0xFB, 0x3C, 0xF3, 0xCC, 0x7B, 0x3C, 0xF3, 0x78, 0xFB, 0x3C, 
  0xFE, 0xC3, 0x00, 0x7D, 0x9B, 0x33, 0xE0, 0xE1, 0x80, 0xDB, 0xBC, 0x30, 
  0xC0, 0x7B, 0x07, 0x83, 0xF8, 0x63, 0xE6, 0x18, 0x6C, 0xE0, 0xCD, 0x9B, 
  0x36, 0x67, 0x60, 0xCF, 0x3C, 0xDE, 0x30, 0xD7, 0xAF, 0x5E, 0xB6, 0xE0, 
  0xCF, 0xF7, 0xBF, 0xCC, 0xCF, 0x3C, 0xDF, 0x0D, 0xE0, 0xFC, 0x63, 0x18, 
  0xFC, 0x19, 0xB8, 0x63, 0x0C, 0xFF, 0xF0, 0xC3, 0x0E, 0xC6, 0x60, 0x62, 
  0xD1, 0x80
};

const GFXglyph kongtext4pt7bGlyphs[] PROGMEM = {
  {     0,   0,   0,   7,   -1,    1 }   // ' '
 ,{     0,   2,   6,   8,    3,   -5 }   // '!'
 ,{     2,   4,   3,   8,    2,   -5 }   // '"'
 ,{     4,   6,   5,   8,    1,   -5 }   // '#'
 ,{     8,   6,   7,   8,    1,   -6 }   // '$'
 ,{    14,   7,   7,   8,    0,   -6 }   // '%'
 ,{    21,   8,   6,   8,    0,   -5 }   // '&'
 ,{    27,   3,   3,   8,    3,   -5 }   // '''
 ,{    29,   3,   6,   8,    3,   -5 }   // '('
 ,{    32,   3,   6,   8,    3,   -5 }   // ')'
 ,{    35,   7,   6,   7,    0,   -6 }   // '*'
 ,{    41,   6,   5,   8,    1,   -5 }   // '+'
 ,{    45,   3,   3,   8,    3,   -2 }   // ','
 ,{    47,   6,   1,   8,    1,   -3 }   // '-'
 ,{    48,   2,   2,   8,    3,   -1 }   // '.'
 ,{    49,   5,   5,   8,    1,   -5 }   // '/'
 ,{    53,   6,   6,   7,    0,   -5 }   // '0'
 ,{    58,   4,   6,   7,    1,   -5 }   // '1'
 ,{    61,   6,   6,   7,    0,   -5 }   // '2'
 ,{    66,   6,   6,   7,    0,   -5 }   // '3'
 ,{    71,   6,   6,   7,    0,   -5 }   // '4'
 ,{    76,   6,   6,   7,    0,   -5 }   // '5'
 ,{    81,   6,   6,   7,    0,   -5 }   // '6'
 ,{    86,   6,   6,   7,    0,   -5 }   // '7'
 ,{    91,   6,   6,   7,    0,   -5 }   // '8'
 ,{    96,   6,   6,   7,    0,   -5 }   // '9'
 ,{   101,   2,   5,   8,    3,   -5 }   // ':'
 ,{   103,   2,   6,   8,    3,   -5 }   // ';'
 ,{   105,   4,   5,   8,    2,   -5 }   // '<'
 ,{   108,   5,   5,   8,    2,   -5 }   // '='
 ,{   112,   4,   5,   8,    2,   -5 }   // '>'
 ,{   115,   6,   6,   8,    1,   -5 }   // '?'
 ,{   120,   6,   6,   7,    0,   -5 }   // '@'
 ,{   125,   6,   6,   7,    0,   -5 }   // 'A'
 ,{   130,   6,   6,   7,    0,   -5 }   // 'B'
 ,{   135,   6,   6,   7,    0,   -5 }   // 'C'
 ,{   140,   6,   6,   7,    0,   -5 }   // 'D'
 ,{   145,   6,   6,   7,    0,   -5 }   // 'E'
 ,{   150,   6,   6,   7,    0,   -5 }   // 'F'
 ,{   155,   6,   6,   7,    0,   -5 }   // 'G'
 ,{   160,   6,   6,   7,    0,   -5 }   // 'H'
 ,{   165,   4,   6,   5,    0,   -5 }   // 'I'
 ,{   168,   6,   6,   7,    0,   -5 }   // 'J'
 ,{   173,   6,   6,   7,    0,   -5 }   // 'K'
 ,{   178,   5,   6,   6,    0,   -5 }   // 'L'
 ,{   182,   7,   6,   8,    0,   -5 }   // 'M'
 ,{   188,   6,   6,   7,    0,   -5 }   // 'N'
 ,{   193,   6,   6,   7,    0,   -5 }   // 'O'
 ,{   198,   6,   6,   7,    0,   -5 }   // 'P'
 ,{   203,   6,   6,   7,    0,   -5 }   // 'Q'
 ,{   208,   6,   6,   7,    0,   -5 }   // 'R'
 ,{   213,   6,   6,   7,    0,   -5 }   // 'S'
 ,{   218,   6,   6,   7,    0,   -5 }   // 'T'
 ,{   223,   6,   6,   7,    0,   -5 }   // 'U'
 ,{   228,   6,   6,   7,    0,   -5 }   // 'V'
 ,{   233,   7,   6,   8,    0,   -5 }   // 'W'
 ,{   239,   6,   6,   7,    0,   -5 }   // 'X'
 ,{   244,   6,   6,   7,    0,   -5 }   // 'Y'
 ,{   249,   6,   6,   7,    0,   -5 }   // 'Z'
 ,{   254,   3,   6,   8,    3,   -5 }   // '['
 ,{   257,   5,   5,   8,    1,   -5 }   // '\'
 ,{   261,   3,   6,   8,    3,   -5 }   // ']'
 ,{   264,   5,   2,   8,    2,   -6 }   // '^'
 ,{   266,   6,   1,   8,    1,    0 }   // '_'
 ,{   267,   3,   3,   8,    2,   -5 }   // '`'
 ,{   269,   7,   5,   8,    0,   -4 }   // 'a'
 ,{   274,   6,   6,   7,    0,   -5 }   // 'b'
 ,{   279,   6,   5,   7,    0,   -4 }   // 'c'
 ,{   283,   6,   6,   7,    0,   -5 }   // 'd'
 ,{   288,   6,   5,   7,    0,   -4 }   // 'e'
 ,{   292,   5,   6,   6,    0,   -5 }   // 'f'
 ,{   296,   6,   6,   7,    0,   -4 }   // 'g'
 ,{   301,   6,   6,   7,    0,   -5 }   // 'h'
 ,{   306,   2,   6,   3,    0,   -5 }   // 'i'
 ,{   308,   5,   7,   6,    0,   -5 }   // 'j'
 ,{   313,   6,   6,   7,    0,   -5 }   // 'k'
 ,{   318,   4,   6,   5,    0,   -5 }   // 'l'
 ,{   321,   7,   5,   8,    0,   -4 }   // 'm'
 ,{   326,   6,   5,   7,    0,   -4 }   // 'n'
 ,{   330,   6,   5,   7,    0,   -4 }   // 'o'
 ,{   334,   6,   6,   7,    0,   -4 }   // 'p'
 ,{   339,   7,   6,   8,    0,   -4 }   // 'q'
 ,{   345,   6,   5,   7,    0,   -4 }   // 'r'
 ,{   349,   6,   5,   7,    0,   -4 }   // 's'
 ,{   353,   6,   6,   7,    0,   -5 }   // 't'
 ,{   358,   7,   5,   8,    0,   -4 }   // 'u'
 ,{   363,   6,   5,   7,    0,   -4 }   // 'v'
 ,{   367,   7,   5,   8,    0,   -4 }   // 'w'
 ,{   372,   6,   5,   7,    0,   -4 }   // 'x'
 ,{   376,   6,   6,   7,    0,   -4 }   // 'y'
 ,{   381,   6,   5,   7,    0,   -4 }   // 'z'
 ,{   385,   5,   6,   8,    1,   -5 }   // '{'
 ,{   389,   2,   6,   8,    3,   -5 }   // '|'
 ,{   391,   5,   6,   8,    2,   -5 }   // '}'
 ,{   395,   6,   3,   8,    1,   -4 }   // '~'
};

uint8_t numbers[10][16]={
  {0B01111110,0B10111101,0B11000011,0B11000011,0B11000011,0B11000011,0B10000001,0B00000000,0B10000001,0B11000011,0B11000011,0B11000011,0B11000011,0B11000011,0B10111101,0B01111110}, // 0
  {0B00000001,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B00000001,0B00000000,0B00000001,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B00000001}, // 1
  {0B11111110,0B01111101,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B01111101,0B10111110,0B11000000,0B11000000,0B11000000,0B11000000,0B11000000,0B10111110,0B01111111}, // 2
  {0B11111110,0B01111101,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B01111110,0B01111110,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B01111101,0B11111110}, // 3
  {0B10000001,0B11000011,0B11000011,0B11000011,0B11000011,0B11000011,0B11000011,0B10111110,0B01111110,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B00000001}, // 4
  {0B01111110,0B10111100,0B11000000,0B11000000,0B11000000,0B11000000,0B11000000,0B10111110,0B01111101,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B00111101,0B01111110}, // 5
  {0B01111110,0B10111100,0B11000000,0B11000000,0B11000000,0B11000000,0B11000000,0B01111110,0B01111101,0B11000011,0B11000011,0B11000011,0B11000011,0B11000011,0B10111101,0B01111110}, // 6
  {0B11111110,0B01111101,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B00000001,0B00000000,0B00000001,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B00000001}, // 7
  {0B01111110,0B10111101,0B11000011,0B11000011,0B11000011,0B11000011,0B11000011,0B01111110,0B01111110,0B11000011,0B11000011,0B11000011,0B11000011,0B11000011,0B10111101,0B01111110}, // 8
  {0B01111110,0B10111101,0B11000011,0B11000011,0B11000011,0B11000011,0B11000011,0B10111110,0B01111110,0B00000011,0B00000011,0B00000011,0B00000011,0B00000011,0B00111101,0B01111110} // 9
  };

class MyServerCallbacks: public BLEServerCallbacks {

    void onConnect(BLEServer* pServer) {

      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

bool forward1 = true, forward2 = true;
uint8_t text1_pos = 0, text1_pos_max = 31 - 21 + 2 + 21; // 3 ky tu, 2 bien + 3 KY TU
uint8_t text2_pos = 33, text2_pos_max = 33 + (33 - 28) + 2 + 28; // 4 ky tu + 2 bien + 4 ky tu




void drawText(int score1, int score2, int brack)
{
  // dma_display->clearScreen();
    //  drawNewText(2,2, false, 8, 8, myRED, String(score1), 0);
    // drawNewText(0,0, false, 40, 2, myGREEN, String(score2), 0);
      color_background = dma_display->color565(0,0,0);
  dma_display->flipDMABuffer(); 
  dma_display->fillScreen(color_background);
  dma_display->drawLine(0, 9, dma_display->width(), 9, myGREEN);

  dma_display->fillRect(0, 0, 32, 9, dma_display->color444(0, 0, 0)); // MASK 2
 

    drawNewText(1,1, false, ofset_1, 1, myRED, PLAYER_1, 0);
    drawNewText(1,1, false, ofset_2, 1, myRED, PLAYER_2, 0);

    drawNewText(2,2, false, 5, 12, myRED, String(score1), 0);
    drawNewText(2,2, false, 38, 12, myRED, String(score2), 0);
 

}
byte bufferArray[3]={0,0, 0};

class MyCallbacks: public BLECharacteristicCallbacks {

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      int length = rxValue.length();
      if (length > 0) {
        Serial.print("Received Value: ");
        for (int i = 0; i < length; i++) {

          bufferArray[i] =  rxValue[i];
          Serial.println(bufferArray[i], HEX);
        }
      }
    }
};

  uint8_t wheelval = 0;

uint8_t stop_latch = 0, pos_latch = 0;
void home() {

  color_background = dma_display->color565(0,0,0);
  dma_display->flipDMABuffer(); 
  dma_display->fillScreen(color_background);
  // uint8_t w = 0;
  // const char *str = "TECHBOT";
  // for (w=0; w<strlen(str); w++) {
  //   dma_display->setTextColor(colorWheel((w*32)+wheelval));
  //   dma_display->print(str[w]);
  // }


    // dot display 
    if (dot[0] == 0) {
      stop[0] = random(0, 31);
      pos[0] = random(0, 63);

      stop_latch = stop[0];
      pos_latch = pos[0];
    }
    if (dot[0] < stop[0]) {
      dot[0]++;
    } else {
      dot[0] = 0;
    }

    if (forward == true) {

      dma_display->drawRect(counter_x, 30, 2, 2, dma_display->color444(15, 15, 0));
      dma_display->drawRect(66 - (2 * counter_total), 30, 2 * counter_total, 2, dma_display->color444(15, 15, 0));
    } else {
      
      dma_display->drawRect(0, 30, 2 * (32 - counter_total), 2, dma_display->color444(15, 15, 0));
      dma_display->drawRect(counter_x, 30, 2, 2, dma_display->color444(15, 15, 0));
    }
    
     drawNewText( 1,1, true, 20, 2, myGREEN, "BRES", 0);
     drawNewTextWithWheel(wheelval, 2,2, true, 3, 12, myRED, "SCORE", 0);

    // dma_display->drawRect(pos[0], 31 - dot[0], 1, 1, dma_display->color444(15, 15, 0));
    
     wheelval +=1;
    displayFirework(pos_latch, stop_latch, firework_cnt);


    delay(20); 


}
uint16_t colorWheel(uint8_t pos) {
  if(pos < 85) {
    return dma_display->color565(pos * 3, 255 - pos * 3, 0);
  } else if(pos < 170) {
    pos -= 85;
    return dma_display->color565(255 - pos * 3, 0, pos * 3);
  } else {
    pos -= 170;
    return dma_display->color565(0, pos * 3, 255 - pos * 3);
  }
}
void drawNewText(uint8_t text_size_x, int8_t text_size_y, bool wrap_flg, uint8_t x, uint8_t y, uint16_t color, String input, uint8_t rotation_value) {

  if (rotation_value > 0)  dma_display->setRotation(rotation_value); 
  dma_display->setTextSize(text_size_x, text_size_y);     // size 1 == 8 pixels high
  dma_display->setTextWrap(wrap_flg); // Don't wrap at end of line - will do ourselves

  dma_display->setCursor(x, y);    // start at top left, with 8 pixel of spacing
  dma_display->setTextColor(color);
  // uint8_t w = 0;
  // const char *str = input;
  // for (w=0; w<strlen(str); w++) {
  //   dma_display->setTextColor(colorWheel((w*32)+colorWheelOffset));
  //   dma_display->print(str[w]);
  // }
   dma_display->print(input);
}

void drawNewTextWithWheel(uint8_t colorWheelOffset, uint8_t text_size_x, int8_t text_size_y, bool wrap_flg, uint8_t x, uint8_t y, uint16_t color, char *str, uint8_t rotation_value) {

  if (rotation_value > 0)  dma_display->setRotation(rotation_value); 
  dma_display->setTextSize(text_size_x, text_size_y);     // size 1 == 8 pixels high
  dma_display->setTextWrap(wrap_flg); // Don't wrap at end of line - will do ourselves

  dma_display->setCursor(x, y);    // start at top left, with 8 pixel of spacing
  dma_display->setTextColor(color);
  uint8_t w = 0;
  for (w=0; w<strlen(str); w++) {
    dma_display->setTextColor(colorWheel((w*32)+colorWheelOffset));
    dma_display->print(str[w]);
  }
  //  dma_display->print(input);
}
void displayNumber(uint8_t offset_x, uint8_t offset_y, uint8_t value) {
  // ROWs
  for (int y = 0+offset_y; y < 16+offset_y; y++) {

    uint8_t shifter = numbers[value][y];
    // COLUMNs
    for (int x = 7+offset_x; x >= 0; x--) {
      if (shifter & 1) dma_display->drawPixel(x,y, dma_display->color565((uint8_t)pixelColumns[y][1], (uint8_t)pixelColumns[y][2], (uint8_t)pixelColumns[y][3]));    
      shifter = shifter>>1; 
    }
  } 
}

void setup() {
  Serial.begin(115200);
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   // module width
    PANEL_RES_Y,   // module height
    PANEL_CHAIN    // Chain length
  );

  mxconfig.double_buff = true;
  mxconfig.latch_blanking = 4;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  mxconfig.clkphase = false;

  // Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  
  dma_display->begin();
  dma_display->setBrightness8(120); // range is 0-255, 0 - 0%, 255 - 100%
  dma_display->clearScreen();

    home();

    pinMode(RESET_BUTTON, INPUT_PULLUP);
 
    pinMode(RIGHT_PLUS_BUTTON, INPUT_PULLUP);
    pinMode(RIGHT_MINUS_BUTTON, INPUT_PULLUP);

    // LEFT_PLUS_BUTTON
    pinMode(LEFT_PLUS_BUTTON, INPUT_PULLUP);
    pinMode(LEFT_MINUS_BUTTON, INPUT_PULLUP);

    // RIGHT_MINUS_BUTTON

  // Create the BLE Device
  BLEDevice::init("BRES::Score");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_UUID_TX,
										BLECharacteristic::PROPERTY_NOTIFY
									);
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
											 CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
										);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  
}

bool resetFlg = false;
bool rightPlusFlg = false;
bool rightMinusFlg = false;
bool leftPlusFlg = false;
bool leftMinusFlg = false;

bool manualMode = false;

int player1 = 0;
int player2 = 0;

void loop() {


// RIGHT_MINUS_BUTTON
     int rstBtn = digitalRead(RESET_BUTTON);
     int rightPlusBtn = digitalRead(RIGHT_PLUS_BUTTON);
     int rightMinusBtn = digitalRead(RIGHT_MINUS_BUTTON);

     int leftPlusBtn = digitalRead(LEFT_PLUS_BUTTON);
     int leftMinusBtn = digitalRead(LEFT_MINUS_BUTTON);

    if (micros() - tag > interval) {
      tag = micros();

      if (forward1 == true) {
        if (text1_pos < text1_pos_max) text1_pos++;
        else {
          forward1 = false;
          text1_pos_max = 0; 
        }
      } else {
        
        if (text1_pos > text1_pos_max) text1_pos--;
        else {
          forward1 = true;
          text1_pos_max = 31 - 21 + 2 + 21;
        }
      }
      
      if (forward2 == true) {
        if (text2_pos < text2_pos_max) text2_pos++;
        else {
          forward2 = false;
          text2_pos_max = 33;
        }
      } else {
        
        if (text2_pos > text2_pos_max) text2_pos--;
        else {
          forward2 = true;
          text2_pos_max = 33 + (33 - 28) + 2 + 28;
        }
      }
      
      
      
      firework_cnt++;
      if (firework_cnt == 8) {
        firework_cnt = 0;
        random_color_code1 = rand();
        random_color_code2 = rand();
        random_color_code3 = rand();
      }
      if (forward == true) {
        counter_x++;
        if (counter_x > (63 - (counter_total * 2))) {
          counter_x = 0;
          counter_total++;
          if (counter_total == 32)  {
            forward = false;
            counter_total = 1;
            counter_x = 62;
          }
        }
      }
      else {
        if (counter_x < 64) counter_x++;
        else {
          counter_x = 64 - (2 *counter_total);
          counter_total++;
          if (counter_total == 32)  {
              forward = true;
              counter_total = 1;
          }
        }
        
       
      }
    }
  
  
    if (deviceConnected) {
        pTxCharacteristic->setValue(&txValue, 2);
        pTxCharacteristic->notify();
        txValue++;
        manualMode = false;
    
		    delay(10); // bluetooth stack will go into congestion, if too many packets are sent
  	} 

    if (!deviceConnected) {
      /// todo here  

     

      if (rstBtn == 0  && !resetFlg) {
        resetFlg = true;
        Serial.println("RESET  OFF");
      }
      else if (rstBtn == 1  && resetFlg  ) {
        resetFlg = false;
       Serial.println("Reset ON");
       manualMode = !manualMode;
       if (!manualMode) {
        player1 = 0;
        player2 = 0;
       }
       


      }

      if (manualMode) {
          if (rightPlusBtn == 0  && !rightPlusFlg) {
              rightPlusFlg = true;
             Serial.println("Right plus OFF");
            }
            else if (rightPlusBtn == 1  && rightPlusFlg  ) {
              rightPlusFlg = false; 
              player2 += 1;

              Serial.println("Right plus ON");
            }

             if (rightMinusBtn == 0  && !rightMinusFlg) {
              rightMinusFlg = true;
             Serial.println("Right minus OFF");
            }
            else if (rightMinusBtn == 1  && rightMinusFlg  ) {
              rightMinusFlg = false; 
              if (player2 > 0) {
                player2 -= 1;
              }
              Serial.println("Right minus ON");
            }


            if (leftPlusBtn == 0  && !leftPlusFlg) {
              leftPlusFlg = true;
             Serial.println("left plus OFF");
            }
            else if (leftPlusBtn == 1  && leftPlusFlg  ) {
              leftPlusFlg = false; 
              player1 += 1;
              Serial.println("left plus ON");
            }


            if (leftMinusBtn == 0  && !leftMinusFlg) {
              leftMinusFlg = true;
             Serial.println("left minus OFF");
            }
            else if (leftMinusBtn == 1  && leftMinusFlg  ) {
              leftMinusFlg = false; 
              if (player1 > 0) {
                player1 -= 1;
              }
              Serial.println("left minus ON");
            }

            drawText(player1, player2, 0);


       } else {
        home();
       }

       
   


    } else {
      drawText(bufferArray[0], bufferArray[1], bufferArray[2]);
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {

        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); 
        // restart advertising
        Serial.print("device not Connected ");
        bufferArray[0]= 0;
                bufferArray[1]= 0;
        bufferArray[2]= 0;

        Serial.println("start advertising.....");
        home();
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
		// do stuff here on connecting
        Serial.println("deviceConnected");
        pTxCharacteristic->setValue("Hello");
        oldDeviceConnected = deviceConnected;
    }
}



















