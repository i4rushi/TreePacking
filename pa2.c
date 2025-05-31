#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    int is_leaf;        
    int label;          
    int width;          
    int height;         
    int xcoord;
    int ycoord;
    char cutline;       
    struct Node* left;  
    struct Node* right; 
} Node;

int max(int a, int b) {
    return (a > b) ? a : b;
}

Node* create_leaf_node(int label, int width, int height) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->is_leaf = 1;
    node->label = label;
    node->width = width;
    node->height = height;
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node* create_cutline_node(char cutline) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->is_leaf = 0;
    node->cutline = cutline;
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node* build_tree(FILE* file) {
    char line[100];
    if (fgets(line, sizeof(line), file) == NULL) {
        //printf("Reached end of file in build_tree\n");
        return NULL;
    }

    line[strcspn(line, "\n")] = 0;
    //printf("Processing line: %s\n", line);

    if (line[0] == 'V' || line[0] == 'H') {
        //printf("Creating cutline node with type: %c\n", line[0]);
        Node* node = create_cutline_node(line[0]);
        node->left = build_tree(file);
        node->right = build_tree(file);
        return node;
    }
    else {
        int label, width, height;
        sscanf(line, "%d(%d,%d)", &label, &width, &height);
        //printf("Creating leaf node - Label: %d, Width: %d, Height: %d\n", label, width, height);
        return create_leaf_node(label, width, height);
    }
}

void postorder_traversal(Node* root, FILE* output, int mode) {
    if (root == NULL) return;

    //printf("Postorder traversal - Current node: %s\n", root->is_leaf ? "Leaf" : "Cutline");
    
    postorder_traversal(root->left, output, mode);
    postorder_traversal(root->right, output, mode);

    if (root->is_leaf && (mode == 1 || mode == 2)) {
        //printf("Writing leaf node - Label: %d, Width: %d, Height: %d\n", root->label, root->width, root->height);
        fprintf(output, "%d(%d,%d)\n", root->label, root->width, root->height);
    } else {
        if(mode == 1){
            //printf("Writing cutline - Type: %c\n", root->cutline);
            fprintf(output, "%c\n", root->cutline);
        }

        if(mode == 2){
            if (root->cutline == 'V') {
                root->width = root->left->width + root->right->width;
                root->height = max(root->left->height, root->right->height);
                //printf("V cutline - New dimensions - Width: %d, Height: %d\n", root->width, root->height);
            } else {
                root->width = max(root->left->width, root->right->width);
                root->height = root->left->height + root->right->height;
                //printf("H cutline - New dimensions - Width: %d, Height: %d\n", root->width, root->height);
            }

            fprintf(output, "%c(%d,%d)\n", root->cutline, root->width, root->height);
        }
    }
}

void preorder_traversal(Node* root, FILE* output) {
    if (root == NULL) return;

    //printf("Preorder traversal - Current node: %s\n", root->is_leaf ? "Leaf" : "Cutline");

    if (root->is_leaf == 0){
        if(root->cutline == 'H'){
            //printf("Processing H cutline - Parent coords: (%d,%d)\n", root->xcoord, root->ycoord);
            int tempH = 0;
            root->right->xcoord = root->left->xcoord = root->xcoord;
            root->right->ycoord = root->ycoord;
            tempH += root->right->height;
            root->left->ycoord = root->ycoord + tempH;
            //printf("H cutline - Left child coords: (%d,%d), Right child coords: (%d,%d)\n", 
            //       root->left->xcoord, root->left->ycoord, root->right->xcoord, root->right->ycoord);
        }
        else{
            //printf("Processing V cutline - Parent coords: (%d,%d)\n", root->xcoord, root->ycoord);
            int tempV = 0;
            root->right->ycoord = root->left->ycoord = root->ycoord;
            root->left->xcoord = root->xcoord;
            tempV += root->left->width;
            root->right->xcoord = root->xcoord + tempV;
            //printf("V cutline - Left child coords: (%d,%d), Right child coords: (%d,%d)\n", 
            //       root->left->xcoord, root->left->ycoord, root->right->xcoord, root->right->ycoord);
        }
    }

    preorder_traversal(root->left, output);
    preorder_traversal(root->right, output);

    if(root->is_leaf){
        //printf("Writing leaf node with coordinates - Label: %d, Coords: (%d,%d)\n", 
        //       root->label, root->xcoord, root->ycoord);
        fprintf(output, "%d((%d,%d)(%d,%d))\n", root->label, root->width, root->height, root->xcoord, root->ycoord);
    }
}

void free_tree(Node* root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

int main(int argc, char* argv[]) {
    //printf("Program started with %d arguments\n", argc);
    if (argc != 5) {
        printf("Usage: %s <in_file> <out_file1> <out_file2>\n", argv[0]);
        return 1;
    }

    //printf("Opening input file: %s\n", argv[1]);
    FILE* in_file = fopen(argv[1], "r");
    if (in_file == NULL) {
        printf("Error opening input file\n");
        return 1;
    }

    //printf("Building tree from input file\n");
    Node* root = build_tree(in_file);
    fclose(in_file);

    FILE* out_file1 = fopen(argv[2], "w");
    if (out_file1 == NULL) {
        printf("Error creating output file\n");
        free_tree(root);
        return 1;
    }

    postorder_traversal(root, out_file1, 1);
    fclose(out_file1);

    FILE* out_file2 = fopen(argv[3], "w");
    if (out_file2 == NULL) {
        printf("Error creating output file\n");
        free_tree(root);
        return 1;
    }

    postorder_traversal(root, out_file2, 2);
    fclose(out_file2);

    FILE* out_file3 = fopen(argv[4], "w");
    if (out_file3 == NULL) {
        printf("Error creating output file\n");
        free_tree(root);
        return 1;
    }

    root->xcoord = root->ycoord = 0;
    preorder_traversal(root, out_file3);
    fclose(out_file3);

    free_tree(root);

    return 0;
}