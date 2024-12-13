#ifndef ROUTES_H
#define ROUTES_H

struct Route {
    char *key;
    char *value;
    struct Route *left;
    struct Route *right;
};

struct Route *initRoute(char* key, char* value);
struct Route *addRoute(struct Route *root, char* key, char* value);
struct Route *search(struct Route *root, char *key);
void inorder(struct Route *root);

#endif // ROUTES_H