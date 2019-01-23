/*This program should have the same svg output as worms.js. The purpose of this file is to quickly generate the paths.
	The focus of this is not visual, like the js file, so the path will not be "drawn", as it is calculated.
	Instead, the eaten paths will be recorded, and at the end, an svg will be created from this.
	How exactly the last 50 lines will be shown using this method is to be determined, but is not necessary for correctness.
*/

/*Possible optimizations:
	Use 6 bits for each point (requires allocation of 1 byte, could use a char)
*/

typedef struct {
	char edges;
} point;

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
	// 		printf("%c ", map[i][j].edges);
	// 	printf("\n");
	// }
	printf("Total size: %d bytes\n", 8+size*8+size*size);


}