#include "ramfs.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
define MAX_FD 66000 //同时存在的FD数量
typedef struct node{
    enum type{FILE,DIR}type;
    void *content;
    char *name;
    bool Is_open;//当前是否被打开中
    int fd;//对应的fd
    int offset;
}node;
typedef struct child{
    int fd;
    struct child *next;//下一个孩子
}child;//孩子域
typedef struct FD{
    struct node * path;//对应文件位置
    struct child * child_field;//孩子域
    bool check;//是否被使用
}FD;
FD tree[MAX_FD];
int ropen(const char *pathname, int flags) {
  // TODO();
}

int rclose(int fd) {
  // TODO();
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
  // TODO();
}

ssize_t rread(int fd, void *buf, size_t count) {
  // TODO();
}

off_t rseek(int fd, off_t offset, int whence) {
  // TODO();
}

int rmkdir(const char *pathname) {

}

int rrmdir(const char *pathname) {
  // TODO();
}

int runlink(const char *pathname) {
  // TODO();
}

void init_ramfs() {
    for(int i=0;i<MAX_FD;i++) {
        tree[i].child_field = NULL;
        tree[i].path = NULL;
        tree[i].check = 0;
    }
    node *root = malloc(sizeof(node));
    root -> type = DIR;



}


