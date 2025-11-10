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
#define TILE_WIDTH 40
#define TILE_HEIGHT 40
#define MARGIN 10
#define OVAL_SIZE 20

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
    robot->markCount = 0;

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


void drawMarker(int x, int y){
                foreground();
    setColour(red);
    fillOval(
    x * TILE_WIDTH + MARGIN + TILE_WIDTH/2 - OVAL_SIZE/2,
    y * TILE_HEIGHT + MARGIN + TILE_HEIGHT/2 - OVAL_SIZE/2,
    OVAL_SIZE,
    OVAL_SIZE
    );
    setColour(black);
}


void drawRobot(int x, int y){
    foreground();
    setColour(blue);
    fillRect(x * TILE_WIDTH + MARGIN + 10, y * TILE_HEIGHT + MARGIN + 10, TILE_WIDTH - 20, TILE_HEIGHT - 20);
}


//draws grid
void drawGrid(struct Grid grid, struct Robot *robot){

    //set window size
    setWindowSize(TILE_WIDTH * grid.width + 2 * MARGIN, TILE_HEIGHT * grid.height + 2 * MARGIN);

    //draws walls
    background();
    fillRect(0, 0, TILE_WIDTH * grid.width + MARGIN, MARGIN);
    fillRect(0, 0, MARGIN, TILE_HEIGHT * grid.height + MARGIN);
    fillRect(0, TILE_HEIGHT * grid.height + MARGIN, TILE_WIDTH * grid.width + MARGIN, MARGIN);
    fillRect(TILE_WIDTH * grid.width + MARGIN, 0, MARGIN, TILE_HEIGHT * grid.height + 2 * MARGIN);

    //draw grid
    for(int w = 0; w < grid.width; w++){

        for(int h = 0; h < grid.height; h++){
            background();
            drawRect(w * TILE_WIDTH + MARGIN, h * TILE_HEIGHT + MARGIN, TILE_WIDTH, TILE_HEIGHT);
            
            //draw markers and obstacles
            switch(grid.grid[h][w].type){
                case OBS:
                    fillRect(w * TILE_WIDTH + MARGIN + 5, h * TILE_HEIGHT + MARGIN + 5, TILE_WIDTH - 10, TILE_HEIGHT - 10);
                    break;

                case MARK:
                    drawMarker(w, h);
                    break;
                }  
        }
    }

    //draw robot
    drawRobot(robot->x, robot->y);

}


void redrawGrid(struct Grid grid, struct Robot *robot, struct Marker *markers, int noMarkers){
    foreground();
    clear();
    for (int i = 0; i < noMarkers; i++) {
        if (grid.grid[markers[i].y][markers[i].x].collected == 0){
            drawMarker(markers[i].x, markers[i].y);
        }
    }
    drawRobot(robot->x, robot->y);
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


//only used in pickUpMarker()
int atMarker(struct Robot *robot, struct Grid grid){
    if (grid.grid[robot->y][robot->x].type == MARK){
        return 1;
    }
    return 0;
}


//only used in forward
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


//picks up marker and returns true if all markers have been picked up
int pickUpMarker(struct Robot *robot, struct Grid grid, int noMarkers){
    if(atMarker(robot, grid) && grid.grid[robot->y][robot->x].collected == 0){
        grid.grid[robot->y][robot->x].type = MARK;
        grid.grid[robot->y][robot->x].collected = 1;
        robot->markCount += 1;

        if (robot->markCount == noMarkers){
            return 1;
        }
    }
    return 0;
}


void dropMarker(){

}


void update(struct Robot *robot, struct Grid grid){
    forward(robot, grid);


}


int main(int argc, char **argv){
     struct Robot *robot = NULL;
    struct Grid grid;
    struct Marker *markers = NULL;
    int noMarkers;
    int p;
    int q;


    //initialize with command line arguements: robot x, robot y, dir, grid width, grid height, no. markers
    if (argc == 7){
        robot = initRobot(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
        grid = initGrid(atoi(argv[4]), atoi(argv[5]));
        markers = initMarkers(atoi(argv[6]), &noMarkers, grid);
    }

    //set marker tiles
    for (int i = 0; i < noMarkers; i++) {
        grid.grid[markers[i].y][markers[i].x].type = MARK;
        p = markers[i].x;
        q = markers[i].y;
    
    }

    drawGrid(grid, robot);
    sleep(1000);    
    update(robot, grid);
    redrawGrid(grid, robot, markers, noMarkers);
    grid.grid[q][p].collected = 1;
    for(int i = 0;i<3;i++){
        sleep(1000);    
        update(robot, grid);
        redrawGrid(grid, robot, markers, noMarkers);
    }
    

    
    
}


