#include <ncurses.h>

#define C_SIZE 512

WINDOW *new_canvas(int size_x, int size_y, int *canvas_height, int *canvas_width, char canvas[C_SIZE][C_SIZE])
{
    // prompts the user to select a size for the new canvas and initializes it
    
    echo();
    WINDOW *size_prompt = newwin(6, 32, (size_y-6)/2, (size_x-32)/2);
    box(size_prompt, 0, 0);

    // size prompt
    int valid_size = 0;
    while (!valid_size)
    {
        mvwprintw(size_prompt, 1, 1, "Enter canvas width: ");
        wscanw(size_prompt, "%d", canvas_width);
    	mvwprintw(size_prompt, 1, 1, "Enter canvas height:     ");
        wmove(size_prompt, 1, 22);
    	wscanw(size_prompt, "%d", canvas_height);
    	if (*canvas_height < (size_y-7) && *canvas_width < (size_x-7)) valid_size = 1;
	    erase();
        refresh();
	    if (valid_size) break;
	    werase(size_prompt);
	    mvwprintw(size_prompt, 3, 1, "Canvas is too large");
	    box(size_prompt, 0, 0);
    }
    // clears the array 
    for (int i = 0; i < C_SIZE; i++)
    {
        for (int j = 0; j < C_SIZE; j++)
        {
            canvas[i][j] = 0x20; // should be 0x20 for space
        }
    }
    // ??
    WINDOW *local_canvas;
    local_canvas = newwin(*canvas_height+2, *canvas_width+2, (size_y-*canvas_height)/2, (size_x-*canvas_width-7)/2+6);
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

void update_menu(int size_x, int size_y, int canvas_height, int canvas_width, char ascii_list[], int current_char, int current_tool, int line_1_x, int line_1_y, int line_2_x, int line_2_y, int help_mode, int show_ui)
{
    // initializes/updates all the menu elements
    
    // top bar
    WINDOW *menu = newwin(1, size_x, 0, 0);

    wattr_on(menu, A_STANDOUT, NULL);
    wprintw(menu, "Size: %dx%d | (F1) Show/Hide Help | (F2) New Canvas | (F3) Save to file | (Q) Quit | Line: %d %d - %d %d", canvas_height, canvas_width, line_1_x, line_1_y, line_2_x, line_2_y);
    wrefresh(menu);

    // character picker
    WINDOW *picker = newwin((93 / (size_y-2))+2, size_x-2, size_y-((93 / (size_y-2))+2), 1); // ??
    box(picker, 0, 0);
    if (help_mode) mvwprintw(picker, 0, 3, "Use [ or ] to change");

    for (int i = 0; i <= 93; i++)
    {
        mvwaddch(picker, i/(size_x-4)+1, (i%(size_x-4))+1, ascii_list[i]); // size-x - 4 per row
        if (i == current_char) mvwaddch(picker, i/(size_x-4)+1, (i%(size_x-4))+1, ascii_list[i] | A_STANDOUT);
    }

    wrefresh(picker);

    // tool picker
    WINDOW *tool_picker;
    if (help_mode) 
    {
        tool_picker = newwin(7, 10, (size_y-canvas_height)/2, 1);    
        box(tool_picker, 0, 0);
        if (current_tool == 0) wattr_on(tool_picker, A_STANDOUT, NULL);
        mvwaddstr(tool_picker, 1, 1, " Brush  ");
        wattr_off(tool_picker, A_STANDOUT, NULL);
        mvwhline(tool_picker, 2, 1, ACS_HLINE, 3);
        if (current_tool == 1) wattr_on(tool_picker, A_STANDOUT, NULL);
        mvwaddstr(tool_picker, 3, 1, " Line   ");
        wattr_off(tool_picker, A_STANDOUT, NULL);
        mvwhline(tool_picker, 4, 1, ACS_HLINE, 3);
        if (current_tool == 2) wattr_on(tool_picker, A_STANDOUT, NULL);
        mvwaddstr(tool_picker, 5, 1, " Eraser ");
        wattr_off(tool_picker, A_STANDOUT, NULL);
    }
    else
    {
        tool_picker = newwin(7, 5, (size_y-canvas_height)/2, 1);
        box(tool_picker, 0, 0);
    	if (current_tool == 0) wattr_on(tool_picker, A_STANDOUT, NULL);
	mvwaddstr(tool_picker, 1, 1, " B ");
    	wattr_off(tool_picker, A_STANDOUT, NULL);
    	mvwhline(tool_picker, 2, 1, ACS_HLINE, 3);
    	if (current_tool == 1) wattr_on(tool_picker, A_STANDOUT, NULL);
    	mvwaddstr(tool_picker, 3, 1, " L ");
    	wattr_off(tool_picker, A_STANDOUT, NULL);
    	mvwhline(tool_picker, 4, 1, ACS_HLINE, 3);
    	if (current_tool == 2) wattr_on(tool_picker, A_STANDOUT, NULL);
    	mvwaddstr(tool_picker, 5, 1, " E ");
    	wattr_off(tool_picker, A_STANDOUT, NULL);
    }
    wrefresh(tool_picker);
    if (!show_ui)
    {
	erase();
	refresh();
    }
}

void init_ascii_list(char ascii_list[])
{
    // initializes the ascii list with all the valid characters

    for (int i = 0; i <= 93; i++) ascii_list[i] = i+33;
}

void update_canvas(WINDOW *c_win, char c[C_SIZE][C_SIZE], int start_x, int start_y, int *canvas_height, int *canvas_width, int row, int col, int prev_row, int prev_col)
{
    // updates the canvas window based on the selected canvas array

    for (int i = 0; i <= *canvas_height+1; i++)
    {
        for (int j = 0; j <= *canvas_width+1; j++)
        {
            mvwaddch(c_win, i, j, c[i+start_x][j+start_y]);
        }
    }

    mvwaddch(c_win, row, col, c[row][col] | A_STANDOUT);
    if(prev_row != row || prev_col != col) mvwaddch(c_win, prev_row, prev_col, c[prev_row][prev_col] | A_NORMAL);

    box(c_win, 0, 0);
    wrefresh(c_win);
}

void draw_line(int line_1_x, int line_1_y, int line_2_x, int line_2_y, char canvas[C_SIZE][C_SIZE], char temp_canvas[C_SIZE][C_SIZE], int current_char, char ascii_list[93])
{
    // draws a line from line_1 to line_2. uses a temporary canvas for the preview 

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

    // special case, horizontal line (might not be needed after horizontal scanline alg)
    if (line_1_x == line_2_x)
    {
        if (line_1_y <= line_2_y) for (int j = line_1_y; j <= line_2_y; j++) temp_canvas[line_1_x][j] = ascii_list[current_char];
        else for (int j = line_2_y; j <= line_1_y; j++) temp_canvas[line_1_x][j] = ascii_list[current_char];
    }
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
    // to do: choose between vertical and horizontal rasterization dependin on the angle (calculate slope?)
}

void move_selection(int sel_1_x, int sel_1_y, int sel_2_x, int sel_2_y, int x_offset, int y_offset, char canvas[C_SIZE][C_SIZE], char temp_canvas[C_SIZE][C_SIZE], int canvas_height, int canvas_width)
{
    if (y_offset > 0)
	for (int i = (canvas_height*2)+2; i >= canvas_height+2; i--)
	    for (int j = canvas_width+2; j <= (canvas_width*2)+2; j++)
		temp_canvas[i+y_offset][j] = temp_canvas[i][j];

    else if (y_offset < 0)
	for (int i = canvas_height+2; i <= (canvas_height*2)+2; i++)
	    for (int j = canvas_width+2; j <= (canvas_width*2)+2; j++)
		temp_canvas[i+y_offset][j] = temp_canvas[i][j];

    else if (x_offset > 0)
	for (int j = (canvas_width*2)+2; j >= canvas_width+2; j--)
	    for (int i = canvas_height+2; i <= (canvas_height*2)+2; i++)
		temp_canvas[i][j+x_offset] = temp_canvas[i][j];

    else if (x_offset < 0)
	for (int j = canvas_width+2; j <= (canvas_width*2)+2; j++)
	    for (int i = canvas_height+2; i <= (canvas_height*2)+2; i++)
		temp_canvas[i][j+x_offset] = temp_canvas[i][j];
}

void export_to_file(char canvas[C_SIZE][C_SIZE], int size_x, int size_y, int canvas_height, int canvas_width)
{
    char filename[32];
    
    echo();
    WINDOW *filename_prompt = newwin(4, 26, (size_y-2)/2, (size_x-26)/2);
    box(filename_prompt, 0, 0);

    mvwprintw(filename_prompt, 1, 1, "Enter filename: ");
    wscanw(filename_prompt, "%s", filename);
    erase();
    noecho();
    refresh();

    FILE *f = fopen(filename, "w");
    for (int i = 1; i <= canvas_height+1; i++)
    {
        fwrite(canvas[i], 1, canvas_width+1, f);
        fprintf(f, "\n");
    }

    fclose(f);
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
    int sel_1_x = 0, sel_1_y = 0, sel_2_x = canvas_width, sel_2_y = canvas_height; // remember to update whenever a new canvas is made 
    int drawing_line = 0;
    int help_mode = 0;
    int show_ui = 1;

    WINDOW *c_win;

    initscr();
    curs_set(0);   
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, size_y, size_x);

    c_win = new_canvas(size_x, size_y, &canvas_height, &canvas_width, canvas);
    init_ascii_list(ascii_list);
    update_menu(size_x, size_y, canvas_height, canvas_width, ascii_list, current_char, current_tool, line_1_x, line_1_y, line_2_x, line_2_y, help_mode, show_ui);
    

    row = 1;
    col = 1;
    int prev_row = 1, prev_col = 1;

    noecho();
    mvwaddch(c_win, row, col, canvas[row-2][col-2] | A_STANDOUT);
    box(c_win, 0, 0);
    wrefresh(c_win);
    int input = 0;
    while (input != 'q'){
        input = getch();
        prev_row = row;
        prev_col = col;
	if (current_tool != 3)
	{
            if (input == KEY_UP && row > 1) row--;
            if (input == KEY_DOWN && row < canvas_height) row++;
            if (input == KEY_LEFT && col > 1) col--;
            if (input == KEY_RIGHT && col < canvas_width) col++;
	}
	else // move mode
	{
            if (input == KEY_UP) move_selection(sel_1_x, sel_1_y, sel_2_x, sel_2_y, 0, -1, canvas, temp_canvas, canvas_height, canvas_width);
            if (input == KEY_DOWN) move_selection(sel_1_x, sel_1_y, sel_2_x, sel_2_y, 0, 1, canvas, temp_canvas, canvas_height, canvas_width);
            if (input == KEY_LEFT) move_selection(sel_1_x, sel_1_y, sel_2_x, sel_2_y, -1, 0, canvas, temp_canvas, canvas_height, canvas_width);
            if (input == KEY_RIGHT) move_selection(sel_1_x, sel_1_y, sel_2_x, sel_2_y, 1, 0, canvas, temp_canvas, canvas_height, canvas_width);
	}

        if (drawing_line == 1)
        {
            line_2_x = row;
            line_2_y = col;
            //update_menu(size_x, size_y, canvas_height, canvas_width, ascii_list, current_char, current_tool, line_1_x, line_1_y, line_2_x, line_2_y);
            draw_line(line_1_x, line_1_y, line_2_x, line_2_y, canvas, temp_canvas, current_char, ascii_list);
        }

        // inputs:

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
	    else if (current_tool == 2) canvas[row][col] = ' '; // eraser tool
	    else if (current_tool == 3) // move tool
	    {
		for (int i = 0; i <= canvas_height+1; i++)
                {
                    for (int j = 0; j <= canvas_width+1; j++)
                    {
                        canvas[i][j] = temp_canvas[i+canvas_height+2][j+canvas_width+2];
                    }
		}
		current_tool = 1;
	    }
        }

	if (input == KEY_F(1))
	{
	    if (!help_mode) help_mode = 1;
	    else
	    {
		help_mode = 0;
		erase();
		refresh();
	    }
	}
        if (input == KEY_F(2))
        {
            erase();
            refresh();
            c_win = new_canvas(size_x, size_y, &canvas_height, &canvas_width, canvas);
        }
        if (input == KEY_F(3))
        {
            export_to_file(canvas, size_x, size_y, canvas_height, canvas_width);
        }
	if (input == KEY_F(4))
	{
	    if (show_ui == 1) show_ui = 0;
	    else show_ui = 1;
	}
        if (input == ']' && current_char < 93)
        {
            current_char++; 
        }
        if (input == '[' && current_char > 0) 
        {
            current_char--; 
        }
        
        if (input == 'b')
        {
            current_tool = 0;
	    drawing_line = 0;
        }
        
        if (input == 'l')
        {
            current_tool = 1;
        }
	if (input == 'e')
	{
	    current_tool = 2;
	    drawing_line = 0;
	}
	if (input == 'm')
	{
	    current_tool = 3;
	    drawing_line = 0;
	    for (int i = 0; i <= canvas_height+1; i++)
   	    {
        	for (int j = 0; j <= canvas_width+1; j++)
                {
            	    temp_canvas[i+canvas_height+2][j+canvas_width+2] = canvas[i][j];
		}
	    }
	}

        // update:

	update_menu(size_x, size_y, canvas_height, canvas_width, ascii_list, current_char, current_tool, line_1_x, line_1_y, line_2_x, line_2_y, help_mode, show_ui);
        if (drawing_line == 1) update_canvas(c_win, temp_canvas, 0, 0, &canvas_height, &canvas_width, row, col, prev_row, prev_col);
	else if (current_tool == 3) update_canvas(c_win, temp_canvas, canvas_height+2, canvas_width+2, &canvas_height, &canvas_width, row, col, prev_row, prev_col);
        else update_canvas(c_win, canvas, 0, 0, &canvas_height, &canvas_width, row, col, prev_row, prev_col);
    }
    
    endwin();

	return 0;
}
