#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LEFT(node) node->left
#define RIGHT(node) node->right

typedef struct Node {
  int key;
  struct Node *left;
  struct Node *right;
  int height;
} Node;

void createNode(Node **dest, int key) {
  if (!dest) return;

  if ((*dest = malloc(sizeof(Node))) != NULL) {
    (*dest)->key = key;
    (*dest)->height = 0;
    (*dest)->left = (*dest)->right = NULL;
  } else {
    exit(1);
  }
}

void insert(Node **root, int key) {
  if (!(*root)) {
    createNode(root, key);
    return;
  }

  Node *ptr = *root;

  while (true) {
    if (key < ptr->key) {
      if (LEFT(ptr)) {
        ptr = LEFT(ptr);
      } else {
        createNode(&LEFT(ptr), key);
        return;
      }
    } else if (key > ptr->key) {
      if (RIGHT(ptr)) {
        ptr = RIGHT(ptr);
      } else {
        createNode(&RIGHT(ptr), key);
        return;
      }
    } else {
      return;
    }
  }
}

int main() {
  Node *root = NULL;
  insert(&root, 0);
  insert(&root, 10);
  insert(&root, -5);
  insert(&root, -10);
  insert(&root, -7);

  return 0;
}