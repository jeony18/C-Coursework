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

