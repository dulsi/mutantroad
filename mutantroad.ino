#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include "TinyConfig.h"
#include "tileset.h"
#include "renegade.h"


#ifdef TINYARCADE_CONFIG
#include "TinyArcade.h"
#endif
#ifdef TINYSCREEN_GAMEKIT_CONFIG
#include "TinyGameKit.h"
#endif

TinyScreen display = TinyScreen(TinyScreenPlus);
unsigned long lastTime;

class Area
{
  public:
    Area(uint8_t x, uint8_t y, const uint8_t *d) : xSize(x), ySize(y), data(d) {}

    uint8_t xSize, ySize;
    const uint8_t *data;
};

const uint8_t roadMap[] = {
2,3,4,4,5,5,5,5,5,5,5,5,6,7,8,8,8,4,4,5,5,5,5,5,5,5,5,6,5,5,5,5,5,5,5,5,6,8,8,8,8,8,8,8,8,8,5,9,5,
10,11,4,4,5,5,5,5,5,5,5,5,6,7,8,8,8,4,4,5,5,5,5,5,5,5,5,6,5,5,5,5,5,5,5,5,5,8,8,8,8,8,8,8,8,8,5,17,5,
13,11,4,4,5,9,5,9,5,9,5,9,6,7,8,8,8,4,4,5,9,5,9,5,9,5,9,6,5,9,5,9,5,9,5,9,6,8,8,8,8,8,8,8,8,8,5,26,5,
15,16,4,4,5,17,5,17,5,17,5,17,6,7,8,8,8,4,4,5,17,5,17,5,17,5,17,6,5,17,5,17,5,17,5,17,6,18,19,19,20,8,21,22,8,8,5,5,5,
23,24,25,4,5,26,27,28,29,26,5,26,30,7,31,32,33,34,4,5,26,35,26,5,26,5,26,6,5,26,5,26,5,26,5,26,6,36,37,37,38,8,39,40,41,42,43,44,45,
46,47,33,34,5,48,5,49,50,51,52,53,54,7,55,56,57,58,4,5,48,54,49,50,51,52,53,6,5,5,5,5,61,19,19,19,62,63,64,64,65,8,66,67,68,69,70,71,6,
72,73,57,58,5,74,45,75,5,76,77,78,6,79,80,80,81,82,4,5,74,45,83,5,18,19,20,6,5,5,45,5,85,37,37,37,86,79,80,80,80,80,87,88,89,87,88,89,80,
112,104,92,82,5,93,5,94,5,95,96,97,6,98,33,99,80,80,100,5,93,5,101,5,63,64,65,6,5,5,5,5,102,64,64,64,103,98,80,80,80,80,80,80,80,80,80,80,80,
112,112,90,105,106,107,107,107,107,107,107,107,107,80,57,58,114,108,109,110,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,111,108,108,108,108,108,108,108,108,108,108,
112,112,90,90,113,114,114,114,114,114,114,114,114,114,73,82,90,90,90,113,108,108,108,108,108,108,108,108,114,114,114,114,114,114,114,114,114,114,108,117,90,90,90,90,90,90,90,90,90,
118,118,118,118,118,118,118,119,118,118,118,118,118,121,120,118,118,118,118,121,121,121,119,121,121,121,121,121,121,121,121,121,121,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,
122,122,122,122,122,122,122,122,122,122,122,122,122,123,124,125,126,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122
};
const Area road(49, 12, roadMap);

class Player
{
  public:
    Player() : x(0), y(15), dir(0), frame(0), attack(0) {}

    const uint8_t *getFrame(int currentY);
    uint8_t getFrameSize();

    int x, y;
    uint8_t dir;
    uint8_t frame;
    uint8_t attack;
};

const uint8_t *Player::getFrame(int currentY)
{
  if (frame < 8)
    return _image_renegade_data + (currentY - y + 15) * 64 *2 + frame * 8 * 2;
  else
    return _image_renegade2_data + (currentY - y + 15) * 24 *2 + (frame - 8) * 12 * 2;
}

uint8_t Player::getFrameSize()
{
  if (frame < 8)
    return 8;
  else
    return 12;
}

Player pc;

class World
{
  public:
    World(const Area *cArea) : currentArea(cArea) { init(); }
    void init();
    void update();
    void draw();
    uint8_t getTile(int x, int y);
    const uint8_t *getTileData(int tile, int y);

    const Area *currentArea;
};

World world(&road);

void World::init()
{
}

void World::update()
{
}


