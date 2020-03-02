int q_learning_table[12][12];

void qLearningFoe(int pos_x, int pos_y, char season)
{
    int greatest = 0;
    int direction = -1;

    printf("[log] actual q-learning foe value %d\n", q_learning_table[pos_y][pos_x]);

    if (pos_y + 1 <  1024 && greatest < q_learning_table[pos_y + 1][pos_x]
    && (grid[pos_y + 1][pos_x].type == BLANK_COMP
    ||  grid[pos_y + 1][pos_x].type == PLAYER_COMP)) {
        greatest  = q_learning_table[pos_y + 1][pos_x];
        direction = 0;
    }
    if (pos_y - 1 >= 0    && greatest < q_learning_table[pos_y - 1][pos_x]
    && (grid[pos_y - 1][pos_x].type == BLANK_COMP
    ||  grid[pos_y - 1][pos_x].type == PLAYER_COMP)) {
        greatest  = q_learning_table[pos_y - 1][pos_x];
        direction = 1;
    }
    if (pos_x - 1 >= 0    && greatest < q_learning_table[pos_y][pos_x - 1]
    && (grid[pos_y][pos_x - 1].type == BLANK_COMP
    ||  grid[pos_y][pos_x - 1].type == PLAYER_COMP)) {
        greatest  = q_learning_table[pos_y][pos_x - 1];
        direction = 2;
    }
    if (pos_x + 1 <  1024 && greatest < q_learning_table[pos_y][pos_x + 1]
    && (grid[pos_y][pos_x + 1].type == BLANK_COMP
    ||  grid[pos_y][pos_x + 1].type == PLAYER_COMP)) {
        greatest  = q_learning_table[pos_y][pos_x + 1];
        direction = 3;
    }

    int next_x = pos_x;
    int next_y = pos_y;
    switch (direction) {
    case 0:
        next_y++;
        break;
    case 1:
        next_y--;
        break;
    case 2:
        next_x--;
        break;
    case 3:
        next_x++;
        break;
    }
    if (direction == -1) {
        for (;;) {
            direction = rand() % 4;
            next_x = pos_x;
            next_y = pos_y;
            switch (direction) {
            case 0:
                next_y++;
                break;
            case 1:
                next_y--;
                break;
            case 2:
                next_x--;
                break;
            case 3:
                next_x++;
                break;
            }
            if ((next_x >= 0 && next_x < 9)
            &&  (next_y >= 0 && next_y < 9)
            &&  (grid[next_y][next_x].type == BLANK_COMP)) {
                break;
            }
        }
    } else {
        int tmp = (int)floor((double)greatest * 0.9);
        q_learning_table[pos_y][pos_x] = (q_learning_table[pos_y][pos_x] > tmp) ?
            q_learning_table[pos_y][pos_x] : tmp;
        if (grid[next_y][next_x].type == PLAYER_COMP) {
            grid[8][8] = grid[pos_y][pos_x];
            grid[pos_y][pos_x].type = BLANK_COMP;
            return;
        }
    }
    grid[next_y][next_x] = grid[pos_y][pos_x];
    printf("[log] cleaning %d, %d\n", pos_y, pos_x);
    grid[pos_y][pos_x].type = BLANK_COMP;
}

void initQLearningDemo()
{
    main_camera.x = 0;
    main_camera.y = 19;
    main_camera.v_block_qnt = 20;
    player_grid_pos_x = 0;
    player_grid_pos_y = 0;

    Component c;
    c.type = PLAYER_COMP;
    c.last_update_tick = 0;
    c.sprite = sprite_assets[1];
    c.update = NULL;
    grid[0][0] = c;
	initFoe(&grid[7][7], qLearningFoe, sprite_assets[0], 8);
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            q_learning_table[y][x] = 0;
        }
    }
    q_learning_table[0][0] = 100;
}