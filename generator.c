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
#include <float.h>
#include <string.h>
#include <argp.h>
#include <limits.h>

int DEBUG = 0;
int DIR_MATRIX[6][2] = {{1, 0}, {1, -1}, {0, -1}, {-1, 0}, {-1, 1}, {0, 1}};
int translated_field_array[11];

//int size = 100;

int start;


/* Variables for argp usage */

const char *argp_program_version =
  "pwormgen 1.0";
const char *argp_program_bug_address =
  "<andmcadams@gmail.com>";

  /* Program documentation. */
static char doc[] =
  "Paterson's Worm Generator -- a program that creates images of Paterson's worms' paths";

  /* A description of the arguments we accept. */
static char args_doc[] = "";

  /* The options we understand. */
static struct argp_option options[] = {
  {"rule", 'r', "RULE", 0, "Create a worm following rule RULE"},
  {"size", 's', "INT", 0, "Run worm on a map of size INT"},
  {"output", 'o', "FILE", 0, "Output to FILE instead of standard output" },
  { 0 }
};

  /* Used by main to communicate with parse_opt. */
struct arguments
{
  int rule[7];
  int size;
  char *output_file;
};

/* Print out the constraints required for rules.
 * This is called when the user inputs an invalid rule.
 */
void print_rule_restrictions() {
  printf("Rules must be strings of seven integers.\n");
  printf("The first integer must be one of {0, 1}.\n");
  printf("The second integer must be one of {0, 1, 2, 3}.\n");
  printf("The third through sixth integers must be one of {0, 1, 2}.\n");
  printf("The seventh integer must be one of {0, 1}.\n");
}

/* Validate rule input */
int validate_rule(struct arguments *arguments, char* rule) {
  if(strlen(rule) != 7)
    return 0;
  int max_val[7] = {1, 3, 2, 2, 2, 2, 1};
  int i;
  int flag = 0;
  for(i = 0 ; i < 7; i++) {
    if((int) rule[i]-'0' > max_val[i] || (int) rule[i]-'0' < 0) {
      printf("Problem with integer in pos %d:\t%d\t(max %d)\n", i, (int) rule[i]-'0', max_val[i]);
      flag = 1;     
    }
    else
      arguments->rule[i] = (int) rule[i]-'0';
  }
  if(flag)
    return 0;
  return 1;
}

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;
  switch (key)
    {
    case 'r':
      if(!validate_rule(arguments, arg)) {
        printf("Invalid rule passed: %s\n", arg);
        print_rule_restrictions();
        exit(1);
      }
      break;
    case 's': {     
      long l = strtol(arg, NULL, 10);
      if(errno != 0 && l == 0) {
        printf("strtol encountered an error.\nExiting...\n");
        exit(1);
      }
      if(l > INT_MAX || l < 1) {
        printf("Value is not a positive integer: %ld\nExiting...", l);
        exit(1);
      }
      arguments->size = (int) l;
      // Should probably add a check to make sure they aren't trying to make an absurdly large array that will consume all of the computer's memory
      break;
    }
    case 'o':
      arguments->output_file = arg;
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1)
        /* Too many arguments. */
        argp_usage (state);
      break;
    case ARGP_KEY_END:
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

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

/* Moves the worm from position (c_x, c_y) to (x, y) along direction dir
 * if (x, y) is within the bounds of the array. The edge being traveled down from
 * (c_x, c_y) is set to 1 and the opposite edge connected to (x, y) is set to 1 (eaten).
 * This is unfortunate redundancy for memory but makes computation easier.
 *
 * Returns true if the worm is moved successfully and false if the worm cannot be moved to (x, y).
 */
bool move_to(point** map, int size, int c_x, int c_y, int x, int y, int to_dir) {
  if(DEBUG) {
    printf("move_to(%d, %d, %d, %d, %d)\n", c_x, c_y, x, y, to_dir);
  }

  if(x >= 0 && x < size && y >= 0 && y < size) {
	  map[x][y].edges |= 1 << ((to_dir + 3) % 6);
	  map[c_x][c_y].edges |= 1 << (to_dir % 6);
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
    else if(path_eaten(edges, dir + 5) && path_eaten(edges, dir + 3))
      choice += translated_field_array[9];
    else {
      choice = 0;
      printf("UNEXPECTED CHOICE");
      exit(1);
    }
  }
  else if(eaten == 4)
    choice += translated_field_array[10];
  return choice;
}


/* Determines the worms next move, and if possible, executes it.
 * This is done by first finding how many paths have been eaten, and then finding the number of
 * paths to pass based on the result and the initial input for the worm's decisions.
 * Returns true if the worm was successfully moved. The array retval is also updated to reflect
 * the worms new x coord, y coord, and direction. Otherwise returns false.
 */
