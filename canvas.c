#include <ncurses.h>

#define C_SIZE 256

WINDOW *new_canvas(int size_x, int size_y, int *canvas_height, int *canvas_width, char canvas[C_SIZE][C_SIZE])
{
    echo();
    WINDOW *size_prompt = newwin(4, 26, (size_y-2)/2, (size_x-26)/2);
    box(size_prompt, 0, 0);

    //prompts the user for the size of the new canvas
    mvwprintw(size_prompt, 1, 1, "Enter canvas size: ");
    wscanw(size_prompt, "%d %d", canvas_height, canvas_width);
    erase();
    refresh();

    //clears the array 
    for (int i = 0; i < C_SIZE; i++)
    {
        for (int j = 0; j < C_SIZE; j++)
        {
            canvas[i][j] = 0x20; //should be 0x20 for space
        }
    }
    //??
    WINDOW *local_canvas;
    local_canvas = newwin(*canvas_height+2, *canvas_width+2, (size_y-*canvas_height)/2, (size_x-*canvas_width)/2);
    box(local_canvas, 0, 0);

    for (int i = 0; i <= *canvas_height+1; i++)
    {
        for (int j = 0; j <= *canvas_width+1; j++)
        {
            mvwaddch(local_canvas, i, j, canvas[i][j]);
        }
    }
    wrefresh(local_canvas);
    noecho();
    return local_canvas;
}

void init_menu(int size_x, int size_y, int canvas_height, int canvas_width, char ascii_list[], int current_char, int line_1_x, int line_1_y, int line_2_x, int line_2_y)
{
    //top bar
    WINDOW *menu = newwin(1, size_x, 0, 0);

    wattr_on(menu, A_STANDOUT, NULL);
    wprintw(menu, "Size: %dx%d | (F1) New Canvas | (B)rush | (L)ine | Line: %d %d - %d %d", canvas_height, canvas_width, line_1_x, line_1_y, line_2_x, line_2_y);
    wrefresh(menu);

    //character picker
    WINDOW *picker = newwin((93 / (size_y-2))+2, size_x-2, size_y-((93 / (size_y-2))+2), 1); //??
    box(picker, 0, 0);

    for (int i = 0; i <= 93; i++)
    {
        mvwaddch(picker, i/(size_x-4)+1, (i%(size_x-4))+1, ascii_list[i]); //size-x - 4 per row
        if (i == current_char) mvwaddch(picker, i/(size_x-4)+1, (i%(size_x-4))+1, ascii_list[i] | A_STANDOUT);
    }

    wrefresh(picker);

    //tool picker
    //todo
}

void init_ascii_list(char ascii_list[])
{
    for (int i = 0; i <= 93; i++) ascii_list[i] = i+33;
}

void update_canvas(WINDOW *c_win, char c[C_SIZE][C_SIZE], int *canvas_height, int *canvas_width, int row, int col, int prev_row, int prev_col)
{
    for (int i = 0; i <= *canvas_height+1; i++)
    {
        for (int j = 0; j <= *canvas_width+1; j++)
        {
            mvwaddch(c_win, i, j, c[i][j]);
        }
    }

    mvwaddch(c_win, row, col, c[row][col] | A_STANDOUT);
    if(prev_row != row || prev_col != col) mvwaddch(c_win, prev_row, prev_col, c[prev_row][prev_col] | A_NORMAL);

    box(c_win, 0, 0);
    wrefresh(c_win);
}

void draw_line(int line_1_x, int line_1_y, int line_2_x, int line_2_y, char canvas[C_SIZE][C_SIZE], char temp_canvas[C_SIZE][C_SIZE], int current_char, char ascii_list[93])
{
    // a temp canvas is created and the original canvas is copied every time the function is called
    // the temporary line is drawn based on the current 4 coords
    // if space is pressed again, the temp canvas is copied to the original
    for (int i = 0; i < C_SIZE; i++)
    {
        for (int j = 0; j < C_SIZE; j++)
        {
            temp_canvas[i][j] = canvas[i][j];
        }
    }
    // swap so that 1 is always above 2
    if (line_1_x > line_2_x)
    {
        int swap_x = line_1_x;
        int swap_y = line_1_y;
        line_1_x = line_2_x;
        line_1_y = line_2_y;
        line_2_x = swap_x;
        line_2_y = swap_y;
    }

    // special case, horizontal line
    if (line_1_x == line_2_x)
    {
        if (line_1_y <= line_2_y) for (int j = line_1_y; j <= line_2_y; j++) temp_canvas[line_1_x][j] = ascii_list[current_char];
        else for (int j = line_2_y; j <= line_1_y; j++) temp_canvas[line_1_x][j] = ascii_list[current_char];
    }

    // line raster alg
    // 1x, 1y - point 1
    // 2x, 2y - point 2
    else
    {
        for (int i = 0; i < C_SIZE; i++)
        {
            if (i >= line_1_x && i <= line_2_x) //scanline raster, checks if we are between the two points
            {
                int j = line_1_y + ((line_2_y-line_1_y) * (i - line_1_x)/(line_2_x - line_1_x));
                temp_canvas[i][j] = ascii_list[current_char]; //fix
            }
        }
    }
}

