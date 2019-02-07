/*This program should have the same svg output as worms.js. The purpose of this file is to quickly generate the paths.
	The focus of this is not visual, like the js file, so the path will not be "drawn", as it is calculated.
	Instead, the eaten paths will be recorded, and at the end, an svg will be created from this.
	How exactly the last 50 lines will be shown using this method is to be determined, but is not necessary for correctness.
*/

/*Possible optimizations:
	Use 6 bits for each point (requires allocation of 1 byte, could use a char)
*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

int DEBUG = 0;
int DIR_MATRIX[6][2] = {{1, 0}, {1, -1}, {0, -1}, {-1, 0}, {-1, 1}, {0, 1}};
int translated_field_array[11];

int retval[3] = {0, 0, 0};
int size = 10;

int start;
int min_x;
int min_y;
int max_x;
int max_y;


/* Point structs are created in case more data ever wants to be added to a point.
 * This could be useful for keeping track of coloring, numbering, etc.
 */
typedef struct {
	char edges;
} point;

/* Translates the initial input governing the worm's path decisions from Gardner's notation
 * to an array covering all possibilities. This is done separately in order to avoid clutter 
 * in other functions.
 */
void translate_field_array(int* field_array) {
  int choice1_f[2] = {3, 4};
  int choice2_f[4] = {0,1,2,3};
  int choice3t_f[3] = {0,1,2};
  int choice3b1_f[3] = {2,0,1};
  int choice3b2_f[3] = {1,0,2};
  int choice3b3_f[3] = {1,0,2};
  int choice3b4_f[3] = {0, 1, 2};
  int choice4_f[2] = {0, 1};

  int temp_arr[11] = {choice1_f[field_array[0]], choice2_f[field_array[1]], choice3t_f[field_array[2]], 
  choice3b1_f[field_array[2]], choice3t_f[field_array[3]], choice3b2_f[field_array[3]], choice3t_f[field_array[4]],
  choice3b3_f[field_array[4]], choice3t_f[field_array[5]], choice3b4_f[field_array[5]], choice4_f[field_array[6]]};
  
  for(int i = 0; i < 11; i++) {
    translated_field_array[i] = temp_arr[i];
  }
}

/* Updates minimum and maximum x and y reached by the worm.
 * These maxes and mins are used when determining the size of the svg canvas.
 */
void update_minmax(int x, int y) {
  if(y > max_y)
    max_y = y;
  if(y < min_y)
    min_y = y;    
  if(x > max_x)
    max_x = x;
  if(x < min_x)
    min_x = x;
}

/* Moves the worm from position (c_x, c_y) to (x, y) along direction dir
 * if (x, y) is within the bounds of the array. The edge being traveled down from
 * (c_x, c_y) is set to 1 and the opposite edge connected to (x, y) is set to 1 (eaten).
 * This is unfortunate redundancy for memory but makes computation easier.
 *
 * Returns true if the worm is moved successfully and false if the worm cannot be moved to (x, y).
 */
bool move_to(point** map, int c_x, int c_y, int x, int y, int to_dir) {
  if(DEBUG) {
    printf("move_to(%d, %d, %d, %d, %d)\n", c_x, c_y, x, y, to_dir);
  }

  if(x >= 0 && x < size && y >= 0 && y < size) {
	  map[x][y].edges |= 1 << ((to_dir + 3) % 6);
	  map[c_x][c_y].edges |= 1 << (to_dir % 6);
    update_minmax(x, y);
	  return true;
  }

  if(DEBUG) {
  	printf("The worm attempted to travel out of bounds and was killed.\n");
  }
  return false;
}

/* Checks to see if the direction dir has already been traveled down from
*  the point that c_edges belongs to.
*/
bool path_eaten(char edges, int dir) {
  return edges & (1 << (dir % 6));
}

/* Returns the number of paths that have already been traveled down from
 * the point that c_edges belongs to.
 */
int get_number_eaten_paths(char edges) {
  int i;
  int eaten = 0;
  for(i = 0; i < 6; i++) {
    eaten += (edges & (1 << i)) && 1;
  }
  return eaten;
}

