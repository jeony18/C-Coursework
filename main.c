#include <stdio.h>
#include <stdlib.h>
#include "graphics.h"
#include <time.h>
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

//represents tile: type, collected, x, y
struct Tile{
    int type; //0=empty, 1=obstacle, 2=marker
    int collected;
    int x;
    int y;
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
            grid.grid[i][j].x = j;
            grid.grid[i][j].y = i;
        }
    }

    return grid;
}


// initialize markers array
struct Marker* initMarkers(int maxMarkers, int *noMarkers, struct Grid grid){
    //random no. markers generated(min 1)
    int numMarkers =  5 + rand() % (maxMarkers-4);
    *noMarkers = numMarkers;
    struct Marker *markers = calloc(numMarkers, sizeof(struct Marker)); //zero-initialized


    //assigns random position for each marker
    for (int i = 0; i < numMarkers; i++) {
        int x;
        int y;
        do{
            x = rand() % grid.width;
            y = rand() % grid.width;
            markers[i].x = x;
            markers[i].y = y;
        }while(grid.grid[y][x].type == OBS);
        
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


struct Tile *randomNeighbor(int x, int y, struct Grid grid, int *found) {
    struct Tile *neighbors[4]; // temporary array of pointers
    int n = 0;

    if(x > 0 && grid.grid[y][x-1].type == 0) {
        struct Tile *t = &grid.grid[y][x-1];
        neighbors[n++] = t;
    }

    if(x < grid.width-1 && grid.grid[y][x+1].type == 0) {
        struct Tile *t = &grid.grid[y][x+1];
        neighbors[n++] = t;
    }

    if(y > 0 && grid.grid[y-1][x].type == 0) {
        struct Tile *t = &grid.grid[y-1][x];
        neighbors[n++] = t;
    }

    if(y < grid.height-1 && grid.grid[y+1][x].type == 0) {
        struct Tile *t = &grid.grid[y+1][x];
        neighbors[n++] = t;
    }

    if(n == 0) {
        *found = 0;
        return NULL;
    }

    *found = 1;
    int idx = rand() % n;
    struct Tile *result = neighbors[idx];

    return result; // caller must free this Tile
}


//generate obstacle clusters
void genClusters(struct Grid grid){
    int minClusters = 3;
    int maxClusters = 8;
    int clusterCount = minClusters + rand() % (maxClusters - minClusters + 1);
    int totalTiles = grid.width * grid.height;
    int targetTilesPerCluster = (totalTiles * 0.4) / clusterCount;

    for(int c = 0; c < clusterCount; c++){
        int index = 0;
        struct Tile **clusterTiles = calloc(targetTilesPerCluster, sizeof(struct Tile*));
        struct Tile **frontier = calloc(targetTilesPerCluster, sizeof(struct Tile*));
        int frontierCount = 0;

        int x, y;
        int side = rand() % 4;
        if(side < 2){
            y = (side == 0) ? 0 : grid.height - 1;
            x = rand() % grid.width;
        } else {
            x = (side == 2) ? 0 : grid.width - 1;
            y = rand() % grid.height;
        }

        grid.grid[y][x].type = OBS;
        clusterTiles[index++] = &grid.grid[y][x];
        frontier[frontierCount++] = &grid.grid[y][x];

        while(index < targetTilesPerCluster && frontierCount > 0){
            int fidx = rand() % frontierCount;
            struct Tile *choice = frontier[fidx];

            int found;
            struct Tile *neighbor = randomNeighbor(choice->x, choice->y, grid, &found);

            if(!found){
                frontier[fidx] = frontier[--frontierCount];
                continue;
            }

            neighbor->type = OBS;
            clusterTiles[index++] = neighbor;
            frontier[frontierCount++] = neighbor;
        }

        free(clusterTiles);
        free(frontier);
    }
}

int closed(struct Tile tile, struct Grid grid) {
    if(tile.x > 0 && grid.grid[tile.y][tile.x - 1].type != OBS) return 0;
    if(tile.x < grid.width - 1 && grid.grid[tile.y][tile.x + 1].type != OBS) return 0;
    if(tile.y > 0 && grid.grid[tile.y - 1][tile.x].type != OBS) return 0;
    if(tile.y < grid.height - 1 && grid.grid[tile.y + 1][tile.x].type != OBS) return 0;

    return 1; 
}

void fillGaps(struct Grid grid){
    int width = grid.width;
    int height = grid.height;

    for (int x = 0; x < width; x++) {
        int y = 0;
        if(closed(grid.grid[y][x], grid)){
            grid.grid[y][x].type = OBS;
        }
    }

    for (int y = 1; y < height; y++) {
        int x = width - 1;
        if(closed(grid.grid[y][x], grid)){
            grid.grid[y][x].type = OBS;
        }
    }

    for (int x = width - 2; x >= 0; x--) {
        int y = height - 1;
        if(closed(grid.grid[y][x], grid)){
            grid.grid[y][x].type = OBS;
        }
    }

    for (int y = height - 2; y > 0; y--) {
        int x = 0;
        if(closed(grid.grid[y][x], grid)){
            grid.grid[y][x].type = OBS;
        }
    }
}

void drawRobot(struct Robot *robot){
    foreground();
    setColour(blue);
    int cx = robot->x * TILE_WIDTH + MARGIN + TILE_WIDTH / 2;
    int cy = robot->y * TILE_HEIGHT + MARGIN + TILE_HEIGHT / 2;
    int size = TILE_WIDTH / 2;
    int vx[3], vy[3];
    
    switch(robot->dir){
        case NORTH:
            vx[0] = cx;
            vy[0] = cy - size/2;
            vx[1] = cx - size/2;
            vy[1] = cy + size/2;
            vx[2] = cx + size/2;
            vy[2] = cy + size/2;
            break;
        case EAST:
            vx[0] = cx + size/2;
            vy[0] = cy;
            vx[1] = cx - size/2;
            vy[1] = cy - size/2;
            vx[2] = cx - size/2;
            vy[2] = cy + size/2;
            break;
        case SOUTH:
            vx[0] = cx;
            vy[0] = cy + size/2;
            vx[1] = cx - size/2;
            vy[1] = cy - size/2;
            vx[2] = cx + size/2;
            vy[2] = cy - size/2;
            break;
        case WEST:
            vx[0] = cx - size/2;
            vy[0] = cy;
            vx[1] = cx + size/2;
            vy[1] = cy - size/2;
            vx[2] = cx + size/2;
            vy[2] = cy + size/2;
            break;
    }
    
    fillPolygon(3, vx, vy);
    setColour(black);
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
    drawRobot(robot);

}



void redrawGrid(struct Grid grid, struct Robot *robot, struct Marker *markers, int noMarkers){
    foreground();
    clear();
    for (int i = 0; i < noMarkers; i++) {
        if (grid.grid[markers[i].y][markers[i].x].collected == 0){
            drawMarker(markers[i].x, markers[i].y);
        }
    }
    drawRobot(robot);
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
            case NORTH:
                robot->y -= 1;
                break;
            
            case EAST:
                robot->x += 1;
                break;

            case SOUTH:
                robot->y += 1;
                break;

            case WEST:
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

int dfs(struct Robot *robot, struct Grid grid, int **visited, struct Marker *markers, int noMarkers) {
    visited[robot->y][robot->x] = 1;

    if(pickUpMarker(robot, grid, noMarkers)) {
        return 1;
    }

    int dirs[4] = {NORTH, EAST, SOUTH, WEST};
    for(int i = 0; i < 4; i++) {
        int targetDir = dirs[i];
        while(robot->dir != targetDir) {
            right(robot);
        }

        if(canMoveForward(robot, grid)) {
            int prevX = robot->x;
            int prevY = robot->y;

            forward(robot, grid);
            redrawGrid(grid, robot, markers, noMarkers);

            if(!visited[robot->y][robot->x]) {
                if(dfs(robot, grid, visited, markers, noMarkers)) {
                    return 1;
                }
            }

            // Backtrack
            robot->x = prevX;
            robot->y = prevY;
            redrawGrid(grid, robot, markers, noMarkers);
        }
    }

    return 0;
}


int main(int argc, char **argv){
    srand(time(NULL));
    struct Robot *robot = NULL;
    struct Grid grid;
    struct Marker *markers = NULL;
    int noMarkers;


    //initialize with command line arguements: robot x, robot y, dir, grid width, grid height, no. markers
    robot = initRobot(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    grid = initGrid(atoi(argv[4]), atoi(argv[5]));
    genClusters(grid);
    //fillGaps(grid);
    markers = initMarkers(atoi(argv[6]), &noMarkers, grid);

    //set marker tiles
    for (int i = 0; i < noMarkers; i++) {
        grid.grid[markers[i].y][markers[i].x].type = MARK;
    
    }

    drawGrid(grid, robot);

    int **visited = malloc(grid.height * sizeof(int *));
    for(int i = 0; i < grid.height; i++) {
        visited[i] = calloc(grid.width, sizeof(int));
    }

    dfs(robot, grid, visited, markers, noMarkers);

    for(int i = 0; i < grid.height; i++) {
        free(visited[i]);
    }
    free(visited);

    
    
}


