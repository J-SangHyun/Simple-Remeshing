#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_FILE_NAME 100
#define MAX_LINE 100
#define L 15

struct Vec3 {
    float x;
    float y;
    float z;
};

float get_distance(struct Vec3 *vec1, struct Vec3 *vec2) {
    float distance = pow(vec1->x - vec2->x, 2) + pow(vec1->y - vec2->y, 2) + pow(vec1->z - vec2->z, 2);
    distance = sqrt(distance);
    return distance;
}

int main() {
    // char filename[MAX_FILE_NAME] = "objects/knot.obj", line[MAX_LINE];
    char line[MAX_LINE];
    char *filename = "./objects/knot.obj";
    int vertex_number = 0, face_number = 0;
    FILE *stream = fopen(filename, "rb");
    int i, j;
    float l_max = L * 3.0f / 4, l_min = L * 4.0f / 5;
    
    // Read OBJ file - Count vertices and faces
    printf("Reading OBJ file...\n");
    while(!feof(stream)) {
		fgets(line, MAX_LINE, stream);
		if(line[0] == 'v' && line[1] == ' ') {
			vertex_number++;
		}
		else if(line[0] == 'f' && line[1] == ' ') {
			face_number++;
		}
		line[0] = '\0';
    }
    fseek(stream, 0, SEEK_SET);
    printf("VERTICES: %d, FACES: %d\n", vertex_number, face_number);
    
    // Read OBJ file - Vertex connectivity
    struct Vec3 *vertex = (struct Vec3 *)malloc(sizeof(struct Vec3) * vertex_number);
    char **vertex_neighbors = (char **)malloc(sizeof(char*) * vertex_number);
    for(i = 0; i < vertex_number; i++) {
        vertex_neighbors[i] = (char *)malloc(sizeof(char) * vertex_number);
    }
    for(i = 0; i < vertex_number; i++) {
        for(j = 0; j < vertex_number; j++) {
            vertex_neighbors[i][j] = 0;
        }
    }
    int **face = (int **)malloc(sizeof(int *) * face_number);
    for(i = 0; i < face_number; i++) {
        face[i] = (int *)malloc(sizeof(int) * 3);
    }
    
    int vertex_idx = 0, face_idx = 0;
    int v1, v2, v3;
    char* cursor;
    while(!feof(stream)) {
		fgets(line, MAX_LINE, stream);
		if(line[0] == 'v' && line[1] == ' ') {
		    cursor = strtok(line+2, " ");
		    vertex[vertex_idx].x = atof(cursor); cursor = strtok(NULL, " ");
		    vertex[vertex_idx].y = atof(cursor); cursor = strtok(NULL, " ");
		    vertex[vertex_idx].z = atof(cursor);
            vertex_idx++;
        }
		else if(line[0] == 'f' && line[1] == ' ') {
		    cursor = strtok(line+2, " ");
			v1 = atoi(cursor) - 1; cursor = strtok(NULL, " ");
		    v2 = atoi(cursor) - 1; cursor = strtok(NULL, " ");
		    v3 = atoi(cursor) - 1;
		    
		    if(!vertex_neighbors[v1][v2]) {
		        vertex_neighbors[v1][v2] = 1;
		        vertex_neighbors[v2][v1] = 1;
            }
            if(!vertex_neighbors[v1][v3]) {
		        vertex_neighbors[v1][v3] = 1;
		        vertex_neighbors[v3][v1] = 1;
            }
            if(!vertex_neighbors[v2][v3]) {
		        vertex_neighbors[v2][v3] = 1;
		        vertex_neighbors[v3][v2] = 1;
            }
            face[face_idx][0] = v1;
            face[face_idx][1] = v2;
            face[face_idx][2] = v3;
			face_idx++;
		}
		line[0] = '\0';
    }
    fclose(stream);
    
    // Edge Split
    printf("Edge Spliting...\n");
    float distance;
    int new_vertex_number = vertex_number;
    int new_face_number = face_number;
    int f;
    for(v1 = 0; v1 < vertex_number; v1++) {
        for(v2 = v1+1; v2 < vertex_number; v2++) {
            if(vertex_neighbors[v1][v2]) {
                distance = get_distance(&vertex[v1], &vertex[v2]);
                if(distance > l_max) {
                    // need to split
                    new_vertex_number++;
                    vertex = (struct Vec3 *)realloc(vertex, sizeof(struct Vec3) * new_vertex_number);
                    vertex_neighbors = (char **)realloc(vertex_neighbors, sizeof(char*) * new_vertex_number);
                    for(i = 0; i < new_vertex_number-1; i++) {
                        vertex_neighbors[i] = (char *)realloc(vertex_neighbors[i], sizeof(char) * new_vertex_number);
                        vertex_neighbors[i][new_vertex_number-1] = 0;
                    }
                    vertex_neighbors[new_vertex_number-1] = (char *)malloc(sizeof(char) * new_vertex_number);
                    for(i = 0; i < new_vertex_number; i++) {
                        vertex_neighbors[new_vertex_number-1][i] = 0;
                    }
                    
                    vertex[new_vertex_number-1].x = (vertex[v1].x + vertex[v2].x) / 2;
                    vertex[new_vertex_number-1].y = (vertex[v1].y + vertex[v2].y) / 2;
                    vertex[new_vertex_number-1].z = (vertex[v1].z + vertex[v2].z) / 2;
                    
                    // connect v1 and new vertex
                    vertex_neighbors[new_vertex_number-1][v1] = 1;
                    vertex_neighbors[v1][new_vertex_number-1] = 1;
                    // connect v2 and new vertex
                    vertex_neighbors[new_vertex_number-1][v2] = 1;
                    vertex_neighbors[v2][new_vertex_number-1] = 1;
                    
                    for(f = 0; f < face_number; f++) {
                        if(face[f][0] == v1 && face[f][1] == v2 && face[f][2] < vertex_number) {
                            new_face_number++;
                            v3 = face[f][2];
                            vertex_neighbors[new_vertex_number-1][v3] = 1;
                            vertex_neighbors[v3][new_vertex_number-1] = 1;
                            face[f][1] = new_vertex_number-1;
                            face = (int **)realloc(face, sizeof(int*) * new_face_number);
                            face[new_face_number-1] = (int *)malloc(sizeof(int) * 3);
                            face[new_face_number-1][0] = new_vertex_number-1;
                            face[new_face_number-1][1] = v2;
                            face[new_face_number-1][2] = v3;
                        }
                        else if(face[f][0] == v1 && face[f][2] == v2 && face[f][1] < vertex_number) {
                            new_face_number++;
                            v3 = face[f][1];
                            vertex_neighbors[new_vertex_number-1][v3] = 1;
                            vertex_neighbors[v3][new_vertex_number-1] = 1;
                            face[f][2] = new_vertex_number-1;
                            face = (int **)realloc(face, sizeof(int*) * new_face_number);
                            face[new_face_number-1] = (int *)malloc(sizeof(int) * 3);
                            face[new_face_number-1][0] = new_vertex_number-1;
                            face[new_face_number-1][1] = v3;
                            face[new_face_number-1][2] = v2;
                        }
                        else if(face[f][1] == v1 && face[f][0] == v2 && face[f][2] < vertex_number) {
                            new_face_number++;
                            v3 = face[f][2];
                            vertex_neighbors[new_vertex_number-1][v3] = 1;
                            vertex_neighbors[v3][new_vertex_number-1] = 1;
                            face[f][1] = new_vertex_number-1;
                            face = (int **)realloc(face, sizeof(int*) * new_face_number);
                            face[new_face_number-1] = (int *)malloc(sizeof(int) * 3);
                            face[new_face_number-1][0] = new_vertex_number-1;
                            face[new_face_number-1][1] = v1;
                            face[new_face_number-1][2] = v3;
                        }
                        else if(face[f][1] == v1 && face[f][2] == v2 && face[f][0] < vertex_number) {
                            new_face_number++;
                            v3 = face[f][0];
                            vertex_neighbors[new_vertex_number-1][v3] = 1;
                            vertex_neighbors[v3][new_vertex_number-1] = 1;
                            face[f][2] = new_vertex_number-1;
                            face = (int **)realloc(face, sizeof(int*) * new_face_number);
                            face[new_face_number-1] = (int *)malloc(sizeof(int) * 3);
                            face[new_face_number-1][0] = new_vertex_number-1;
                            face[new_face_number-1][1] = v2;
                            face[new_face_number-1][2] = v3;
                        }
                        else if(face[f][2] == v1 && face[f][0] == v2 && face[f][1] < vertex_number) {
                            new_face_number++;
                            v3 = face[f][1];
                            vertex_neighbors[new_vertex_number-1][v3] = 1;
                            vertex_neighbors[v3][new_vertex_number-1] = 1;
                            face[f][2] = new_vertex_number-1;
                            face = (int **)realloc(face, sizeof(int*) * new_face_number);
                            face[new_face_number-1] = (int *)malloc(sizeof(int) * 3);
                            face[new_face_number-1][0] = new_vertex_number-1;
                            face[new_face_number-1][1] = v3;
                            face[new_face_number-1][2] = v1;
                        }
                        else if(face[f][2] == v1 && face[f][1] == v2 && face[f][0] < vertex_number) {
                            new_face_number++;
                            v3 = face[f][0];
                            vertex_neighbors[new_vertex_number-1][v3] = 1;
                            vertex_neighbors[v3][new_vertex_number-1] = 1;
                            face[f][2] = new_vertex_number-1;
                            face = (int **)realloc(face, sizeof(int*) * new_face_number);
                            face[new_face_number-1] = (int *)malloc(sizeof(int) * 3);
                            face[new_face_number-1][0] = new_vertex_number-1;
                            face[new_face_number-1][1] = v1;
                            face[new_face_number-1][2] = v3;
                        }
                    }
                    
                    // disconnect v1 and v2
                    vertex_neighbors[v1][v2] = 0;
		            vertex_neighbors[v2][v1] = 0;
                }
            }
        }
    }
    vertex_number = new_vertex_number;
    face_number = new_face_number;
    
    // Edge Collapse
    
    // Edge Flip
    
    // Vertex Shift
    
    // Save remeshed object as OBJ file
    printf("Saving remeshed OBJ file...\n");
    stream = fopen("./objects/remeshed_knot.obj", "wt");
    for(i = 0; i < vertex_number; i++) {
        fprintf(stream, "v %f %f %f\n", vertex[i].x, vertex[i].y, vertex[i].z);
    }
    for(f = 0; f < face_number; f++) {
        fprintf(stream, "f %d %d %d\n", face[f][0]+1, face[f][1]+1, face[f][2]+1);
    }
    fclose(stream);
    
    return 0;
}
