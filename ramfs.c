#include "ramfs.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_FD 5000 //同时存在的FD数量
typedef struct package{
    int num;
    char *pack[100];//假定路径深度最多为100,可能不够
}pkg;
typedef struct node{
    enum type{ff,dd}type;
    struct node *first_child;//第一个孩子
    struct node *next_brother;//下一个兄弟
    void *content;
    int size;
    char *name;
}node;
typedef struct FD{
    node * path;//对应文件位置
    int offset;
    bool Can_read;
    bool Can_write;
    bool Is_using;
}FD;
FD tree[MAX_FD];
node root;

int check_O_CREAT(int data){
    if((data>>6)%2==1)
        return 1;
    return 0;
}
int check_O_TRUNC(int data){
    if((data>>9)%2==1)
        return 1;
    return 0;
}
int check_O_APPEND(int data){
    if((data>>10)%2==1)
        return 1;
    return 0;
}
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
            data.num = -1;
            break;
        }//如果不合法退出
        data.pack[data.num]=token;
        data.num++;
        token=strtok(NULL,"/");
    }
    return data;
}
node* search(char *target,node *start){
    if(start->first_child == NULL)
        return NULL;
    else{
        node *new_start = start->first_child;
        while (new_start != NULL){
            if(strcmp(target,new_start->name)==0)
                return new_start;
            else
                new_start = new_start->next_brother;
        }
        return NULL;
    }
}
node* creat_file(node *start,const char *name){
    node *newfile = malloc(sizeof(node));
    newfile->first_child = NULL;
    newfile->next_brother = NULL;
    newfile->size = 0;
    newfile->content = NULL;
    newfile->type = ff;
    newfile->name = name;
    if(start->first_child == NULL){
        start->first_child = newfile;
    }
    else {
        node *newstart = start->first_child;
        while (newstart->next_brother!=NULL){
            newstart = newstart->next_brother;
        }
        newstart->next_brother = newfile;
    }
    return newfile;
}
int find_empty_fd(){
    for(int i=0;i<MAX_FD;i++){
        if(tree[i].Is_using == 0)
            return i;
    }
    return -1;
}
int ropen(const char *pathname, int flags) {
    pkg a_name = analyze(pathname);
    if(a_name.num==-1)
        return -1;//名称解析失败
    node * now = &root;//从根目录出发
    if(pathname[0]!='/')
        return -1;//第一位不是'/'
    for(int i = 0;i < a_name.num-1;i++){
        if(search(a_name.pack[i],now)==NULL)
            return -1;//找不到路径
        else
            now = search(a_name.pack[i],now);
    }//此时的now是目标上级目录
    if(search(a_name.pack[a_name.num-1],now)==NULL){    //找不到最终地址
        if(check_O_CREAT(flags)==1){
            int tmplen = strlen(pathname);
            if(pathname[tmplen-1] == '/')
                return -1;//文件名后是'/'
            now = creat_file(now,a_name.pack[a_name.num-1]);       //创建
        }
        else{
            return -1;
        }
    }
    else
        now = search(a_name.pack[a_name.num-1],now);
    if(now->type == ff && pathname[strlen(pathname)-1]=='/')//文件后有'/'
        return -1;
    if(now->type == dd){        //目录省略flag
        int fd = find_empty_fd();
        tree[fd].Is_using = 1;
        tree[fd].path = now;
        //tree[fd].flags=?;tree[fd].offset=?目录不设置
        return fd;
    }
    else{           //地址指向的是文件
        int fd = find_empty_fd();
        tree[fd].Is_using = 1;
        tree[fd].path = now;
        if(flags%2==1){
            tree[fd].Can_write = 1;
            tree[fd].Can_read = 0;//只写
        }
        else{
            tree[fd].Can_read = 1;
            tree[fd].Can_write = 0;//只读
            if((flags>>1)%2 ==1)
                tree[fd].Can_write = 1;//可读可写
            //确定读写状态
        }
        if(check_O_TRUNC(flags)==1&&tree[fd].Can_write==1){//清空
            free(now->content);//释放空间
            now->content = NULL;
            now->size = 0;
        }
        if(check_O_APPEND(flags)==1)
            tree[fd].offset = now->size;
        else
            tree[fd].offset = 0;
        return fd;
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
        tree[i].path = NULL;
        tree[i].Is_using = 0;
    }
    root.type = dd;
    root.name = "/";
    root.content = NULL;
    root.size = -1;
    root.first_child = NULL;
    root.next_brother = NULL;
}


