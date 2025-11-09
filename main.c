#include <stdio.h>
#include <stdlib.h>
#include "graphics.h"
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define EMPTY 0
#define OBS 1
#define MARK 2

//represents robot: x,y mark position in grid; dir stores direction(0=N, 1=E, 2=S, 3=W); markCount is no. carried markers
struct Robot{
    int x;
    int y;
    int dir;
    int markCount;
};

struct Tile{
    int type; //0=empty, 1=obstacle, 2=marker
    int collected;
};

//represents grid: width and height dimensions, and pointer to array of grid tiles
struct Grid{
    int width;
    int height;
    struct Tile **grid;
};

//represents marker: x,y position and collected(0=F, 1=T)
struct Marker{
    int x;
    int y;
};

struct Robot* initRobot(int x, int y, int dir){
    struct Robot *robot = malloc(sizeof(struct Robot));
    
        
    if (!robot) {
        printf("Robot memory allocation failed!\n");
        exit(1);
    }

    robot->x = x;
    robot->y = y;
    robot->dir = dir;

    return robot;
}

//initializes grid, 0=empty, 1=obstacle, 2=marker
struct Grid initGrid(int width, int height) {
    struct Grid grid;
    grid.width = width;
    grid.height = height;

    // allocate 2D array
    grid.grid = malloc(height * sizeof(struct Tile*));
    for (int i = 0; i < height; i++) {
        grid.grid[i] = malloc(width * sizeof(struct Tile));
    }

    // initialize tiles
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            grid.grid[i][j].type = 0;
            grid.grid[i][j].collected = 0;
        }
    }

    return grid;
}

// initialize markers array
struct Marker* initMarkers(int maxMarkers, int *noMarkers, struct Grid grid){
    //random no. markers generated(min 1)
    int numMarkers = 1 + rand() % maxMarkers;
    *noMarkers = numMarkers;
    struct Marker *markers = calloc(numMarkers, sizeof(struct Marker)); //zero-initialized


    //assigns random position for each marker
    for (int i = 0; i < numMarkers; i++) {
        markers[i].x = rand() % grid.width;
        markers[i].y = rand() % grid.height;
        message("hello");
        
    }

    return markers;

}

//draws grid
void drawGrid(struct Grid grid){
    int tileWidth = 40;
    int tileHeight = 40;
    int margin = 10;
    int ovalSize = 20;

    //set window size
    setWindowSize(tileWidth * grid.width + 2 * margin, tileHeight * grid.height + 2 * margin);

    //draws walls
    fillRect(0, 0, tileWidth * grid.width + margin, margin);
    fillRect(0, 0, margin, tileHeight * grid.height + margin);
    fillRect(0, tileHeight * grid.height + margin, tileWidth * grid.width + margin, margin);
    fillRect(tileWidth * grid.width + margin, 0, margin, tileHeight * grid.height + 2 * margin);

    //draw grid
    for(int w = 0; w < grid.width; w++){

        for(int h = 0; h < grid.height; h++){
            
            drawRect(w * tileWidth + margin, h * tileHeight+margin, tileWidth, tileHeight);
            
            //draw markers and obstacles
            switch(grid.grid[h][w].type){
                case OBS:
                    fillRect(w * tileWidth + margin + 5, h * tileHeight+margin + 5, tileWidth - 10, tileHeight - 10);
                    break;

                case MARK:
                    setColour(red);
                    fillOval(
                    w * tileWidth + margin + tileWidth/2 - ovalSize/2,
                    h * tileHeight + margin + tileHeight/2 - ovalSize/2,
                    ovalSize,
                    ovalSize
                    );
                    setColour(black);
                    break;
                }
            
        }
        
    }

}

void left(struct Robot *robot){
    //update direction
    robot->dir = (robot->dir + 3)%4;

    //redraw
}

void right(struct Robot *robot){
    //update direction
    robot->dir = (robot->dir + 1)%4;

    //redraw
}

int atMarker(struct Robot *robot, struct Grid grid){
    if (grid.grid[robot->y][robot->x].type == MARK){
        return 1;
    }
    return 0;
}

int canMoveForward(struct Robot *robot, struct Grid grid){
    int x = robot->x;
    int y = robot->y;

    switch(robot->dir){
        case 0:
            y += 1;
            break;
        
        case 1:
            x += 1;
            break;

        case 2:
            y -= 1;
            break;

        case 3:
            x -= 1;
            break;
    }

    if(grid.grid[y][x].type != OBS){
        return 1;
    }
    return 0;
}

void forward(struct Robot *robot, struct Grid grid){
    //update position
    if(canMoveForward(robot, grid)){
        switch(robot->dir){
            case 0:
                robot->y += 1;
                break;
            
            case 1:
                robot->x += 1;
                break;

            case 2:
                robot->y -= 1;
                break;

            case 3:
                robot->x -= 1;
                break;
        }
    }

    //redraw
}

void pickUpMarker(struct Robot *robot, struct Grid grid){
    if(atMarker(robot, grid) && grid.grid[robot->y][robot->x].collected == 0){
        grid.grid[robot->y][robot->x].type = MARK;
        grid.grid[robot->y][robot->x].collected = 1;
    }
}

void dropMarker(){

}



int main(int argc, char **argv){
     struct Robot *robot = NULL;
    struct Grid grid;
    struct Marker *markers = NULL;
    int noMarkers;

    //initialize with command line arguements
    if (argc == 7){
        robot = initRobot(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
        grid = initGrid(atoi(argv[4]), atoi(argv[5]));
        markers = initMarkers(atoi(argv[6]), &noMarkers, grid);
    }

    //set marker tiles
    for (int i = 0; i < noMarkers; i++) {
        grid.grid[markers[i].y][markers[i].x].type = MARK;
    
    }

    drawGrid(grid);
}


