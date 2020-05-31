#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include "TinyConfig.h"
#include "tileset.h"
#include "renegade.h"
#include "mutant.h"
#include "ui.h"


#ifdef TINYARCADE_CONFIG
#include "TinyArcade.h"
#endif
#ifdef TINYSCREEN_GAMEKIT_CONFIG
#include "TinyGameKit.h"
#endif

TinyScreen display = TinyScreen(TinyScreenPlus);
unsigned long lastTime;

#define MAX_FLAGS 50

#define CMD_SPAWNMUTANT 1

class AreaScript
{
  public:
    AreaScript(int x1, uint8_t f, uint8_t c, uint8_t a1, int a2, int a3) : x(x1), flag(f), cmd(c), arg1(a1), arg2(a2), arg3(a3) {}

    bool run() const;

    int x;
    uint8_t flag;
    uint8_t cmd;
    uint8_t arg1;
    int arg2;
    int arg3;
};

class Area
{
  public:
    Area(uint8_t x, uint8_t y, const uint8_t *d, const AreaScript *s) : xSize(x), ySize(y), data(d), script(s) {}

    uint8_t xSize, ySize;
    const uint8_t *data;
    const AreaScript *script;
};

const uint8_t tilesetCollision[] = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 0,
  1, 1, 1, 1, 1, 1, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 1, 1, 1, 1, 1,
  1, 0, 1, 1, 1, 1, 1, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1
};

const AreaScript roadScript[] = {
  AreaScript(100, 0, CMD_SPAWNMUTANT, 0, 100, 80),
  AreaScript(115, 1, CMD_SPAWNMUTANT, 0, 110, 70),
  AreaScript(0, 0, 0, 0, 0, 0),
};
const uint8_t roadMap[] = {
2,3,4,4,5,5,5,5,5,5,5,5,6,7,8,8,8,4,4,5,5,5,5,5,5,5,5,6,5,5,5,5,5,5,5,5,6,8,8,8,8,8,8,8,8,8,5,9,5,
10,11,4,4,5,5,5,5,5,5,5,5,6,7,8,8,8,4,4,5,5,5,5,5,5,5,5,6,5,5,5,5,5,5,5,5,5,8,8,8,8,8,8,8,8,8,5,17,5,
13,11,4,4,5,9,5,9,5,9,5,9,6,7,8,8,8,4,4,5,9,5,9,5,9,5,9,6,5,9,5,9,5,9,5,9,6,8,8,8,8,8,8,8,8,8,5,26,5,
15,16,4,4,5,17,5,17,5,17,5,17,6,7,8,8,8,4,4,5,17,5,17,5,17,5,17,6,5,17,5,17,5,17,5,17,6,18,19,19,20,8,21,22,8,8,5,5,5,
23,24,25,4,5,26,27,28,29,26,5,26,30,7,31,32,33,34,4,5,26,35,26,5,26,5,26,6,5,26,5,26,5,26,5,26,6,36,37,37,38,8,39,40,41,42,43,44,45,
46,47,33,34,5,48,5,49,50,51,52,53,54,7,55,56,57,58,4,5,48,54,49,50,51,52,53,6,5,5,5,5,61,19,19,19,62,63,64,64,65,8,66,67,68,69,70,71,6,
72,73,57,58,5,74,45,75,5,76,77,78,6,79,80,80,81,82,4,5,74,45,83,5,18,19,20,6,5,5,45,5,85,37,37,37,86,79,80,80,80,80,87,88,89,87,88,89,80,
112,104,92,82,5,93,5,94,5,95,96,97,6,98,80,80,80,80,100,5,93,5,101,5,63,64,65,6,5,5,5,5,102,64,64,64,103,98,80,80,80,80,80,80,80,80,80,80,80,
112,112,90,105,106,107,107,107,107,107,107,107,107,80,111,108,114,108,109,110,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,111,108,108,108,108,108,108,108,108,108,108,
112,112,90,90,113,114,114,114,114,114,114,114,114,114,108,117,90,90,90,113,108,108,108,108,108,108,108,108,114,114,114,114,114,114,114,114,114,114,108,117,90,90,90,90,90,90,90,90,90,
118,118,118,118,118,118,118,119,118,118,118,118,118,121,120,118,118,118,118,121,121,121,119,121,121,121,121,121,121,121,121,121,121,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,
122,122,122,122,122,122,122,122,122,122,122,122,122,123,124,125,126,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122
};
const Area road(49, 12, roadMap, roadScript);