bool determine_move(point** map, int size, int* retval, int step) {
  if(DEBUG) {
    printf("determineMove(map, [%d, %d, %d], %d)\n", retval[0], retval[1], retval[2], step);
    printf("c_edges: ");
    for(int w = 0; w < 6; w++)
      printf("%d ", 0 || map[retval[0]][retval[1]].edges & (1 << w));
    printf("\n");
  }

  int c_x = retval[0];
  int c_y = retval[1];
  int c_dir = retval[2];
  char c_edges = map[c_x][c_y].edges;

  int new_dir = (c_dir + 3) % 6;
  int eaten = get_number_eaten_paths(c_edges);
  int choice = get_number_paths_to_pass(c_edges, new_dir, eaten);  
  
  int count = 0;
  while(choice != 0 && count != 5) {
    count += 1;
    choice -= !path_eaten(c_edges, new_dir + count);
  }

  //If choice is not 0, we have cycled through all the paths but could not find one to go down.
  if(choice == 0) {
    new_dir = (new_dir+count) % 6;
    int x = c_x + DIR_MATRIX[new_dir][0];
    int y = c_y + DIR_MATRIX[new_dir][1];

    if(move_to(map, size, c_x, c_y, x, y, new_dir))
    {
	    retval[0] = x;
	    retval[1] = y;
	    retval[2] = new_dir;

	    return true;
  	}
  	else
  		return false;
  }
  else
    return false;
}


/* Creates an empty map for the worm to move around.
 * The use of calloc is due to the fact that we want all the paths to initially be zero.
 * We need to allocate the point** to include enough space for size point pointers.
 * We need to allocate the point* to include enough space for size points.
 * The point pointer pointers correspond to x coordinates. The point pointers correspond to y coordinates.
 * Returns a 2d array of points all initialized to have their edges set to 0.
 */
point** init_graph(int size) {
	point** map = (point **) calloc(1, sizeof(point *)*size);
	int i,j;
	for(i = 0; i < size; i++)
		map[i] = (point *) calloc(1, sizeof(point)*size);
  return map;
}

/* Free the map when it is no longer needed.
 */
void free_map(point ** map, int size) {
  int i;
  for(i = 0; i < size; i++) {
    free(map[i]);
  }
  free(map);
}

/* Prints out the line tags for the svg to standard out.
 * Iterates through the array and for each vertex, goes through the six edges. If an edge is marked as crossed
 * then we calculate an adjusted i and j (shifted over by our starting value since we started at (start, start)).
 * Then we determine the x and y values for each end of the line. Line length is fixed at 10.
 * The edge in the vertex at the end of the line is then set to 0 so it is not recalculated.
 */
// This should probably be changed to output a path as it goes on in order to cut down on line tags
void map_to_svg(point ** map, int size) {
	int i,j;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			point p = map[i][j];
			if(p.edges == 0)
				continue;
			char c = p.edges;
			int t;

			for(t = 0; t < 6; t++) {
				if(c & (1 << t)) {
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

					printf("<line x1=\"%d\" x2=\"%d\" y1=\"%.3f\" y2=\"%.3f\" stroke=\"#000000\" style=\"stroke-width: 2; stroke-linecap: round;\"></line>", p_x, p_nx, p_y, p_ny);
				}
			}
		}
	}
}

/* Finds the minimum and maximum x and y pixel values.
 * This will iterate through the entire map, but losing a bit of performance to reduce storage
 * is preferable in this situation since I have limited storage.
 */
float* find_min_max(point ** map, int size, int line_length) {
  float* min_max = malloc(4*sizeof(float));
  float tmp_arr[4] = {FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX};
  memcpy(min_max, tmp_arr, 4*sizeof(float));

  int i,j, adj_i, adj_j;
  float p_x, p_y;
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      if(map[i][j].edges) {
        adj_j = j - start;
        adj_i = i -start;
        p_y = -5*sqrt(3)*adj_j;
        p_x = 10*adj_i + 5*adj_j;
        if(min_max[0] > p_x)
          min_max[0] = p_x;
        if(min_max[1] < p_x)
          min_max[1] = p_x;
        if(min_max[2] > p_y)
          min_max[2] = p_y;
        if(min_max[3] < p_y)
          min_max[3] = p_y; 
      }
    }
  }
  return min_max;
}

/* Creates an svg of the path taken by the worm.
 * The size of the svg is determined by calculating the highest and lowest possible
 * values of x and y coordinates. 
 */
void create_svg(point ** map, int size) {
  float* min_max = find_min_max(map, size, 10);
	printf("<svg height=\"100%%\" version=\"1.1\" width=\"100%%\" xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"%.3f %.3f %.3f %.3f\" onresize=\"fixBounds()\">\n<desc></desc>\n<defs></defs>\n", min_max[0], min_max[2], min_max[1]-min_max[0], min_max[3]-min_max[2]);
	free(min_max);

  printf("<g>\n");
	map_to_svg(map, size);
	printf("</g>\n");
	printf("</svg>\n");
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char **argv) {
	int i,j;
  //int field_array[7] = {0, 1, 2, 1, 0, 2, 0};
  struct arguments arguments;

  /* Default values. */
  for(int i = 0; i < 7; i++)
    arguments.rule[i] = 0;
  arguments.size = 100;
  arguments.output_file = "-";
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  point** map = init_graph(arguments.size);

  translate_field_array(arguments.rule);
	//We want to start in the middle of the array. This is not always optimal, but it is simple to implement.
	start = arguments.size/2;

  //Should probably check this before continuing.
  //Hardcode in this step since this is an arbitrary movement to the right
  move_to(map, arguments.size, start, start, start+1, start, 0);
  // Start "moving" the worm
	int step = 1;
  int retval[3] = {start+1, start, 0};

  while(1){
    step += 1;
    bool success = determine_move(map, arguments.size, retval, step);
    if(!success)
      break;
  }
	create_svg(map, arguments.size);
  free_map(map, arguments.size);
}