int main()
{	
	char canvas[C_SIZE][C_SIZE];
    char temp_canvas[C_SIZE][C_SIZE];
    char ascii_list[93];
    int current_tool = 0; // 0 - brush, 1 - line
    int current_char = 0;
    
    int row = 2;
    int col = 2;
    int canvas_height = 20, canvas_width = 64;
    int size_y = 0, size_x = 0;

    int line_1_x = 0, line_1_y = 0, line_2_x = 0, line_2_y = 0;
    int drawing_line = 0;

    WINDOW *c_win;

    initscr();
    curs_set(0);   
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, size_y, size_x);

    c_win = new_canvas(size_x, size_y, &canvas_height, &canvas_width, canvas);
    init_ascii_list(ascii_list);
    init_menu(size_x, size_y, canvas_height, canvas_width, ascii_list, current_char, line_1_x, line_1_y, line_2_x, line_2_y);
    

    row = 1;
    col = 1;
    int prev_row = 1, prev_col = 1;

    noecho();
    mvwaddch(c_win, row, col, canvas[row-2][col-2] | A_STANDOUT);
    box(c_win, 0, 0);
    wrefresh(c_win);
    int input = 0;
    while (input != '`'){
        input = getch();
        prev_row = row;
        prev_col = col;
        if (input == KEY_UP && row > 1) row--;
        if (input == KEY_DOWN && row < canvas_height) row++;
        if (input == KEY_LEFT && col > 1) col--;
        if (input == KEY_RIGHT && col < canvas_width) col++;

        if (drawing_line == 1)
        {
            line_2_x = row;
            line_2_y = col;
            init_menu(size_x, size_y, canvas_height, canvas_width, ascii_list, current_char, line_1_x, line_1_y, line_2_x, line_2_y);
            draw_line(line_1_x, line_1_y, line_2_x, line_2_y, canvas, temp_canvas, current_char, ascii_list);
        }

        if (input == ' ')
        {
            if (current_tool == 0) canvas[row][col] = ascii_list[current_char]; // brush tool
            else if (current_tool == 1) // line tool
            {
                if (drawing_line == 0)
                {
                    drawing_line = 1;
                    line_1_x = row;
                    line_1_y = col;
                    for (int i = 0; i < C_SIZE; i++)
                    {
                        for (int j = 0; j < C_SIZE; j++)
                        {
                            temp_canvas[i][j] = canvas[i][j];
                        }
                    }
                    init_menu(size_x, size_y, canvas_height, canvas_width, ascii_list, current_char, line_1_x, line_1_y, line_2_x, line_2_y);
                }
                else 
                {
                    for (int i = 0; i < C_SIZE; i++)
                    {
                        for (int j = 0; j < C_SIZE; j++)
                        {
                            canvas[i][j] = temp_canvas[i][j];
                        }
                    }
                    drawing_line = 0;
                }
            }
        }

        if (input == KEY_F(1))
        {
            erase();
            refresh();
            c_win = new_canvas(size_x, size_y, &canvas_height, &canvas_width, canvas);
            init_menu(size_x, size_y, canvas_height, canvas_width, ascii_list, current_char, line_1_x, line_1_y, line_2_x, line_2_y);
        }
        if (input == ']' && current_char < 93)
        {
            current_char++; 
            init_menu(size_x, size_y, canvas_height, canvas_width, ascii_list, current_char, line_1_x, line_1_y, line_2_x, line_2_y);
        }
        if (input == '[' && current_char > 0) 
        {
            current_char--; 
            init_menu(size_x, size_y, canvas_height, canvas_width, ascii_list, current_char, line_1_x, line_1_y, line_2_x, line_2_y);
        }

        
        if (input == 'b')
        {
            current_tool = 0;
        }
        
        if (input == 'l')
        {
            current_tool = 1;
        }

        if (drawing_line == 1) update_canvas(c_win, temp_canvas, &canvas_height, &canvas_width, row, col, prev_row, prev_col);
        else update_canvas(c_win, canvas, &canvas_height, &canvas_width, row, col, prev_row, prev_col);
    }
    
    endwin();

	return 0;
}