/* Determines how many uneaten paths need to be passed before selecting the one the worm will travel down.
 * Why is this method so ugly? Well, Gardner's notation is unfortunately difficult to generalize
 * and requires specific cases. The field_array is an array containing the actual values of the
 * radio buttons, but these need to be converted into the number of paths to skip.
 * tl;dr I didn't make the notation but I need to maintain consistency to check correctness.
 *
 * There's probably some way to reduce the repetition of the code using more arrays and indexing,
 * but I would rather keep it readable than short.
 */
int get_number_paths_to_pass(char edges, int dir, int eaten) {
  int choice = 1;
  if(eaten == 1)
    choice += translated_field_array[0];
  else if(eaten == 2)
    choice += translated_field_array[1];
  else if(eaten == 3) {
    //check orientation of the true (eaten) paths
    if(path_eaten(edges, dir + 1) && path_eaten(edges, dir + 2))
      choice += translated_field_array[2];
    else if(path_eaten(edges, dir + 1) && path_eaten(edges, dir + 3))
      choice += translated_field_array[3];
    else if(path_eaten(edges, dir + 3) && path_eaten(edges, dir + 2))
      choice += translated_field_array[4];
    else if(path_eaten(edges, dir + 5) && path_eaten(edges, dir + 1))    
      choice += translated_field_array[5];
    else if(path_eaten(edges, dir + 4) && path_eaten(edges, dir + 3))
      choice += translated_field_array[6];
    else if(path_eaten(edges, dir + 4) && path_eaten(edges, dir + 2))
      choice += translated_field_array[7];
    else if(path_eaten(edges, dir + 5) && path_eaten(edges, dir + 4))
      choice += translated_field_array[8];
    else if(path_eaten(edges, dir + 1) && path_eaten(edges, dir + 3))
      choice += translated_field_array[9];
    else {
      choice = 0;
      printf("UNEXPECTED CHOICE");
    }
  }
  else if(eaten == 4)
    choice += translated_field_array[10];
  return choice;
}


/*
 *
 *
 */
bool determine_move(point** map, int c_x, int c_y, int c_dir, int step) {
  if(DEBUG) {
    printf("determineMove(%d, %d, %d, %d)\n", c_x, c_y, c_dir, step);
  }
  if(DEBUG) {
  	int w;
  	printf("c_edges: ");
  	for(w = 0; w < 6; w++)
    	printf("%d ", 0 || map[c_x][c_y].edges & (1 << w));
    printf("\n");
  }
  //check in order of preference
  char c_edges = map[c_x][c_y].edges;

  int x = c_x;
  int y = c_y;
  int new_dir = (c_dir + 3) % 6;
  int eaten = get_number_eaten_paths(c_edges);
  int choice = get_number_paths_to_pass(c_edges, new_dir, eaten);  
  
  int count = 0;
  while(choice != 0 && count != 5) {
    count += 1;
    choice -= !path_eaten(c_edges, new_dir + count);
  }
  if(choice == 0) {
    new_dir = (new_dir+count) % 6;
    x += DIR_MATRIX[new_dir][0];
    y += DIR_MATRIX[new_dir][1];

    if(move_to(map, c_x, c_y, x, y, new_dir))
    {
	    retval[0] = x;
	    retval[1] = y;
	    retval[2] = new_dir;
	    return true;
  	}
  	else
  		return false;
  }
  return false;
}

point** init_graph(int size) {
	point** map = (point **) calloc(1, sizeof(point *)*size);
	int i,j;
	for(i = 0; i < size; i++)
		map[i] = (point *) calloc(1, sizeof(point)*size);
  return map;
}

