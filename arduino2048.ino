#include <time.h>
#include <U8g2lib.h>

//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);  //OLED

U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R2, /* CS=*/10, /* reset=*/12); //LCD12864
#include "Number.h"

/*Pins of Buttons*/
#define Up 4
#define Down 5
#define Left 6
#define Right 7

//#define CHEAT  //开局生成两个1024以作弊

inline int getRand(int a, int b)
{
  return rand() % (b - a + 1) + a;
}

enum DIRECTION
{
  LEFT,
  RIGHT,
  UP,
  DOWN
};
class board
{
  private:
    int map[4][4];
    long score;
    using PtrToMemberFunc = bool (board::*)(int);                                                   //指向成员函数的指针
    PtrToMemberFunc Moves[4] = {&board::LeftOne, &board::RightOne, &board::UpOne, &board::DownOne}; //把四个方向移动的方法组织成一个数组
    bool changed;

  public:
    void init();
    void summon(bool noFour = false);
    bool isOver();
    bool isWon();
    bool isChanged()
    {
      return changed;
    };
    bool LeftOne(int ro);
    bool RightOne(int ro);
    bool UpOne(int ro);
    bool DownOne(int ro);
    void move(DIRECTION di);
    void updateDisplay();
    void setNum(int a, int b);
};

void board::init()
{

  memset(map, 0, sizeof(map));
  summon();
  summon(true);
#ifdef CHEAT
  map[0][0] = 1024;
  map[1][0] = 1024;
#endif
  changed = true;
  score = 0;
}
void board::summon(bool noFour)
{
  struct block
  {
    int x;
    int y;
  } blanks[16];
  memset(blanks, 0, sizeof(blanks));

  int ptr = 0;
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      if (map[i][j] == 0) //遍历棋盘，寻找空格，放入队列
        blanks[ptr++] = {i, j};
  block &sele = blanks[getRand(0, ptr - 1)]; //从队列里随机选择一个空格
  if (noFour)
    map[sele.x][sele.y] = 2;
  else
    map[sele.x][sele.y] = getRand(1, 10) > 3 ? 2 : 4;
}

bool board::LeftOne(int ro)
{
  bool fail = true;
  int c = 0;
  while (c < 4)
  {
    int nextc = c + 1;
    while (nextc < 4 && map[ro][nextc] == 0)
      nextc++;
    if (nextc >= 4)
      break;
    if (map[ro][c] == 0)
    {
      fail = false;
      map[ro][c] = map[ro][nextc];
      map[ro][nextc] = 0;
      continue;
    }
    else if (map[ro][c] == map[ro][nextc])
    {
      fail = false;
      map[ro][c] *= 2;
      score += map[ro][c];
      map[ro][nextc] = 0;
    }
    ++c;
  }
  return fail;
}
bool board::RightOne(int ro)
{
  bool fail = true;
  int c = 3;
  while (c >= 0)
  {
    int nextc = c - 1;
    while (nextc >= 0 && map[ro][nextc] == 0)
      nextc--;
    if (nextc < 0)
      break;
    if (map[ro][c] == 0)
    {
      fail = false;
      map[ro][c] = map[ro][nextc];
      map[ro][nextc] = 0;
      continue;
    }
    else if (map[ro][c] == map[ro][nextc])
    {
      fail = false;
      map[ro][c] *= 2;
      score += map[ro][c];
      map[ro][nextc] = 0;
    }
    --c;
  }
  return fail;
}
bool board::UpOne(int ro)
{
  bool fail = true;

  int c = 0;
  while (c < 4)
  {
    int nextc = c + 1;
    while (nextc < 4 && map[nextc][ro] == 0)
      nextc++;
    if (nextc >= 4)
      break;
    if (map[c][ro] == 0)
    {
      fail = false;
      map[c][ro] = map[nextc][ro];
      map[nextc][ro] = 0;
      continue;
    }
    else if (map[c][ro] == map[nextc][ro])
    {
      fail = false;
      map[c][ro] *= 2;
      score += map[c][ro];
      map[nextc][ro] = 0;
    }
    ++c;
  }
  return fail;
}
bool board::DownOne(int ro)
{
  bool fail = true;

  int c = 3;
  while (c >= 0)
  {
    int nextc = c - 1;
    while (nextc >= 0 && map[nextc][ro] == 0)
      nextc--;
    if (nextc < 0)
      break;
    if (map[c][ro] == 0)
    {
      fail = false;
      map[c][ro] = map[nextc][ro];
      map[nextc][ro] = 0;
      continue;
    }
    else if (map[c][ro] == map[nextc][ro])
    {
      fail = false;
      map[c][ro] *= 2;
      score += map[c][ro];
      map[nextc][ro] = 0;
    }
    --c;
  }
  return fail;
}
void board::move(DIRECTION di)
{
  bool fail = true;
  for (int i = 0; i < 4; ++i)
  {
    bool tmp = (this->*Moves[di])(i);
    fail &= tmp;
  }
  changed = !fail;
  if (!fail)
    summon();
}
bool board::isOver()
{
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
    {
      if (map[i][j] == 0)
        return false;
      if (j < 3 && map[i][j] == map[i][j + 1])
      {
        return false;
      }
      if (i < 3 && map[i][j] == map[i + 1][j])
      {
        return false;
      }
    }
  return true;
}
bool board::isWon()
{
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
    {
      if (map[i][j] >= 2048)
        return true;
    }
  return false;
}