#define STATE_GAME 1
#define STATE_GAMEOVER 2

#define PAUSE_GAMEOVER 100

int state = STATE_GAME;
int pause = 0;

#define OBJTYPE_NONE 0
#define OBJTYPE_PLAYER 1
#define OBJTYPE_MUTANT 2

#define PLFRAME_GETUP 8
#define PLFRAME_HURT 9
#define PLFRAME_FALLING 10
#define PLFRAME_GROUND 11
#define PLFRAME_PUNCH 12
#define PLFRAME_KICK 14

#define PAUSE_PUNCH 16
#define PAUSE_KICK 20
#define PAUSE_HURT 16
#define PAUSE_GROUND 100

#define PLAYER_HEALTH 8

#define MUTANTSTATE_APPROACH 0
#define MUTANTSTATE_ATTACK 1
#define MUTANTSTATE_BACKOFF 2

#define MUTANTSTATEDATA_APPROACH 10
#define MUTANTSTATEDATA_ATTACK 5
#define MUTANTSTATEDATA_BACKOFF 15

class Mob
{
  public:
    Mob() : objtype(OBJTYPE_NONE), x(0), y(0), dir(0), frame(0), pause(0), health(0), knockdown(0), state(0), data(0) {}

    int getAdjustedX();
    const uint8_t *getFrame(int currentY);
    uint8_t getFrameSize();

    uint8_t objtype;
    int x, y;
    uint8_t dir;
    uint8_t frame;
    uint8_t pause;
    uint8_t health;
    uint8_t knockdown;
    uint8_t state;
    uint8_t data;
};

int Mob::getAdjustedX()
{
  switch(objtype)
  {
    case OBJTYPE_PLAYER:
    case OBJTYPE_MUTANT:
      if ((frame <= PLFRAME_HURT) || ((dir == 0) && (frame >= PLFRAME_PUNCH)))
        return x;
      else if (frame >= PLFRAME_PUNCH)
        return x - 4;
      else if ((dir == 1) && ((frame == PLFRAME_FALLING) || (frame == PLFRAME_GROUND)))
        return x;
      else if (frame == PLFRAME_FALLING)
        return x - 4;
      else
        return x - 8;
    default:
      return x;
  }
}

const uint8_t *Mob::getFrame(int currentY)
{
  switch(objtype)
  {
    case OBJTYPE_PLAYER:
      if (frame < PLFRAME_FALLING)
        return _image_renegade_data + (currentY - y + 15) * 80 *2 + frame * 8 * 2;
      else if (frame == PLFRAME_FALLING)
        return _image_renegade3_data + (currentY - y + 15) * 12 *2;
      else if (frame == PLFRAME_GROUND)
        return _image_renegade4_data + (currentY - y + 15) * 16 *2;
      else
        return _image_renegade2_data + (currentY - y + 15) * 48 *2 + (frame - PLFRAME_PUNCH) * 12 * 2;
      break;
    case OBJTYPE_MUTANT:
      if (frame < PLFRAME_FALLING)
        return _image_mutant_data + (currentY - y + 15) * 80 *2 + frame * 8 * 2;
      else if (frame == PLFRAME_FALLING)
        return _image_mutant3_data + (currentY - y + 15) * 12 *2;
      else if (frame == PLFRAME_GROUND)
        return _image_mutant4_data + (currentY - y + 15) * 16 *2;
      else
        return _image_mutant2_data + (currentY - y + 15) * 24 *2 + (frame - PLFRAME_PUNCH) * 12 * 2;
      break;
    default:
      break;
  }
  return NULL;
}