void map_to_svg(point ** map) {
	/*Iterate through map
		For each encounter, set to false (and set its corresponding edge in another node as false)
		Create a line based on this.
		Going to need to calculate lattice point coordinates based on line length
		Optimization (storage but not runtime): Truncate coordinates to two or three decimal places
			This is because svg size is dependent on the number of characters in the markup
			Not truncating leaves about 17*numLines characters
	*/

	//Let x y = 0,0 be the center. Moving along the horizontal lines translates you 
	//http://clintbellanger.net/articles/isometric_math/


	//Try timing this with it all in one giant printf, and one where it's broken up. I'm not sure how much overhead
	//there will be from calling printf again just to print the closing tag
	int i,j;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			point p = map[i][j];
			if(DEBUG) {
		  	int w;
		  	printf("c_edges: ");
		  	for(w = 0; w < 6; w++)
		    	printf("%d ", 0 || map[i][j].edges & (1 << w));
		    printf("\n");
		  }
			if(p.edges == 0)
				continue;
			char c = p.edges;
			int t;


// (-5, -5) (-1,1) 					(5, -5) (0,1)
//                \        /
//(-10, 0) (-1,0) --(0,0)-- (10, 0) (1,0)
//        				/        \
// (-5, 5) (0,-1)           (5, 5) (1,-1)
			for(t = 0; t < 6; t++) {
				//if there is an edge in this direction
				if(c & (1 << t)) {
					//we need to:
					//draw the line
					//set this bit to 0 in map and set its partner's bit to 0 as well.
					//might want to do a bounds check here too.
					double p_y, p_ny;
					int p_x, p_nx;

					//Subtracting by start centers the image (assuming starting x and y are the same)
					int adj_i = i - start;
					int adj_j = j - start;
					int n_x = adj_i+DIR_MATRIX[t][0];
					int n_y = adj_j+DIR_MATRIX[t][1];

					p_y = -5*sqrt(3)*adj_j;
					p_ny = -5*sqrt(3)*n_y;
					p_x = 10*adj_i + 5*adj_j;
					p_nx = 10*n_x + 5*n_y;
					map[i][j].edges &= ~(1 << t);
					map[i+DIR_MATRIX[t][0]][j+DIR_MATRIX[t][1]].edges &= ~(1 << ((t+3) % 6));

					//This should probably be changed to make it output a path as it goes on in order to reduce the amount of memory
					printf("<line x1=\"%d\" x2=\"%d\" y1=\"%.3f\" y2=\"%.3f\" stroke=\"#000000\" style=\"stroke-width: 2; stroke-linecap: round;\">", p_x, p_nx, p_y, p_ny);
					printf("</line>");//<!--(%d, %d) to (%d, %d)-->\n", i, j, i+DIR_MATRIX[t][0], j+DIR_MATRIX[t][1]);
				}
			}
		}
	}
}

void create_svg(point ** map) {
	printf("<!--(%d, %d, %d, %d)-->\n", min_x, max_x, min_y, max_y);
	int min_px = 10*(min_x-start) - 5*(max_y-start);
	int max_px = 10*(max_x-start) + 5*(max_y-start);
	int min_py = -5*sqrt(3)*(max_y - start);
	int max_py = -5*sqrt(3)*(min_y - start);
	printf("<svg height=\"100%%\" version=\"1.1\" width=\"100%%\" xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"%d %d %d %d\" onresize=\"fixBounds()\">\n<desc></desc>\n<defs></defs>\n", min_px, min_py, max_px-min_px, max_py-min_py);
	printf("\t<g>\n");
	map_to_svg(map);
	printf("\t</g>\n");
	printf("</svg>\n");
}

int main() {
	point** map = init_graph(size);
	int i,j;
  int field_array[7] = {1, 2, 2, 1, 0, 2, 0};

  translate_field_array(field_array);
	//We want to start in the middle of the array. This is not always optimal, but it is simple to implement.
	start = size/2;
	min_x = start;
	min_y = start;
	max_x = start;
	max_y = start;
	//printf("Total size: %d bytes\n", 8+size*8+size*size);
  move_to(map, start, start, start+1, start, 0);
  // Start "moving" the worm
	int step = 1;
  retval[0] = start+1;
  retval[1] = start;
  retval[2] = 0;
  while(1){
    step += 1;
    bool success = determine_move(map, retval[0], retval[1], retval[2], step);
    if(!success)
      break;
  }
	create_svg(map);
}