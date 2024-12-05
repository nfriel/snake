#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>

#define WIDTH 30
#define HEIGHT 20
#define NAPTIME 150
#define COLOR_PURPLE 8
#define MAX_SEGMENTS (WIDTH * HEIGHT)
#define INITIAL_SNAKE_LENGTH 1
#define BCHAR '.'
#define SCHAR '#'
#define SCHARUP '^'
#define SCHARDOWN 'v'
#define SCHARLEFT '<'
#define SCHARRIGHT '>'
#define ACHAR 'O'
#define BORDER '#'

// Structs for storing positions of snake head and tail segments
enum snakeDirection {UP, DOWN, LEFT, RIGHT};

typedef struct snakeHead
{
    int xPos;
    int yPos;
    enum snakeDirection localDirection;
} snakeHead;

// Global variables

// Snake head
snakeHead snake;

// Tail array
snakeHead tail[MAX_SEGMENTS];

// Apple
snakeHead apple;

// 2d array representing board
char board[WIDTH][HEIGHT];

// Snake length
int snakeLength = INITIAL_SNAKE_LENGTH;

// Initial snake direction
enum snakeDirection direction = RIGHT;

// Function prototypes
void initBoard(void);
void drawBoard(void);
void drawStart(void);
void drawGameOver(void);
void initSnake(void);
void drawSnakeHeadOnBoard(void);
void drawSnakeHeadOnBoardDirectional(void);
void moveSnakeHead(void);
void storeTailPos(void);
void eraseTail(void);
void drawTailOnBoard(void);
void generateApple(void);
void appleCheck(void);
bool collisionCheck(void);
void getInput(void);

int main(void)
{
    // Start curses mode
    initscr();
    // Hide keystrokes
    noecho();
    // Color mode
    start_color();
    // Define color
    init_color(COLOR_PURPLE, 500, 0, 500);
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_PURPLE, COLOR_BLACK);

    // Start screen
    drawStart();
    getch();
    clear();

    // Allow for key input
    keypad(stdscr, TRUE);

    // Initialize board
    initBoard();

    // Add an apple
    generateApple();

    // Game still going on?
    bool game = true;

    // Make getch() non blocking, will return ERR if no input
    nodelay(stdscr, TRUE);

    // Hide cursor
    curs_set(0);

    // Main game loop
    while (game)
    {
        // Check for user input
        getInput();

        // Clear board
        clear();

        // Move snake
        moveSnakeHead();

        // Check for collision with apple
        appleCheck();

        // Update tail position
        storeTailPos();

        // Draw snake head on board
        drawSnakeHeadOnBoardDirectional();

        // Draw tail on board
        drawTailOnBoard();

        // Add apple
        board[apple.xPos][apple.yPos] = ACHAR;

        // Draw board + snake + tail
        drawBoard();

        // Wipe tail
        eraseTail();

        // Check for collisions
        game = collisionCheck();

        // Refresh
        refresh();

        // Nap
        napms(NAPTIME - (snakeLength * 2));
    }

    drawGameOver();
    refresh();
    napms(4000);
    getch();

    // Stop curses mode
    endwin();
    // Show cursor
    curs_set(1);

    return 0;
}

void initBoard(void)
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            board[j][i] = BCHAR;
        }
    }
}

void drawBoard(void)
{
    // Top border
    for (int i = 0; i < WIDTH + 2; i++)
    {
        attron(COLOR_PAIR(3));
        addch(BORDER);
        attroff(COLOR_PAIR(3));
    }
    addch('\n');

    // Draw to screen
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH + 1; j++)
        {
            // If first of line
            if (j == 0)
            {
                attron(COLOR_PAIR(3));
                addch(BORDER);
                attroff(COLOR_PAIR(3));
            }

            // If last of line
            if (j == WIDTH)
            {
                attron(COLOR_PAIR(3));
                addch(BORDER);
                attroff(COLOR_PAIR(3));
                break;
            }

            // Check if part of tail
            bool isTail = false;

            for (int k = 0; k < snakeLength; k++)
            {
                if (tail[k].xPos == j && tail[k].yPos == i)
                {
                    isTail = true;
                }
            }

            // Is it an apple?
            if (apple.xPos == j && apple.yPos == i)
            {
                attron(COLOR_PAIR(2));
                addch(board[j][i]);
                attroff(COLOR_PAIR(2));
            }
            else if (!isTail)
            {
                addch(board[j][i]);
            }
            else
            {
                attron(COLOR_PAIR(1));
                addch(board[j][i]);
                attroff(COLOR_PAIR(1));
            }
        }
        addch('\n');
    }

    // Bottom border
    for (int i = 0; i < WIDTH + 2; i++)
    {
        attron(COLOR_PAIR(3));
        addch(BORDER);
        attroff(COLOR_PAIR(3));
    }
}

void drawStart(void)
{
    char *title = "SNAKE";
    char *subtitle = "PRESS ANY KEY TO START";

    int titleLen = strlen(title);
    int subtitleLen = strlen(subtitle);

    snakeLength = titleLen + subtitleLen;

    initBoard();

    int start = WIDTH / 2 - (titleLen / 2);
    int start2 = WIDTH / 2 - (subtitleLen / 2);


    for (int i = 0; i < titleLen; i++)
    {
        board[start + i][HEIGHT / 4] = title[i];
        tail[i].xPos = start + i;
        tail[i].yPos = HEIGHT / 4;
    }

    for (int i = 0; i < subtitleLen; i++)
    {
        board[start2 + i][HEIGHT / 4 + 2] = subtitle[i];
        tail[i + titleLen].xPos = start2 + i;
        tail[i + titleLen].yPos = HEIGHT / 4 + 2;
    }

    // Write "SNAKE"
    drawBoard();

    snakeLength = INITIAL_SNAKE_LENGTH;
}

