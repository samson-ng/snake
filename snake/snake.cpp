// pong.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <windows.h>
#include <iostream>
#include <conio.h>
#include <sstream> 
#include <math.h> 
#include <gl\gl.h>
#include <gl\glu.h>
#include "GL/freeglut.h"
#pragma comment(lib, "Opengl32.lib")

// keycodes
#define VK_W 0x57
#define VK_S 0x53

const int width = 500;
const int height = 500;
int interval = 200;

enum {
  numberOfCols = 19,
  numberOfRows = 19,
  tailLength = 70,
  birthPlace_x = (numberOfCols + tailLength)/2,
  birthPlace_y = numberOfRows/2,
  numberOfSquares = numberOfRows*numberOfCols
};

enum direction {
  LEFT,
  RIGHT,
  UP,
  DOWN,
  NONE
};

struct vec {

  int x, y;
  vec() {}

};

class square {

  static const int size = 5;
  static const int gap = 10;
  static const int origin_x = (width - numberOfCols*size - (numberOfCols-1)*gap) / 2;
  static const int origin_y = (height - numberOfRows*size - (numberOfRows-1)*gap) / 2;

public:

  vec coord;
  vec vertex[4];
  direction orient;

  square() {}

  void set_coord(int col, int row) {
    coord.x = col;
    coord.y = row;
  }

  void draw() {
    vertex[0].x = origin_x + (size + gap)*coord.x;
    vertex[0].y = origin_y + (size + gap)*coord.y;
    vertex[1].x = origin_x + (size + gap)*coord.x + size;
    vertex[1].y = origin_y + (size + gap)*coord.y;
    vertex[2].x = origin_x + (size + gap)*coord.x + size;
    vertex[2].y = origin_y + (size + gap)*coord.y + size;
    vertex[3].x = origin_x + (size + gap)*coord.x;
    vertex[3].y = origin_y + (size + gap)*coord.y + size;

    switch (orient) {
    case LEFT:
      vertex[0].x -= gap;
      vertex[3].x -= gap;
      break;
    case RIGHT:
      vertex[1].x += gap;
      vertex[2].x += gap;
      break;
    case UP:
      vertex[2].y += gap;
      vertex[3].y += gap;
      break;
    case DOWN:
      vertex[0].y -= gap;
      vertex[1].y -= gap;
      break;
    }

    glBegin(GL_QUADS);
    glVertex2f( (float)vertex[0].x, (float)vertex[0].y);
    glVertex2f( (float)vertex[1].x, (float)vertex[1].y);
    glVertex2f( (float)vertex[2].x, (float)vertex[2].y);
    glVertex2f( (float)vertex[3].x, (float)vertex[3].y);
    glEnd();

  }
};

struct linkedList {

  struct item {
    item* next;
    item* prev;
    square piece;
  };

  item head;
  item* tail;
  direction status;
  item* iterator;

  linkedList() {
    head.next = &head;
    head.prev = &head;
    tail = &head;
  }

  void move(int head_movement_x, int head_movement_y){

    vec prevCoord = head.piece.coord;
    head.piece.coord.x += head_movement_x;
    head.piece.coord.y += head_movement_y;

    iterator = head.next;
    while (iterator != &head) {
      vec currentCoord = iterator->piece.coord;
      iterator->piece.coord = prevCoord;
      prevCoord = currentCoord;

      if (iterator->prev->piece.coord.y > iterator->piece.coord.y) iterator->piece.orient = UP;
      if (iterator->prev->piece.coord.y < iterator->piece.coord.y) iterator->piece.orient = DOWN;
      if (iterator->prev->piece.coord.x > iterator->piece.coord.x) iterator->piece.orient = RIGHT;
      if (iterator->prev->piece.coord.x < iterator->piece.coord.x) iterator->piece.orient = LEFT;
      printf("%i", iterator->piece.orient);
      iterator = iterator->next;
    } printf("\n");

  }

