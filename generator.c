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