void board::updateDisplay()
{
  u8g2.clearBuffer();

  u8g2.drawFrame(0, 0, 61, 61);
  u8g2.drawHLine(0, 15, 61);
  u8g2.drawHLine(0, 30, 61);
  u8g2.drawHLine(0, 45, 61);
  u8g2.drawVLine(15, 0, 61);
  u8g2.drawVLine(30, 0, 61);
  u8g2.drawVLine(45, 0, 61);
  u8g2.setFont(u8g2_font_crox4t_tr);
  u8g2.drawStr(65, 28, "Score:");
  char score_str[6];
  itoa(score, score_str, 10);
  u8g2.drawStr(65, 48, score_str);

  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      if (map[i][j] != 0)
        setNum(i, j);
  u8g2.sendBuffer();
}
void board::setNum(int a, int b)
{
  int num = map[a][b];
  u8g2.drawXBMP(1 + a * 15, 1 + b * 15, 14, 14, GET_NUM_DATA(num));
}

char getKey()
{
  while (!(digitalRead(Up) || digitalRead(Down) || digitalRead(Left) || digitalRead(Right)))
    ;
  bool U = digitalRead(Up);
  bool D = digitalRead(Down);
  bool L = digitalRead(Left);
  bool R = digitalRead(Right);
  while (digitalRead(Up) || digitalRead(Down) || digitalRead(Left) || digitalRead(Right))
    ;
  if (U)
    return 'a';
  if (D)
    return 'd';
  if (L)
    return 'w';
  if (R)
    return 's';
  return 'x';
}

board Game;
void setup()
{
  srand((unsigned)time(NULL) + analogRead(A0));
  u8g2.begin();

}

void loop()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_maniac_tr);
  u8g2.drawStr(27, 24, "2048");
  u8g2.setFont(u8g2_font_7x14B_mr);
  u8g2.drawStr(48, 48, "Start");
  u8g2.drawFrame(45, 36, 40, 15);
  u8g2.sendBuffer();
  getKey();
  Game.init();
  while (!(Game.isOver() || Game.isWon()))
  {
    if (Game.isChanged())
      Game.updateDisplay();
    char cmd = getKey();
    switch (cmd)
    {
      case 'w':
        Game.move(UP);
        break;
      case 'a':
        Game.move(LEFT);
        break;
      case 's':
        Game.move(DOWN);
        break;
      case 'd':
        Game.move(RIGHT);
        break;
      default:
        break;
    }
  }
  Game.updateDisplay();
  delay(2000);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_maniac_tr);
  if (Game.isWon())
  {
    u8g2.drawStr(23, 24, "You");
    u8g2.drawStr(23, 53, "Win!");
  }
  else
  {
    u8g2.drawStr(23, 24, "Game");
    u8g2.drawStr(23, 53, "Over");
  }
  u8g2.sendBuffer();
  delay(2000);
}