  void collision_detect (int offset_x, int offset_y){
    iterator = head.next;
    while (iterator != &head) {
      if (head.piece.coord.x == iterator->piece.coord.x + offset_x &&
        head.piece.coord.y == iterator->piece.coord.y + offset_y) {
          status = NONE; break;
      }
      iterator = iterator->next;
    }
  }

  item* grow(item *prev) {

    item *current = new item;
    current->next = prev->next;
    current->prev = prev;
    current->next->prev = current;
    prev->next = current;
    return current;
  }

};

square background[numberOfSquares];
linkedList snake;

void init() {

  snake.status = RIGHT;

  for (int j = 0; j < numberOfRows; ++j) {
    for (int i = 0; i < numberOfCols; ++i) {
      background[j*numberOfCols + i].set_coord(i,j);
      background[j*numberOfCols + i].orient = NONE;
    }
  }

  snake.head.piece.set_coord(birthPlace_x, birthPlace_y);
  snake.head.piece.orient = NONE;

  for (int i = 0; i < tailLength; i ++) {
    snake.tail = snake.grow(snake.tail);
    snake.tail->piece.set_coord
      (--snake.tail->prev->piece.coord.x,snake.tail->prev->piece.coord.y);
  }

}

void draw() {
  // clear (has to be done at the beginning)
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  glColor3f(0.1f, 0.1f, 0.1f);
  for (int i = 0; i < numberOfSquares; ++i) {
    background[i].draw();
  }

  glColor3f(1.0f, 0.0f, 0.0f);
  snake.iterator = snake.head.next;
  while (snake.iterator != &snake.head) {
    snake.iterator->piece.draw();
    snake.iterator = snake.iterator->next;
  }

  snake.head.piece.draw();

  // swap buffers (has to be done at the end)
  glutSwapBuffers();
}

void keyboard() {

  if (GetAsyncKeyState(VK_SPACE)) {
    snake.tail = snake.grow(snake.tail);
    interval = (int)(interval*0.95);
  }

  if (snake.status == UP || snake.status == DOWN) {
    if (GetAsyncKeyState(VK_LEFT)) snake.status = LEFT;
    if (GetAsyncKeyState(VK_RIGHT)) snake.status = RIGHT;
    if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN)) snake.status = snake.status;
  }

  if (snake.status == LEFT || snake.status == RIGHT) {
    if (GetAsyncKeyState(VK_UP)) snake.status = UP;
    if (GetAsyncKeyState(VK_DOWN)) snake.status = DOWN;
    if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState(VK_RIGHT)) snake.status = snake.status;
  }
}

void update(int value) {

  keyboard();
  switch (snake.status) {
  case LEFT:
    snake.collision_detect(1,0);
    if (snake.head.piece.coord.x == 0) snake.status = NONE;
    if (snake.status != NONE) snake.move(-1,0);
    break;
  case RIGHT:
    snake.collision_detect(-1,0);
    if (snake.head.piece.coord.x == numberOfCols - 1) snake.status = NONE;
    if (snake.status != NONE) snake.move(1,0);
    break;
  case UP:
    snake.collision_detect(0,-1);
    if (snake.head.piece.coord.y == numberOfRows - 1) snake.status = NONE;
    if (snake.status != NONE) snake.move(0,1);
    break;
  case DOWN:
    snake.collision_detect(0,1);
    if (snake.head.piece.coord.y == 0) snake.status = NONE;
    if (snake.status != NONE) snake.move(0,-1);
    break;
  }

  // Call update() again in 'interval' milliseconds
  glutTimerFunc(interval, update, 0);

  // Redisplay frame
  glutPostRedisplay();
}

void enable2D(int width, int height) {
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, width, 0.0f, height, 0.0f, 1.0f);
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();
}

int _tmain(int argc, char** argv) {
  // initialize opengl (via glut)
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(width, height);
  glutCreateWindow("Snake");

  init();

  // Register callback functions
  glutDisplayFunc(draw);
  glutTimerFunc(interval, update, 0);

  // setup scene to 2d mode and set draw color to white
  enable2D(width, height);

  // start the whole thing
  glutMainLoop();
  return 0;
}