uint8_t Mob::getFrameSize()
{
  switch(objtype)
  {
    case OBJTYPE_PLAYER:
    case OBJTYPE_MUTANT:
      if (frame <= PLFRAME_HURT)
        return 8;
      else if ((frame >= PLFRAME_PUNCH) || (frame == PLFRAME_FALLING))
        return 12;
      else
        return 16;
      break;
    default:
      break;
  }
  return 0;
}

#define MAX_MOBS 10
#define MOBS_NONE 255

Mob mobs[MAX_MOBS];
uint8_t order[MAX_MOBS];

class World
{
  public:
    World(const Area *cArea) : currentArea(cArea), tick(0), scriptStart(0) { init(); }
    void init();
    void update();
    void draw();
    uint8_t getCollision(int xWhere, int yWhere);
    uint8_t getTile(int x, int y);
    const uint8_t *getTileData(int tile, int y);
    void tryMove(int mobidx, uint8_t joyDir);

    const Area *currentArea;
    int tick;
    int scriptStart;
    bool flags[MAX_FLAGS];
};

World world(&road);

void World::init()
{
  mobs[0].objtype = OBJTYPE_PLAYER;
  mobs[0].x = 0;
  mobs[0].y = 78;
  mobs[0].dir = 0;
  mobs[0].health = PLAYER_HEALTH;
  mobs[1].objtype = OBJTYPE_MUTANT;
  for (int i = 1; i < MAX_MOBS; i++)
    mobs[i].objtype = OBJTYPE_NONE;
  for (int i = 0; i < MAX_FLAGS; i++)
    flags[i] = false;
  scriptStart = 0;
}

