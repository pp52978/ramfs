#include "ramfs.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_FD 66000 //同时存在的FD数量
typedef struct package{
    int num;
    char *pack[100];//假定路径深度最多为100,可能不够
}pkg;
typedef struct node{
    enum type{ff,dd}type;
    void *content;
    int size;
    char *name;
    bool Is_open;//当前是否被打开中
    int fd;//对应的fd
    int offset;
    bool read;
    bool write;
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


int check_valid(char *token){
    int len = strlen(token);
    if(len>32)
        return -1;
    else {
        for(int i=0;i<len;i++)
        {
            if(!((token[i]>='A'&&token[i]<='Z')||(token[i]>='a'&&token[i]<='z')||(token[i]>='0'&&token[i]<='9')||(token[i]=='.')))
                return -1;
        }
    }
    return 0;
}//检查文件名合法性
struct package analyze(const char *pathname){
    char tmp[1030];
    strcpy(tmp,pathname);
    pkg data;
    data.num=0;
    char *token;
    token=strtok(tmp,"/");
    while(token!=NULL)
    {
        if(check_valid(token) == -1){
            data.num = 0;
            break;
        }
        data.pack[data.num]=token;
        data.num++;
        token=strtok(NULL,"/");
    }
    data.num--;
    return data;
}
int search(char * target,int start){
    if(tree[start].child_field == NULL)
        return -1;
    else{
        child *new_start = tree[start].child_field;
        while (new_start != NULL){
            if(strcmp(target,tree[new_start->fd].path->name)==0)
                return new_start->fd;
            else
                new_start = new_start->next;
        }
        return -1;
    }

}//返回对象的fd值

int ropen(const char *pathname, int flags) {
    pkg a_name = analyze(pathname);
    if(a_name.num==-1)
        return -1;
    int now = 0;//从根目录出发
    if(pathname[0]!='/')
        return -1;
    for(int i = 0;i <= a_name.num;i++){
        if(search(a_name.pack[i],now)==-1)
            return -1;
        else
            now = search(a_name.pack[i],now);
    }
    if(tree[now].path->type == dd)
        return now;
    else {
        int tplen = strlen(pathname)
    }

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
    root -> type = dd; //后续root可能需要进一步初始化
    root ->fd = 0;
    tree[0].path = root;//0号分给根目录





}


