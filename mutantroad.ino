#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include "TinyConfig.h"
#include "tileset.h"
#include "renegade.h"
#include "mutant.h"


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

#define OBJTYPE_NONE 0
#define OBJTYPE_PLAYER 1
#define OBJTYPE_MUTANT 2

class Mob
{
  public:
    Mob() : objtype(OBJTYPE_NONE), x(0), y(0), dir(0), frame(0), attack(0) {}

    int getAdjustedX();
    const uint8_t *getFrame(int currentY);
    uint8_t getFrameSize();

    uint8_t objtype;
    int x, y;
    uint8_t dir;
    uint8_t frame;
    uint8_t attack;
};

int Mob::getAdjustedX()
{
  switch(objtype)
  {
    case OBJTYPE_PLAYER:
      if ((frame < 8) || (dir == 0))
        return x;
      else
        return x - 4;
    default:
      return x;
  }
}

const uint8_t *Mob::getFrame(int currentY)
{
  switch(objtype)
  {
    case OBJTYPE_PLAYER:
      if (frame < 8)
        return _image_renegade_data + (currentY - y + 15) * 64 *2 + frame * 8 * 2;
      else
        return _image_renegade2_data + (currentY - y + 15) * 24 *2 + (frame - 8) * 12 * 2;
      break;
    case OBJTYPE_MUTANT:
//      if (frame < 8)
        return _image_mutant_data + (currentY - y + 15) * 32 *2 + frame * 8 * 2;
/*      else
        return _image_renegade2_data + (currentY - y + 15) * 24 *2 + (frame - 8) * 12 * 2;*/
      break;
    default:
      break;
  }
}

uint8_t Mob::getFrameSize()
{
  switch(objtype)
  {
    case OBJTYPE_PLAYER:
    case OBJTYPE_MUTANT:
      if (frame < 8)
        return 8;
      else
        return 12;
      break;
    default:
      break;
  }
}

#define MAX_MOBS 10
#define MOBS_NONE 255

Mob mobs[MAX_MOBS];
uint8_t order[MAX_MOBS];

class World
{
  public:
    World(const Area *cArea) : currentArea(cArea), tick(0) { init(); }
    void init();
    void update();
    void draw();
    uint8_t getTile(int x, int y);
    const uint8_t *getTileData(int tile, int y);

    const Area *currentArea;
    int tick;
};

World world(&road);

void World::init()
{
  mobs[0].objtype = OBJTYPE_PLAYER;
  mobs[0].x = 0;
  mobs[0].y = 78;
  mobs[1].objtype = OBJTYPE_MUTANT;
  mobs[1].x = 50;
  mobs[1].y = 78;
}

void World::update()
{
  for (int i = 0; i < MAX_MOBS; i++)
  {
    switch(mobs[i].objtype)
    {
      case OBJTYPE_PLAYER:
        tick = (tick + 1) % 8;
        if ((tick == 0) || (tick == 4))
        {
          uint8_t btn = checkButton(TAButton1 | TAButton2);
          uint8_t joyDir = checkJoystick(TAJoystickUp | TAJoystickDown | TAJoystickLeft | TAJoystickRight);
          if (mobs[i].attack)
          {
            mobs[i].attack--;
            if (mobs[i].attack == 0)
            {
              if (mobs[i].frame == 8)
                mobs[i].frame = 2;
              else
                mobs[i].frame = 0;
            }
          }
          else if (btn > 0)
          {
            if (btn & TAButton1)
            {
              mobs[i].attack = 4;
              if (mobs[i].frame >= 2)
                mobs[i].frame = 9;
              else
                mobs[i].frame = 8;
            }
          }
          else if (joyDir > 0)
          {
            if (mobs[i].frame >= 4)
              mobs[i].frame = 0;
            if ((joyDir & TAJoystickUp) && (mobs[i].y - 16 > 0))
              mobs[i].y--;
            else if ((joyDir & TAJoystickDown) && (mobs[i].y + 1 < world.currentArea->ySize * 8))
              mobs[i].y++;
            if (joyDir & TAJoystickLeft)
            {
              if (mobs[i].x > 0)
                mobs[i].x--;
              mobs[i].dir = 1;
            }
            else if (joyDir & TAJoystickRight)
            {
              if (mobs[i].x + 9 < world.currentArea->xSize * 8)
                mobs[i].x++;
              mobs[i].dir = 0;
            }
            if (tick == 0)
              mobs[i].frame = (mobs[i].frame + 1) % 4;
          }
          else if (tick == 0)
          {
            if (mobs[i].frame < 4)
            {
              mobs[i].frame = 4;
            }
            else
            {
              mobs[i].frame++;
              if (mobs[i].frame == 8)
                mobs[i].frame = 4;
            }
          }
        }
        break;
      default:
        break;
    }
  }
}


void World::draw()
{
  int startX(0), startY(0);

  startX = mobs[0].x + 52 - 96;
  startY = mobs[0].y + 20 - 64;
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
      int orderidx;
      for (orderidx = 0; orderidx < MAX_MOBS; orderidx++)
      {
        if (mobs[orderidx].objtype == OBJTYPE_NONE)
          order[orderidx] = MOBS_NONE;
        else if (orderidx == 0)
          order[orderidx] = orderidx;
        else
        {
          int w = 0;
          for (w = 0; w < orderidx; w++)
          {
            if (order[w] == MOBS_NONE)
            {
              order[w] = orderidx;
              order[orderidx] = MOBS_NONE;
            }
            if (mobs[orderidx].y < mobs[order[w]].y)
              break;
          }
          if (w != orderidx)
          {
            memmove(order + w + 1, order + w, orderidx - w);
          }
          order[w] = orderidx;
        }
      }
      for (orderidx = 0; orderidx < MAX_MOBS; orderidx++)
      {
        if (order[orderidx] == MOBS_NONE)
          break;
        if ((currentY > mobs[order[orderidx]].y - 16) && (currentY <= mobs[order[orderidx]].y))
        {
          uint8_t len = mobs[order[orderidx]].getFrameSize();
          const uint8_t *data = mobs[order[orderidx]].getFrame(currentY);
          int adjustedX = mobs[order[orderidx]].getAdjustedX();
          for (int i = 0; i < len; i++)
          {
            int k = i;
            if (mobs[order[orderidx]].dir == 1)
              k = len - k;
            if (((data[i * 2] != 0xf8) || (data[i * 2 + 1] != 0x1f)) &&
              (adjustedX - startX + k >= 0) && (adjustedX - startX + k < 96))
            {
              lineBuffer[(adjustedX - startX + k) * 2] = data[i * 2];
              lineBuffer[(adjustedX - startX + k) * 2 + 1] = data[i * 2 + 1];
            }
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
  unsigned long oldTime = lastTime;
  lastTime = millis();
  if ((lastTime > oldTime) && (lastTime - oldTime < 33))
  {
    delay(33 - (lastTime - oldTime));
  }
}
