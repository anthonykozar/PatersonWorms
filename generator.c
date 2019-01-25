/*This program should have the same svg output as worms.js. The purpose of this file is to quickly generate the paths.
	The focus of this is not visual, like the js file, so the path will not be "drawn", as it is calculated.
	Instead, the eaten paths will be recorded, and at the end, an svg will be created from this.
	How exactly the last 50 lines will be shown using this method is to be determined, but is not necessary for correctness.
*/

/*Possible optimizations:
	Use 6 bits for each point (requires allocation of 1 byte, could use a char)
*/
#include <math.h>

int DEBUG = 0;
int DIR_MATRIX[6][2] = {{1, 0}, {1, -1}, {0, -1}, {-1, 0}, {-1, 1}, {0, 1}};
int field_array[7] = {1, 2, 2, 1, 0, 2, 0};
int choice1_f[2] = {3, 4};
int choice2_f[4] = {0,1,2,3};
int choice3t_f[3] = {0,1,2};
int choice3b1_f[3] = {2,0,1};
int choice3b2_f[3] = {1,0,2};
int choice3b3_f[3] = {1,0,2};
int choice3b4_f[3] = {0, 1, 2};
int choice4_f[2] = {0, 1};

int retval[3] = {0, 0, 0};


double snap_center_x;
double snap_center_y;
double line_length;

typedef struct {
	char edges;
} point;

int move_to(point** map, int c_x, int c_y, int x, int y, int to_dir, int step) {
  if(DEBUG) {
    printf("moveTo(%d, %d, %d, %d, %d)", c_x, c_y, x, y, to_dir);
  };
  
  map[x][y].edges |= 1 << ((to_dir + 3) % 6);
  map[c_x][c_y].edges |= 1 << (to_dir % 6);
  
  double x1 = snap_center_x;
  double x2 = snap_center_x - line_length*cos(((to_dir + 3) % 6)*M_PI/3);
  double y1 = snap_center_y;
  double y2 = snap_center_y - line_length*sin(((to_dir + 3) % 6)*M_PI/3);

  snap_center_x = x2;
  snap_center_y = y2;

  return 0;
}
int* determine_move(point** map, int c_x, int c_y, int c_dir, int step) {
  if(DEBUG) {
    printf("determineMove(%d, %d, %d, %d", c_x, c_y, c_dir, step);
  }
  char c_edges = map[c_x][c_y].edges;
  if(DEBUG) {
    printf("c_edges:\t%c", map[c_x][c_y].edges);
  }
  //check in order of preference
  int x = c_x;
  int y = c_y;
  int new_dir = (c_dir + 3) % 6;
  int eaten = 0;
  int i;
  for(i = 0; i < 6; i++){eaten += (c_edges & (1 << i)) >> i;}

  int choice = 1;
  //Determine the correct field of choice
  if(eaten == 1)
    choice += choice1_f[field_array[0]];
  else if(eaten == 2)
    choice += choice2_f[field_array[1]];
  else if(eaten == 3) {
    //check orientation of the true (eaten) paths
    if(c_edges & (1 << ((new_dir + 1) % 6)) && c_edges & (1 << ((new_dir + 2) % 6)))
    {
      choice += choice3t_f[field_array[2]];
      if(DEBUG){
        printf("choice3t_f[field_array[2]]: %d", choice3t_f[field_array[2]]);
      }
    }
    else if(c_edges & (1 << ((new_dir + 1) % 6)) && c_edges & (1 << ((new_dir + 3) % 6)))
    {
      choice += choice3b1_f[field_array[2]];
      if(DEBUG){
        printf("choice3b1_f[field_array[2]]: %d", choice3b1_f[field_array[2]]);
      }
    }
    else if(c_edges & (1 << ((new_dir + 3) % 6)) && c_edges & (1 << ((new_dir + 2) % 6)))
    {
      choice += choice3t_f[field_array[3]];
      if(DEBUG){
        printf("choice3t_f[field_array[3]]: %d", choice3t_f[field_array[3]]);
      }
    }
    else if(c_edges & (1 << ((new_dir + 5) % 6)) && c_edges & (1 << ((new_dir + 1) % 6)))
    {
      choice += choice3b2_f[field_array[3]];
      if(DEBUG){
        printf("choice3b2_f[field_array[3]]: %d", choice3b2_f[field_array[3]]);
      }
    }
    else if(c_edges & (1 << ((new_dir + 4) % 6)) && c_edges & (1 << ((new_dir + 3) % 6)))
    {
      choice += choice3t_f[field_array[4]];
      if(DEBUG){
        printf("choice3t_f[field_array[4]]: %d", choice3t_f[field_array[4]]);
      }
    }
    else if(c_edges & (1 << ((new_dir + 4) % 6)) && c_edges & (1 << ((new_dir + 2) % 6)))
    {
      choice += choice3b3_f[field_array[4]];
      if(DEBUG){
        printf("choice3b3_f[field_array[4]]: %d", choice3b3_f[field_array[4]]);
      }
    }
    else if(c_edges & (1 << ((new_dir + 5) % 6)) && c_edges & (1 << ((new_dir + 4) % 6)))
    {
      choice += choice3t_f[field_array[5]];
      if(DEBUG){
        printf("choice3t_f[field_array[5]]: %d", choice3t_f[field_array[5]]);
      }
    }
    else if(c_edges & (1 << ((new_dir + 5) % 6)) && c_edges & (1 << ((new_dir + 3) % 6)))
    {
      choice += choice3b4_f[field_array[5]];
      if(DEBUG){
        printf("choice3b4_f[field_array[5]]: %d", choice3b4_f[field_array[5]]);
      }
    }
    else {
      choice = 0;
      printf("UNEXPECTED CHOICE");
    }
  }
  else if(eaten == 4)
    choice += choice4_f[field_array[6]];
  
  
  int count = 0;
  while(choice != 0 && count != 5) {
    count += 1;
    choice -= !((c_edges & (1 << count)) >> count);
  }
  if(choice == 0) {
    new_dir = (new_dir+count) % 6;
    x += DIR_MATRIX[new_dir][0];
    y += DIR_MATRIX[new_dir][1];
    
    int zooming = moveTo(map, c_x, c_y, x, y, new_dir, step);
    retval = {x, y, new_dir};
    return retval;
  }
  return 0;
}

void next_step() {

}

point** init_graph(int size) {
	point** map = (point **) calloc(1, sizeof(point *)*size);
	int i,j;
	for(i = 0; i < size; i++)
		map[i] = (point *) calloc(1, sizeof(point)*size);
	return map;
}

int main() {
	int size = 1000;
	point** map = init_graph(size);
	int i,j;
	// for(i = 0; i < size; i++) {
	// 	for(j = 0; j < size; j++)
	// 		printf("%c ", map{i}{j}.edges);
	// 	printf("\n");
	// }
	printf("Total size: %d bytes\n", 8+size*8+size*size);


}