void World::update()
{
  if (state == STATE_GAME)
  {
    for (int i = 0; i < MAX_MOBS; i++)
    {
      switch(mobs[i].objtype)
      {
        case OBJTYPE_PLAYER:
          tick = (tick + 1) % 8;
          if (mobs[i].pause)
          {
            mobs[i].pause--;
            if (mobs[i].pause == 0)
            {
              if ((mobs[i].frame == PLFRAME_PUNCH) || (mobs[i].frame == PLFRAME_KICK))
                mobs[i].frame = 2;
              else if (mobs[i].frame == PLFRAME_FALLING)
              {
                mobs[i].frame = PLFRAME_GROUND;
                mobs[i].pause = PAUSE_GROUND;
              }
              else if (mobs[i].frame == PLFRAME_GROUND)
              {
                if (mobs[i].health == 0)
                {
                  mobs[i].pause = PAUSE_GAMEOVER;
                  state = STATE_GAMEOVER;
                }
                else
                {
                  mobs[i].frame = PLFRAME_GETUP;
                  mobs[i].pause = PAUSE_HURT;
                }
              }
              else
                mobs[i].frame = 0;
            }
          }
          else
          {
            if ((mobs[i].knockdown > 0) && ((tick % 2) == 0))
            {
              mobs[i].knockdown--;
              if ((mobs[i].knockdown & 0x0F) == 7)
                mobs[i].knockdown = 0;
            }
            if ((tick == 0) || (tick == 4))
            {
              uint8_t btn = checkButton(TAButton1 | TAButton2);
              uint8_t joyDir = checkJoystick(TAJoystickUp | TAJoystickDown | TAJoystickLeft | TAJoystickRight);
              if (btn > 0)
              {
                if (btn & TAButton1)
                {
                  mobs[i].pause = PAUSE_PUNCH;
                  if (mobs[i].frame >= 2)
                    mobs[i].frame = PLFRAME_PUNCH + 1;
                  else
                    mobs[i].frame = PLFRAME_PUNCH;
                  int x1,x2;
                  if (mobs[i].dir == 1)
                  {
                    x1 = mobs[i].x - 2;
                    x2 = mobs[i].x + 1;
                  }
                  else
                  {
                    x1 = mobs[i].x + 7;
                    x2 = mobs[i].x + 10;
                  }
                  for (int mobidx = 0; mobidx < MAX_MOBS; mobidx++)
                  {
                    if (mobidx == i)
                      continue;
                    if (mobs[mobidx].objtype == OBJTYPE_NONE)
                      continue;
                    if ((mobs[mobidx].y >= mobs[i].y - 1) && (mobs[mobidx].y <= mobs[i].y + 1))
                    {
                      if ((mobs[mobidx].x <= x2) && (mobs[mobidx].x + 8 >= x1))
                      {
                        if ((mobs[mobidx].frame != PLFRAME_HURT) && (mobs[mobidx].frame != PLFRAME_FALLING) && (mobs[mobidx].frame != PLFRAME_GROUND) && (mobs[mobidx].frame != PLFRAME_GETUP))
                        {
                          mobs[mobidx].frame = PLFRAME_HURT;
                          mobs[mobidx].pause = PAUSE_HURT;
                          mobs[mobidx].health--;
                          mobs[mobidx].knockdown = (mobs[mobidx].knockdown + 16) | 0x0F;
                          if (mobs[mobidx].knockdown == 0x3F)
                          {
                            mobs[mobidx].frame = PLFRAME_FALLING;
                            mobs[mobidx].knockdown = 0;
                          }
                        }
                      }
                    }
                  }
                }
                if (btn & TAButton2)
                {
                  mobs[i].pause = PAUSE_KICK;
                  if (mobs[i].frame >= 2)
                    mobs[i].frame = PLFRAME_KICK + 1;
                  else
                    mobs[i].frame = PLFRAME_KICK;
                  int x1,x2;
                  if (mobs[i].dir == 1)
                  {
                    x1 = mobs[i].x - 4;
                    x2 = mobs[i].x + 1;
                  }
                  else
                  {
                    x1 = mobs[i].x + 7;
                    x2 = mobs[i].x + 12;
                  }
                  for (int mobidx = 0; mobidx < MAX_MOBS; mobidx++)
                  {
                    if (mobidx == i)
                      continue;
                    if (mobs[mobidx].objtype == OBJTYPE_NONE)
                      continue;
                    if ((mobs[mobidx].y >= mobs[i].y - 1) && (mobs[mobidx].y <= mobs[i].y + 1))
                    {
                      if ((mobs[mobidx].x <= x2) && (mobs[mobidx].x + 8 >= x1))
                      {
                        if ((mobs[mobidx].frame != PLFRAME_HURT) && (mobs[mobidx].frame != PLFRAME_FALLING) && (mobs[mobidx].frame != PLFRAME_GROUND) && (mobs[mobidx].frame != PLFRAME_GETUP))
                        {
                          mobs[mobidx].frame = PLFRAME_HURT;
                          mobs[mobidx].pause = PAUSE_HURT;
                          mobs[mobidx].health--;
                          mobs[mobidx].knockdown = (mobs[mobidx].knockdown + 16) | 0x0F;
                          if (mobs[mobidx].knockdown == 0x3F)
                          {
                            mobs[mobidx].frame = PLFRAME_FALLING;
                            mobs[mobidx].knockdown = 0;
                          }
                        }
                      }
                    }
                  }
                }
              }
              else if (joyDir > 0)
              {
                tryMove(i, joyDir);
                if (joyDir & TAJoystickLeft)
                  mobs[i].dir = 1;
                if (joyDir & TAJoystickRight)
                  mobs[i].dir = 0;
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
          }
          break;
        case OBJTYPE_MUTANT:
          if (mobs[i].pause)
          {
            mobs[i].pause--;
            if (mobs[i].pause == 0)
            {
              if (mobs[i].frame == PLFRAME_PUNCH)
                mobs[i].frame = 2;
              else if (mobs[i].frame == PLFRAME_FALLING)
              {
                mobs[i].frame = PLFRAME_GROUND;
                mobs[i].pause = PAUSE_GROUND;
              }
              else if (mobs[i].frame == PLFRAME_GROUND)
              {
                if (mobs[i].health == 0)
                  mobs[i].objtype = OBJTYPE_NONE;
                else
                {
                  mobs[i].frame = PLFRAME_GETUP;
                  mobs[i].pause = PAUSE_HURT;
                }
              }
              else
                mobs[i].frame = 0;
            }
          }
          else
          {
            if ((mobs[i].knockdown > 0) && ((tick % 2) == 0))
            {
              mobs[i].knockdown--;
              if ((mobs[i].knockdown & 0x0F) == 7)
                mobs[i].knockdown = 0;
            }
            if (tick % 8 == 0)
            {
              switch(mobs[i].state)
              {
                case MUTANTSTATE_ATTACK:
                {
                  int x1,x2;
                  if (mobs[i].dir == 1)
                  {
                    x1 = mobs[i].x - 2;
                    x2 = mobs[i].x + 1;
                  }
                  else
                  {
                    x1 = mobs[i].x + 7;
                    x2 = mobs[i].x + 10;
                  }
                  if ((mobs[0].y >= mobs[i].y - 1) && (mobs[0].y <= mobs[i].y + 1))
                  {
                    if ((mobs[0].x <= x2) && (mobs[0].x + 8 >= x1))
                    {
                      if ((mobs[0].frame != PLFRAME_HURT) && (mobs[0].frame != PLFRAME_FALLING) && (mobs[0].frame != PLFRAME_GROUND) && (mobs[0].frame != PLFRAME_GETUP))
                      {
                        mobs[i].state = MUTANTSTATE_ATTACK;
                        mobs[i].data = MUTANTSTATEDATA_ATTACK;
                        mobs[i].pause = PAUSE_PUNCH;
                        if (mobs[i].frame >= 2)
                          mobs[i].frame = PLFRAME_PUNCH + 1;
                        else
                          mobs[i].frame = PLFRAME_PUNCH;
                        mobs[0].frame = PLFRAME_HURT;
                        mobs[0].pause = PAUSE_HURT;
                        mobs[0].health--;
                        mobs[0].knockdown = (mobs[0].knockdown + 16) | 0x0F;
                        if ((mobs[0].knockdown == 0x3F) || (mobs[0].health == 0))
                        {
                          mobs[0].frame = PLFRAME_FALLING;
                          mobs[0].knockdown = 0;
                          mobs[i].state = MUTANTSTATE_BACKOFF;
                          mobs[i].data = MUTANTSTATEDATA_BACKOFF;
                        }
                      }
                    }
                    else
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
                }
                case MUTANTSTATE_APPROACH:
                {
                  int x1,x2;
                  {
                    x1 = mobs[i].x;
                    x2 = mobs[i].x + 7;
                  }
                  bool dirChange = true;
                  uint8_t joyDir = 0;
                  if ((mobs[0].x <= x2) && (mobs[0].x + 8 >= x1))
                  {
                    dirChange = false;
                    if (mobs[i].dir == 1)
                      joyDir |= TAJoystickRight;
                    else
                      joyDir |= TAJoystickLeft;
                  }
                  else if (mobs[0].x > mobs[i].x + 9)
                      joyDir |= TAJoystickRight;
                  else if (mobs[0].x < mobs[i].x - 1)
                      joyDir |= TAJoystickLeft;
                  if (mobs[0].y > mobs[i].y)
                      joyDir |= TAJoystickDown;
                  else if (mobs[0].y < mobs[i].y)
                      joyDir |= TAJoystickUp;
                  tryMove(i, joyDir);
                  if (dirChange)
                  {
                    if (joyDir & TAJoystickLeft)
                      mobs[i].dir = 1;
                    if (joyDir & TAJoystickRight)
                      mobs[i].dir = 0;
                  }
                  break;
                }
                case MUTANTSTATE_BACKOFF:
                {
                  uint8_t joyDir = 0;
                  if ((mobs[0].x <= mobs[i].x) && (mobs[i].x - mobs[0].x < 100))
                  {
                    joyDir |= TAJoystickRight;
                  }
                  else if ((mobs[0].x > mobs[i].x) && (mobs[0].x - mobs[i].x < 100))
                  {
                    joyDir |= TAJoystickLeft;
                  }
                  if (mobs[0].y > mobs[i].y)
                      joyDir |= TAJoystickDown;
                  else if (mobs[0].y < mobs[i].y)
                      joyDir |= TAJoystickUp;
                  tryMove(i, joyDir);
                  if (mobs[0].x <= mobs[i].x)
                    mobs[i].dir = 1;
                  else if (mobs[0].x > mobs[i].x)
                    mobs[i].dir = 0;
                  break;
                }
                default:
                  break;
              }
              mobs[i].data--;
              if (mobs[i].data == 0)
              {
                switch(mobs[i].state)
                {
                  case MUTANTSTATE_ATTACK:
                    mobs[i].state = MUTANTSTATE_APPROACH;
                    mobs[i].data = MUTANTSTATEDATA_APPROACH;
                    break;
                  case MUTANTSTATE_APPROACH:
                    mobs[i].state = MUTANTSTATE_ATTACK;
                    mobs[i].data = MUTANTSTATEDATA_ATTACK;
                    break;
                  case MUTANTSTATE_BACKOFF:
                  default:
                    mobs[i].state = MUTANTSTATE_APPROACH;
                    mobs[i].data = MUTANTSTATEDATA_APPROACH;
                    break;
                }
              }
            }
          }
          break;
        default:
          break;
      }
    }
  }
  else if (state == STATE_GAMEOVER)
  {
    if (mobs[0].pause > 0)
    {
      mobs[0].pause--;
      uint8_t btn = checkButton(TAButton1 | TAButton2);
      uint8_t joyDir = checkJoystick(TAJoystickUp | TAJoystickDown | TAJoystickLeft | TAJoystickRight);
    }
    else
    {
      uint8_t btn = checkButton(TAButton1 | TAButton2);
      uint8_t joyDir = checkJoystick(TAJoystickUp | TAJoystickDown | TAJoystickLeft | TAJoystickRight);
      if (btn & TAButton1)
      {
        init();
        state = STATE_GAME;
      }
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
  for (int i = scriptStart; (currentArea->script[i].x != 0) && (currentArea->script[i].x <= startX + 96); i++)
  {
    if (currentArea->script[i].run() && (i == scriptStart))
      scriptStart++;
  }
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
    if ((state == STATE_GAMEOVER) && (lines > 28) && (lines < 41))
    {
      const uint8_t *data = _image_game_over_data + (lines - 29) * 89 *2;
      for (int i = 0; i < 89; i++)
      {
        if ((data[i * 2] != 0xf8) || (data[i * 2 + 1] != 0x1f))
        {
          lineBuffer[(2 + i) * 2] = data[i * 2];
          lineBuffer[(2 + i) * 2 + 1] = data[i * 2 + 1];
        }
      }
    }
    else if ((state == STATE_GAME) && (lines > 2) && (lines < 11))
    {
      int where = 4;
      uint8_t health = mobs[0].health;
      while (health >= 4)
      {
        const uint8_t *data = _image_heart_data + (lines - 3) * 32 * 2 + 24 * 2;
        for (int i = 0; i < 8; i++)
        {
          if ((data[i * 2] != 0xf8) || (data[i * 2 + 1] != 0x1f))
          {
            lineBuffer[(where + i) * 2] = data[i * 2];
            lineBuffer[(where + i) * 2 + 1] = data[i * 2 + 1];
          }
        }
        where += 8;
        health -= 4;
      }
      if (health > 0)
      {
        const uint8_t *data = _image_heart_data + (lines - 3) * 32 * 2 + (health -1) * 8 * 2;
        for (int i = 0; i < 8; i++)
        {
          if ((data[i * 2] != 0xf8) || (data[i * 2 + 1] != 0x1f))
          {
            lineBuffer[(where + i) * 2] = data[i * 2];
            lineBuffer[(where + i) * 2 + 1] = data[i * 2 + 1];
          }
        }
      }
    }
    display.writeBuffer(lineBuffer,96 * 2);
  }
  display.endTransfer();
}

uint8_t World::getCollision(int xWhere, int yWhere)
{
  return tilesetCollision[currentArea->data[xWhere + (yWhere * currentArea->xSize)] - 1];
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

void World::tryMove(int i, uint8_t joyDir)
{
  if (mobs[i].frame >= 4)
    mobs[i].frame = 0;
  if ((joyDir & TAJoystickUp) && (mobs[i].y - 16 > 0))
  {
    if ((mobs[i].y % 8) == 0)
    {
      int yWhere = mobs[i].y / 8;
      int xWhere = mobs[i].x / 8;
      int xMod = mobs[i].x % 8;
      bool col1 = getCollision(xWhere, yWhere - 1);
      if (xMod == 0)
      {
        if (col1 == 0)
          mobs[i].y--;
      }
      else
      {
        bool col2 = getCollision(xWhere + 1, yWhere - 1);
        if ((col1 == 0) && (col2 == 0))
          mobs[i].y--;
        else if ((col1 == 0) && (col2 != 0) && (xMod == 1))
        {
          mobs[i].y--;
          mobs[i].x--;
        }
        else if ((col1 != 0) && (col2 == 0) && (xMod == 7))
        {
          mobs[i].y--;
          mobs[i].x++;
        }
      }
    }
    else
      mobs[i].y--;
  }
  else if ((joyDir & TAJoystickDown) && (mobs[i].y + 1 < world.currentArea->ySize * 8))
  {
    if ((mobs[i].y % 8) == 7)
    {
      int yWhere = mobs[i].y / 8;
      int xWhere = mobs[i].x / 8;
      int xMod = mobs[i].x % 8;
      bool col1 = getCollision(xWhere, yWhere + 1);
      if (xMod == 0)
      {
        if (col1 == 0)
          mobs[i].y++;
      }
      else
      {
        bool col2 = getCollision(xWhere + 1, yWhere + 1);
        if ((col1 == 0) && (col2 == 0))
          mobs[i].y++;
        else if ((col1 == 0) && (col2 != 0) && (xMod == 1))
        {
          mobs[i].y++;
          mobs[i].x--;
        }
        else if ((col1 != 0) && (col2 == 0) && (xMod == 7))
        {
          mobs[i].y++;
          mobs[i].x++;
        }
      }
    }
    else
      mobs[i].y++;
  }
  if (joyDir & TAJoystickLeft)
  {
    if (mobs[i].x > 0)
    {
      if ((mobs[i].x % 8) == 0)
      {
        int yWhere = mobs[i].y / 8;
        int xWhere = mobs[i].x / 8;
        bool col1 = getCollision(xWhere - 1, yWhere);
        if (col1 == 0)
          mobs[i].x--;
      }
      else
        mobs[i].x--;
    }
  }
  else if (joyDir & TAJoystickRight)
  {
    if (mobs[i].x + 9 < world.currentArea->xSize * 8)
    {
      if ((mobs[i].x % 8) == 0)
      {
        int yWhere = mobs[i].y / 8;
        int xWhere = mobs[i].x / 8;
        int yMod = mobs[i].y % 8;
        bool col1 = getCollision(xWhere + 1, yWhere);
        if (col1 == 0)
          mobs[i].x++;
      }
      else
        mobs[i].x++;
    }
  }
  if (tick == 0)
    mobs[i].frame = (mobs[i].frame + 1) % 4;
}

bool AreaScript::run() const
{
  if (world.flags[flag] == true)
    return true;
  switch(cmd)
  {
    case CMD_SPAWNMUTANT:
      for (int i = 1; i < MAX_MOBS; i++)
      {
        if (mobs[i].objtype == OBJTYPE_NONE)
        {
          mobs[i].objtype = OBJTYPE_MUTANT;
          mobs[i].x = arg2;
          mobs[i].y = arg3;
          mobs[i].dir = ((mobs[i].x > mobs[0].x) ? 1 : 0);
          mobs[i].health = 3;
          mobs[i].state = MUTANTSTATE_APPROACH;
          mobs[i].data = MUTANTSTATEDATA_APPROACH;
          world.flags[flag] = true;
          return true;
        }
      }
      break;
    default:
      break;
  }
  return false;
}

void setup()
{
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

void loop()
{
  world.update();
  world.draw();
  unsigned long oldTime = lastTime;
  lastTime = millis();
  if ((lastTime > oldTime) && (lastTime - oldTime < 33))
  {
    delay(33 - (lastTime - oldTime));
  }
}
