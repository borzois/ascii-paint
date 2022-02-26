#include <ncurses.h>
#include <string.h>

#define C_SIZE 512

typedef struct {
    int line_1_x, line_1_y, line_2_x, line_2_y;
} line;

typedef struct {
	int row;
	int col;
} position;

WINDOW *new_canvas(int size_x, int size_y, int *canvas_height, int *canvas_width, char canvas[C_SIZE][C_SIZE])
{
	// prompts the user to select a size for the new canvas and initializes it

	echo();
	WINDOW *size_prompt = newwin(6, 32, (size_y-6)/2, (size_x-32)/2);
	box(size_prompt, 0, 0);

	// size prompt
	int valid_size = 0;
	while (!valid_size) {
		mvwprintw(size_prompt, 1, 1, "Max size: %d", size_x-8);
		mvwprintw(size_prompt, 2, 1, "Enter canvas width: ");
		wscanw(size_prompt, "%d", canvas_width);
		mvwprintw(size_prompt, 1, 1, "Max size:  %d", size_y-8);
		mvwprintw(size_prompt, 2, 1, "Enter canvas height:       ");
		wmove(size_prompt, 2, 22);
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
		for (int j = 0; j < C_SIZE; j++)
			canvas[i][j] = 0x20; // should be 0x20 for space
	// ??
	WINDOW *local_canvas;
	local_canvas = newwin(*canvas_height+2, *canvas_width+2, (size_y-*canvas_height)/2, (size_x-*canvas_width-7)/2+6);
	box(local_canvas, 0, 0);

	for (int i = 0; i <= *canvas_height+1; i++)
		for (int j = 0; j <= *canvas_width+1; j++)
			mvwaddch(local_canvas, i, j, canvas[i][j]);

	wrefresh(local_canvas);
	noecho();
	return local_canvas;
}

void update_menu(int size_x, int size_y, int canvas_height, int canvas_width, char ascii_list[], int current_char, int current_tool, line new_line, int help_mode, int show_ui)
{
	// initializes/updates all the menu elements

	// top bar
	WINDOW *menu = newwin(1, size_x, 0, 0);

	wattr_on(menu, A_STANDOUT, NULL);
	wprintw(menu, "Size: %dx%d | (F1) Show/Hide Help | (F2) New Canvas | (F3) Save to file | (Q) Quit | Line: %d %d - %d %d", canvas_height, canvas_width, new_line.line_1_x, new_line.line_1_y, new_line.line_2_x, new_line.line_2_y);
	wrefresh(menu);

	// character picker
	WINDOW *picker = newwin((93 / (size_y-2))+2, size_x-2, size_y-((93 / (size_y-2))+2), 1); // ??
	box(picker, 0, 0);
	if (help_mode) mvwprintw(picker, 0, 3, "Use [ or ] to change");

	for (int i = 0; i <= 93; i++) {
		mvwaddch(picker, i/(size_x-4)+1, (i%(size_x-4))+1, ascii_list[i]); // size-x - 4 per row
		if (i == current_char) mvwaddch(picker, i/(size_x-4)+1, (i%(size_x-4))+1, ascii_list[i] | A_STANDOUT);
	}

	wrefresh(picker);

	// tool picker
	WINDOW *tool_picker;
	if (help_mode) {
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
	} else {
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
	if (!show_ui) {
		erase();
		refresh();
	}
}

void init_ascii_list(char ascii_list[])
{
	// initializes the ascii list with all the valid characters

	for (int i = 0; i <= 93; i++) 
		ascii_list[i] = i+33;
}

void update_canvas(WINDOW *c_win, char c[C_SIZE][C_SIZE], int start_x, int start_y, int *canvas_height, int *canvas_width, position pos, position prev_pos)
{
	// updates the canvas window based on the selected canvas array

	for (int i = 0; i <= *canvas_height+1; i++)
		for (int j = 0; j <= *canvas_width+1; j++)
			mvwaddch(c_win, i, j, c[i+start_x][j+start_y]);

	mvwaddch(c_win, pos.row, pos.col, c[pos.row][pos.col] | A_STANDOUT);
	if(prev_pos.row != pos.row || prev_pos.col != pos.col) 
		mvwaddch(c_win, prev_pos.row, prev_pos.col, c[prev_pos.row][prev_pos.col] | A_NORMAL);

	box(c_win, 0, 0);
	wrefresh(c_win);
}

void draw_line(line new_line, char canvas[C_SIZE][C_SIZE], char temp_canvas[C_SIZE][C_SIZE], int current_char, char ascii_list[93])
{
	// draws a line from line_1 to line_2. uses a temporary canvas for the preview 

	for (int i = 0; i < C_SIZE; i++)
		for (int j = 0; j < C_SIZE; j++)
			temp_canvas[i][j] = canvas[i][j];

	// swap so that 1 is always above 2
	if (new_line.line_1_x > new_line.line_2_x) {
		int swap_x = new_line.line_1_x;
		int swap_y = new_line.line_1_y;
		new_line.line_1_x = new_line.line_2_x;
		new_line.line_1_y = new_line.line_2_y;
		new_line.line_2_x = swap_x;
		new_line.line_2_y = swap_y;
	}

	// special case, horizontal line (might not be needed after horizontal scanline alg)
	if (new_line.line_1_x == new_line.line_2_x) {
		if (new_line.line_1_y <= new_line.line_2_y) for (int j = new_line.line_1_y; j <= new_line.line_2_y; j++) temp_canvas[new_line.line_1_x][j] = ascii_list[current_char];
		else for (int j = new_line.line_2_y; j <= new_line.line_1_y; j++) temp_canvas[new_line.line_1_x][j] = ascii_list[current_char];
	} else {
		for (int i = 0; i < C_SIZE; i++) {
			if (i >= new_line.line_1_x && i <= new_line.line_2_x) { //scanline raster, checks if we are between the two points
				int j = new_line.line_1_y + ((new_line.line_2_y-new_line.line_1_y) * (i - new_line.line_1_x)/(new_line.line_2_x - new_line.line_1_x));
				temp_canvas[i][j] = ascii_list[current_char]; //fix
			}
		}
	}
    // to do: choose between vertical and horizontal rasterization dependin on the angle (calculate slope?)
}

void move_selection(int sel_1_x, int sel_1_y, int sel_2_x, int sel_2_y, int x_offset, int y_offset, char canvas[C_SIZE][C_SIZE], char temp_canvas[C_SIZE][C_SIZE], int canvas_height, int canvas_width)
{
	if (y_offset > 0) // down
		for (int i = C_SIZE-2; i >= 0; i--)
			for (int j = 0; j < C_SIZE; j++)
				temp_canvas[i+y_offset][j] = temp_canvas[i][j];

	else if (y_offset < 0) // up
		for (int i = 1; i < C_SIZE; i++)
			for (int j = 0; j < C_SIZE; j++)
				temp_canvas[i+y_offset][j] = temp_canvas[i][j];

	else if (x_offset > 0) // right
		for (int j = C_SIZE-2; j >= 0; j--)
			for (int i = 0; i < C_SIZE; i++)
				temp_canvas[i][j+x_offset] = temp_canvas[i][j];

	else if (x_offset < 0) // left
		for (int j = 1; j < C_SIZE; j++)
			for (int i = 0; i < C_SIZE; i++)
				temp_canvas[i][j+x_offset] = temp_canvas[i][j];
}

void export_to_file(char canvas[C_SIZE][C_SIZE], int size_x, int size_y, int canvas_height, int canvas_width)
{
	char filename[32];

	echo();
	WINDOW *filename_prompt = newwin(4, 32, (size_y-2)/2, (size_x-32)/2);
	box(filename_prompt, 0, 0);

	mvwprintw(filename_prompt, 1, 1, "Enter filename:           .txt");
	wmove(filename_prompt, 1, 17);
	int len = wscanw(filename_prompt, "%s", filename);

	if (len != 0) {   // fix
		strcat(filename, ".txt");
		FILE *f = fopen(filename, "w");
		for (int i = 1; i <= canvas_height+1; i++) {
			fwrite(canvas[i], 1, canvas_width+1, f);
			fprintf(f, "\n");
		}
		fclose(f);
	}

	erase();
	noecho();
	refresh();
}

int main()
{	
	char canvas[C_SIZE][C_SIZE];
	char temp_canvas[C_SIZE][C_SIZE];
	char ascii_list[93];
	int current_tool = 0; 
	int current_char = 0;

	int canvas_height = 20, canvas_width = 64;
	int size_y = 0, size_x = 0;

	position pos, prev_pos;
	pos.row = 1, pos.col = 1;
	prev_pos.row = 1, prev_pos.col = 1;

	line new_line;
	new_line.line_1_x = 0, new_line.line_1_y = 0, new_line.line_2_x = 0, new_line.line_2_y = 0;

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
	update_menu(size_x, size_y, canvas_height, canvas_width, ascii_list, current_char, current_tool, new_line, help_mode, show_ui);

	noecho();
	mvwaddch(c_win, pos.row, pos.col, ' ' | A_STANDOUT);
	box(c_win, 0, 0);
	wrefresh(c_win);
	int input = 0;

	while (input != 'q'){
		input = getch();
		prev_pos.row = pos.row;
		prev_pos.col = pos.col;

		// arrow keys
		if (current_tool != 3) {
			if (input == KEY_UP && pos.row > 1) pos.row--;
			if (input == KEY_DOWN && pos.row < canvas_height) pos.row++;
			if (input == KEY_LEFT && pos.col > 1) pos.col--;
			if (input == KEY_RIGHT && pos.col < canvas_width) pos.col++;
		} else { // move mode
			if (input == KEY_UP) move_selection(sel_1_x, sel_1_y, sel_2_x, sel_2_y, 0, -1, canvas, temp_canvas, canvas_height, canvas_width);
			if (input == KEY_DOWN) move_selection(sel_1_x, sel_1_y, sel_2_x, sel_2_y, 0, 1, canvas, temp_canvas, canvas_height, canvas_width);
			if (input == KEY_LEFT) move_selection(sel_1_x, sel_1_y, sel_2_x, sel_2_y, -1, 0, canvas, temp_canvas, canvas_height, canvas_width);
			if (input == KEY_RIGHT) move_selection(sel_1_x, sel_1_y, sel_2_x, sel_2_y, 1, 0, canvas, temp_canvas, canvas_height, canvas_width);
		}
		// update after moving
		if (drawing_line == 1) {
			new_line.line_2_x = pos.row;
			new_line.line_2_y = pos.col;
			draw_line(new_line, canvas, temp_canvas, current_char, ascii_list);
        }

		// space/enter's function depends on the current tool
		if (input == ' ' || input == KEY_ENTER) {
			if (current_tool == 0)               // brush tool
				canvas[pos.row][pos.col] = ascii_list[current_char]; 

			else if (current_tool == 1) {        // line tool
				if (drawing_line == 0) {         // initializes a new line
					drawing_line = 1;
					new_line.line_1_x = pos.row;
					new_line.line_1_y = pos.col;
					for (int i = 0; i < C_SIZE; i++) 
						for (int j = 0; j < C_SIZE; j++) 
							temp_canvas[i][j] = canvas[i][j];
				} else {                         // completes the line
					for (int i = 0; i < C_SIZE; i++)
						for (int j = 0; j < C_SIZE; j++)
							canvas[i][j] = temp_canvas[i][j];
					drawing_line = 0;
				}
			}

			else if (current_tool == 2) 
				canvas[pos.row][pos.col] = ' ';          // eraser tool

			else if (current_tool == 3) {        // move tool - applies the transformation and sets the tool back to brush
				for (int i = 0; i <= canvas_height+1; i++)
						for (int j = 0; j <= canvas_width+1; j++)
							canvas[i][j] = temp_canvas[i+canvas_height+2][j+canvas_width+2];

				current_tool = 1;
			}
		}

		// help mode toggle
		if (input == KEY_F(1)) {
			if (!help_mode) 
				help_mode = 1;
			else {
				help_mode = 0;
				erase();
				refresh();
			}
		}

		// new canvas
		if (input == KEY_F(2)) {
			erase();
			refresh();
			c_win = new_canvas(size_x, size_y, &canvas_height, &canvas_width, canvas);
		}

		// save to file
		if (input == KEY_F(3)) {
			export_to_file(canvas, size_x, size_y, canvas_height, canvas_width);
		}

		// ui visibility toggle
		if (input == KEY_F(4)) {
			if (show_ui == 1) 
				show_ui = 0;
			else show_ui = 1;
		}

		// character switchers
		if (input == ']' && current_char < 93) {
			current_char++; 
		}
		if (input == '[' && current_char > 0) {
			current_char--; 
		}

		// brush tool	
		if (input == 'b') {
			current_tool = 0;
			drawing_line = 0;
		}
			
		// line tool
		if (input == 'l') {
			current_tool = 1;
		}

		// eraser tool
		if (input == 'e') {
			current_tool = 2;
			drawing_line = 0;
		}
		
		// move tool
		if (input == 'm') {
			current_tool = 3;
			drawing_line = 0;
			for (int i = 0; i <= canvas_height+1; i++)
				for (int j = 0; j <= canvas_width+1; j++)
						temp_canvas[i+canvas_height+2][j+canvas_width+2] = canvas[i][j];
		}

        // updates
        update_menu(size_x, size_y, canvas_height, canvas_width, ascii_list, current_char, current_tool, new_line, help_mode, show_ui);
        if (drawing_line == 1) update_canvas(c_win, temp_canvas, 0, 0, &canvas_height, &canvas_width, pos, prev_pos);
        else if (current_tool == 3) update_canvas(c_win, temp_canvas, canvas_height+2, canvas_width+2, &canvas_height, &canvas_width, pos, prev_pos);
        else update_canvas(c_win, canvas, 0, 0, &canvas_height, &canvas_width, pos, prev_pos);
    }
    
    endwin();

	return 0;
}
