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


//represents grid: width and height dimensions, and pointer to array of grid tiles, only the grid array will be altered so it is a pointer
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

//initializes robot
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


//initializes grid: 0=empty, 1=obstacle, 2=marker
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

    return result;
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

//draws robot
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
            drawRect(w * TILE_WIDTH + MARGIN, h * TILE_HEIGHT + MARGIN, TILE_WIDTH, TILE_HEIGHT);
            
            //draw markers as red circles and obstacles as black squares
            switch(grid.grid[h][w].type){
                case OBS:
                    fillRect(w * TILE_WIDTH + MARGIN + 5, h * TILE_HEIGHT + MARGIN + 5, TILE_WIDTH - 10, TILE_HEIGHT - 10);
                    break;

                case MARK:
                    drawMarker(w, h);
                    break;
                    background();
                }  
        }
    }

    //draw robot
    drawRobot(robot);

}

//redraws the foreground with updated robot positions and marker states
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

//updates dir, turning robot counterclockwise
void left(struct Robot *robot){
    robot->dir = (robot->dir + 3)%4;
}

//updates dir, turning robot clockwise
void right(struct Robot *robot){
    robot->dir = (robot->dir + 1)%4;
}

//checks if tile is an uncollected marker
int atMarker(struct Robot *robot, struct Grid grid){
    if (grid.grid[robot->y][robot->x].type == MARK && grid.grid[robot->y][robot->x].collected == 0){
        return 1;
    }
    return 0;
}

//checks there is a valid tile in the robot's direction
int canMoveForward(struct Robot *robot, struct Grid grid){
    int x = robot->x;
    int y = robot->y;
    
    switch(robot->dir){
        case 0:
            y -= 1;
            break;
        
        case 1:
            x += 1;
            break;

        case 2:
            y += 1;
            break;

        case 3:
            x -= 1;
            break;
    }
    if(x >= 0 && x < grid.width && y >= 0 && y < grid.height && grid.grid[y][x].type != OBS){
    return 1;
    }
    return 0;
}

//moves the robot forward if possible
void forward(struct Robot *robot, struct Grid grid){
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
}

//picks up marker by setting tile's collected attr to 1, increments markCount
void pickUpMarker(struct Robot *robot, struct Grid grid){
    grid.grid[robot->y][robot->x].collected = 1;
    robot->markCount += 1;
}

// move robot from current position to target position (adjacent tile)
void moveToAdjacent(struct Robot *robot, int targetX, int targetY, struct Grid grid) {
    if (robot->x < targetX) turnTo(robot, EAST);
    else if (robot->x > targetX) turnTo(robot, WEST);
    else if (robot->y < targetY) turnTo(robot, SOUTH);
    else if (robot->y > targetY) turnTo(robot, NORTH);

    forward(robot, grid);
}

// DFS algorithm
// explore entire grid using stacks
void dfsMove(struct Robot *robot, struct Grid grid, int **visited, struct Marker *markers, int noMarkers) {
    struct Tile stack[grid.width * grid.height];
    int top = 0;
    stack[top].x = robot->x;
    stack[top].y = robot->y;
    visited[robot->y][robot->x] = 1;

    while (top >= 0) {
        struct Tile current = stack[top];

        // moves robot to tile on top of stack and redraw frame to animate
        if (robot->x != current.x || robot->y != current.y) {
            moveToAdjacent(robot, current.x, current.y, grid);
            redrawGrid(grid, robot, markers, noMarkers);
            sleep(100);
        }

        // pick up marker if available
        if (atMarker(robot, grid)) {
            pickUpMarker(robot, grid);
            redrawGrid(grid, robot, markers, noMarkers);
            sleep(100);
            if (robot->markCount == noMarkers) break;
        }

        // explore neighbor "branches", adds them in NESW order
        int moved = 0;
        for (int i = 0; i < 4; i++) {
            int nx = robot->x;
            int ny = robot->y;

            switch (i) {
                case NORTH: 
                    ny -= 1; 
                    break;
                case EAST:  
                    nx += 1; 
                    break;
                case SOUTH: 
                    ny += 1; 
                    break;
                case WEST:  
                    nx -= 1;
                    break;
            }
            
            //depth first so move to the first branch then break
            if (nx >= 0 && nx < grid.width && ny >= 0 && ny < grid.height &&
                grid.grid[ny][nx].type != OBS && !visited[ny][nx]) {
                visited[ny][nx] = 1;
                stack[++top].x = nx;
                stack[top].y = ny;
                moved = 1;
                break;
            }
        }

        if (!moved) {
            // backtack once one branch has been fully explored
            top--;
        }
    }
}



int main(int argc, char **argv){
    srand(time(NULL));

    //initialize robot, grid, markers array, and generate obstacle clusters and then draw
    struct Robot *robot = initRobot(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    struct Grid grid = initGrid(atoi(argv[4]), atoi(argv[5]));
    genClusters(grid);

    int noMarkers;
    struct Marker *markers = initMarkers(atoi(argv[6]), &noMarkers, grid);
    for(int i = 0; i < noMarkers; i++){
        grid.grid[markers[i].y][markers[i].x].type = MARK;
    }

    drawGrid(grid, robot);

    
    int **visited = malloc(grid.height * sizeof(int *));
    for(int i = 0; i < grid.height; i++){
        visited[i] = calloc(grid.width, sizeof(int));
    }

    //robot logic loop
    while(robot->markCount < noMarkers){
        dfsMove(robot, grid, visited, markers, noMarkers);
    }
}