void World::draw()
{
  int startX(0), startY(0);

  startX = pc.x + 52 - 96;
  startY = pc.y + 20 - 64;
  if (startX < 0)
    startX = 0;
  else if (startX + 96 >= currentArea->xSize * 8)
    startX = currentArea->xSize * 8 - 96;
  if (startY < 0)
    startY = 0;
  else if (startY + 64 >= currentArea->ySize * 8)
    startY = currentArea->ySize * 8 - 64;
  display.goTo(0,0);
  display.startData();

  uint8_t lineBuffer[96 * 2];

  for(int lines = 0; lines < 64; ++lines)
  {
    int currentY = startY + lines;
    if ((currentY < 0) || (currentY >= (currentArea->ySize * 8)))
    {
      memset(lineBuffer, 0, 96 * 2);
    }
    else
    {
      int x = 0;
      if (startX < 0)
      {
        x = startX * -1;
        memset(lineBuffer, 0, x * 2);
      }
      int currentTile = getTile(startX + x, currentY);
      if (x == 0)
      {
        int init = (x + startX) % 8;
        if (init != 0)
        {
          if (currentTile == 0)
            memset(lineBuffer + x * 2, 255, (8 - init) * 2);
          else if (currentTile == 255)
            memset(lineBuffer + x * 2, 0, 8 * 2);
          else
            memcpy(lineBuffer + x * 2, getTileData(currentTile, currentY % 8) + (init * 2), (8 - init) * 2);
          x += 8 - init;
        }
      }
      while (x <= 88)
      {
        if (startX + x > ((currentArea->xSize - 1) * 8))
        {
          memset(lineBuffer + x * 2, 0, (96 - x) * 2);
          x = 96;
          break;
        }
        currentTile = getTile(startX + x, currentY);
        if (currentTile == 0)
          memset(lineBuffer + x * 2, 255, 8 * 2);
        else if (currentTile == 255)
          memset(lineBuffer + x * 2, 0, 8 * 2);
        else
          memcpy(lineBuffer + x * 2, getTileData(currentTile, currentY % 8), 8 * 2);
        x += 8;
      }
      if (x != 96)
      {
        int init = 96 - x;
        if (startX + x > ((currentArea->xSize - 1) * 8))
        {
          memset(lineBuffer + x * 2, 0, init * 2);
          x = 96;
        }
        else
        {
          currentTile = getTile(startX + x, currentY);
          if (currentTile == 0)
            memset(lineBuffer + x * 2, 255, init * 2);
        else if (currentTile == 255)
          memset(lineBuffer + x * 2, 0, 8 * 2);
          else
            memcpy(lineBuffer + x * 2, getTileData(currentTile, currentY % 8), init * 2);
        }
      }
/*      for (int n = 0; n < currentArea->countNPC; n++)
      {
        if ((currentY >= npc[n].y) && (currentY < npc[n].y + 8))
        {
          const uint8_t *data = getTileData(currentArea->npc[n].icon, currentY - npc[n].y);
          for (int i = 0; i < 8; i++)
          {
            if (((data[i * 2] != 255) || (data[i * 2 + 1] != 255)) && (startX <= npc[n].x + i) && (startX + 96 > npc[n].x + i))
            {
              lineBuffer[(npc[n].x + i - startX) * 2] = data[i * 2];
              lineBuffer[(npc[n].x + i - startX) * 2 + 1] = data[i * 2 + 1];
            }
          }
        }
      }*/
      if ((currentY > pc.y - 16) && (currentY <= pc.y))
      {
        uint8_t len = pc.getFrameSize();
        const uint8_t *data = pc.getFrame(currentY);
        for (int i = 0; i < len; i++)
        {
          int k = i;
          if (pc.dir == 1)
            k = len - k;
          if ((data[i * 2] != 0xf8) || (data[i * 2 + 1] != 0x1f))
          {
            lineBuffer[(pc.x - startX + k) * 2] = data[i * 2];
            lineBuffer[(pc.x - startX + k) * 2 + 1] = data[i * 2 + 1];
          }
        }
      }
    }
/*    if (state == STATE_MENU)
    {
      choice.draw(lines, lineBuffer);
    }*/
    display.writeBuffer(lineBuffer,96 * 2);
  }
  display.endTransfer();
}

uint8_t World::getTile(int x, int y)
{
  return currentArea->data[x / 8 + ((y / 8) * currentArea->xSize)];
}

const uint8_t *World::getTileData(int tile, int y)
{
  y += ((tile - 1) / 8) * 8;
  int x = ((tile - 1) % 8) * 8;
  return _image_tileset_data + (x + y * (8 * 8 )) * 2;
}

int t = 0;
void setup() {
  arcadeInit();
  display.begin();
  display.setBitDepth(TSBitDepth16);
  display.setBrightness(15);
  display.setFlip(false);

#ifdef TINYARCADE_CONFIG
  USBDevice.init();
  USBDevice.attach();
#endif
  SerialUSB.begin(9600);
  randomSeed(analogRead(0));
  lastTime = millis();
}

void loop() {
  world.update();
  world.draw();
  t = (t + 1) % 8;
  if ((t == 0) || (t == 4))
  {
    uint8_t btn = checkButton(TAButton1 | TAButton2);
    uint8_t joyDir = checkJoystick(TAJoystickUp | TAJoystickDown | TAJoystickLeft | TAJoystickRight);
    if (pc.attack)
    {
      pc.attack--;
      if (pc.attack == 0)
      {
        if (pc.frame == 8)
          pc.frame = 2;
        else
          pc.frame = 0;
      }
    }
    else if (btn > 0)
    {
      if (btn & TAButton1)
      {
        pc.attack = 4;
        if (pc.frame >= 2)
          pc.frame = 9;
        else
          pc.frame = 8;
      }
    }
    else if (joyDir > 0)
    {
      if (pc.frame >= 4)
        pc.frame = 0;
      if ((joyDir & TAJoystickUp) && (pc.y - 16 > 0))
        pc.y--;
      else if ((joyDir & TAJoystickDown) && (pc.y + 1 < world.currentArea->ySize * 8))
        pc.y++;
      if (joyDir & TAJoystickLeft)
      {
        if (pc.x > 0)
          pc.x--;
        pc.dir = 1;
      }
      else if (joyDir & TAJoystickRight)
      {
        if (pc.x + 9 < world.currentArea->xSize * 8)
          pc.x++;
        pc.dir = 0;
      }
      if (t == 0)
        pc.frame = (pc.frame + 1) % 4;
    }
    else if (t == 0)
    {
      if (pc.frame < 4)
      {
        pc.frame = 4;
      }
      else
      {
        pc.frame++;
        if (pc.frame == 8)
          pc.frame = 4;
      }
    }
  }
  unsigned long oldTime = lastTime;
  lastTime = millis();
  if ((lastTime > oldTime) && (lastTime - oldTime < 33))
  {
    delay(33 - (lastTime - oldTime));
  }
}