void drawGameOver(void)
{
    clear();
    
    char *title = "GAME";
    char *subtitle = "OVER";

    int titleLen = strlen(title);
    int subtitleLen = strlen(subtitle);

    snakeLength = titleLen + subtitleLen;

    initBoard();

    int start = WIDTH / 2 - (titleLen / 2);
    int start2 = WIDTH / 2 - (subtitleLen / 2);


    for (int i = 0; i < titleLen; i++)
    {
        board[start + i][HEIGHT / 4] = title[i];
        tail[i].xPos = start + i;
        tail[i].yPos = HEIGHT / 4;
    }

    for (int i = 0; i < subtitleLen; i++)
    {
        board[start2 + i][HEIGHT / 4 + 2] = subtitle[i];
        tail[i + titleLen].xPos = start2 + i;
        tail[i + titleLen].yPos = HEIGHT / 4 + 2;
    }

    // Write "SNAKE"
    drawBoard();
}

void initSnake(void)
{
    snake.xPos = WIDTH / 2;
    snake.yPos = HEIGHT / 2;
}

void drawSnakeHeadOnBoard(void)
{
    // Draw head onto board array
    board[snake.xPos][snake.yPos] = SCHAR;
}

void drawSnakeHeadOnBoardDirectional(void)
{
    switch (direction)
    {
        case UP:
            board[snake.xPos][snake.yPos] = SCHARUP;
            break;
        case DOWN:
            board[snake.xPos][snake.yPos] = SCHARDOWN;
            break;
        case LEFT:
            board[snake.xPos][snake.yPos] = SCHARLEFT;
            break;
        case RIGHT:
            board[snake.xPos][snake.yPos] = SCHARRIGHT;
            break;
    }

    // Draw head onto board array
    board[snake.xPos][snake.yPos] = SCHAR;
}


void moveSnakeHead(void)
{
    // Update x and y coordinates of snake based on direction
    snake.localDirection = direction;

    switch (direction)
    {
        case UP:
            if (snake.yPos == 0)
                snake.yPos = HEIGHT - 1;
            else
                snake.yPos--;
            break;

        case DOWN:
            if (snake.yPos == HEIGHT - 1)
                snake.yPos = 0;
            else
                snake.yPos++;
            break;

        case LEFT:
            if (snake.xPos == 0)
                snake.xPos = WIDTH - 1;
            else
                snake.xPos--;
            break;

        case RIGHT:
            if (snake.xPos == WIDTH - 1)
                snake.xPos = 0;
            else
                snake.xPos++;
            break;
    }
}

void storeTailPos(void)
{
    // Take position of snake head and store in tail array

    // Shift array +1
    for (int i = snakeLength; i >= 0; i--)
    {
        tail[i] = tail[i-1];
    }

    // Store current position of head at position 0
    tail[0] = snake;
}

void eraseTail(void)
{
    for (int i = 0; i < snakeLength; i++)
    {
        board[tail[i].xPos][tail[i].yPos] = BCHAR;
    }
}

void drawTailOnBoard(void)
{
    for (int i = 0; i < snakeLength; i++)
    {
        board[tail[i].xPos][tail[i].yPos] = SCHAR;

        switch (tail[i].localDirection)
    {
        case UP:
            board[tail[i].xPos][tail[i].yPos] = SCHARUP;
            break;
        case DOWN:
            board[tail[i].xPos][tail[i].yPos] = SCHARDOWN;
            break;
        case LEFT:
            board[tail[i].xPos][tail[i].yPos] = SCHARLEFT;
            break;
        case RIGHT:
            board[tail[i].xPos][tail[i].yPos] = SCHARRIGHT;
            break;
    }
    }
}

void generateApple(void)
{
    // Random x and y values based on width and height
    int x = rand() % (WIDTH);
    int y = rand() % (HEIGHT);

    apple.xPos = x;
    apple.yPos = y;
}

void appleCheck(void)
{
    // If snake head touches the apple
    if (snake.xPos == apple.xPos && snake.yPos == apple.yPos)
    {
        snakeLength++;
        generateApple();
    }
}

bool collisionCheck(void)
{
    for (int i = 1; i < snakeLength; i++)
    {
        if (snake.xPos == tail[i].xPos && snake.yPos == tail[i].yPos)
        {
            return false;
        }
    }
    return true;
}

void getInput(void)
{
    int ch;

    ch = getch();

    if (ch == ERR)
        return;

    switch (ch)
    {
        case KEY_UP:
            if (direction != DOWN)
                direction = UP;
            break;

        case KEY_DOWN:
            if (direction != UP)
                direction = DOWN;
            break;

        case KEY_LEFT:
            if (direction != RIGHT)
                direction = LEFT;
            break;

        case KEY_RIGHT:
            if (direction != LEFT)
                direction = RIGHT;
            break;

        case 'p':
            do
            {
                ch = getch();
            }
            while (ch != 'p');
}
}