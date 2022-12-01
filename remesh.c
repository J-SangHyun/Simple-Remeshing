#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_NAME 100
#define MAX_LINE 100

int main() {
    // char filename[MAX_FILE_NAME] = "objects/knot.obj", line[MAX_LINE];
    char line[MAX_LINE];
    char *filename = "./objects/knot.obj";
    int vertex_number = 0, face_number = 0;
    FILE *stream = fopen(filename, "rb");
    
    // Read OBJ file - Count vertices and faces
    while (!feof(stream)) {
		fgets(line, MAX_LINE, stream);
		if (line[0] == 'v' && line[1] ==' ')
			vertex_number++;
		else if (line[0] == 'f' && line[1] ==' ')
			face_number++;
    }
    fseek(stream, 0, SEEK_SET);
    printf("VERTICES: %d, FACES: %d\n", vertex_number, face_number);
    
    return 0;
}
