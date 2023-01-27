#include "ramfs.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define MAX_FD 5000 //同时存在的FD数量

typedef struct package{
    int num;
    char *pack[1000];//假定路径深度最多为100,可能不够
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

void release_pkg(pkg package){
    for(int i=0;i<package.num;i++){
        free(package.pack[i]);
    }
}
int check_fd(int fd){
    if(fd>MAX_FD||fd<0)
        return -1;
    return 0;
}
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
    if(pathname[0]!='/'){
        data.num=-1;
        return data;
    }
    char *token;
    token=strtok(tmp,"/");
    while(token!=NULL)
    {
        if(check_valid(token) == -1){
            data.num = -1;
            break;
        }//如果不合法退出
        char *ntoken = malloc(strlen(token)+1);
        strcpy(ntoken,token);
        data.pack[data.num]=ntoken;
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
node* creat_file(node *start,char *name){
    node *newfile = malloc(sizeof(node));
    newfile->first_child = NULL;
    newfile->next_brother = NULL;
    newfile->size = 0;
    newfile->content = NULL;
    newfile->type = ff;
    char *nname = malloc((strlen(name)+1)*sizeof(char));
    strcpy(nname,name);
    newfile->name = nname;//可能需要malloc
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
node* creat_dir(node *start,char *name){
    node *newdir = malloc(sizeof(node));
    newdir->first_child = NULL;
    newdir->next_brother = NULL;
    newdir->size = 0;
    newdir->content = NULL;
    newdir->type = dd;
    char *nname = malloc((strlen(name)+1)*sizeof(char));
    strcpy(nname,name);
    newdir->name = nname;//可能需要malloc
    if(start->first_child == NULL){
        start->first_child = newdir;
    }
    else {
        node *newstart = start->first_child;
        while (newstart->next_brother!=NULL){
            newstart = newstart->next_brother;
        }
        newstart->next_brother = newdir;
    }
    return newdir;
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

    for(int i = 0;i < a_name.num-1;i++){
        if(search(a_name.pack[i],now)==NULL){
            release_pkg(a_name);
            return -1;//找不到路径
        }

        else
            now = search(a_name.pack[i],now);
    }//此时的now是目标上级目录
    if(search(a_name.pack[a_name.num-1],now)==NULL){    //找不到最终地址
        if(check_O_CREAT(flags)==1){
            int tmplen = strlen(pathname);
            if(pathname[tmplen-1] == '/'||now->type==ff){
                release_pkg(a_name);
                return -1;//文件名后是'/'或者上级是文件
            }

            now = creat_file(now,a_name.pack[a_name.num-1]);       //创建
        }
        else{
            release_pkg(a_name);
            return -1;
        }
    }
    else
        now = search(a_name.pack[a_name.num-1],now);
    if(now->type == ff && pathname[strlen(pathname)-1]=='/'){
        release_pkg(a_name);
        return -1;
    }//文件后有'/'

    if(now->type == dd){        //目录省略flag
        int fd = find_empty_fd();
        tree[fd].Is_using = 1;
        tree[fd].path = now;
        //tree[fd].flags=?;tree[fd].offset=?目录不设置
        release_pkg(a_name);
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
        release_pkg(a_name);
        return fd;
    }
    

}

int rclose(int fd) {
    if(check_fd(fd)==-1)
        return -1;
    if(tree[fd].Is_using == 0)
        return -1;
    tree[fd].path = NULL;
    tree[fd].Is_using = 0;
    return 0;
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    if(check_fd(fd)==-1)
        return -1;
    if(tree[fd].Is_using == 0||tree[fd].Can_write==0||tree[fd].path->type==dd)
        return -1;
    if(tree[fd].offset + count > tree[fd].path->size){
        void *newcontent = realloc(tree[fd].path->content,tree[fd].offset+count);
        tree[fd].path->content = newcontent;
        for(int i=tree[fd].path->size;i<tree[fd].offset;i++){
            *((char*)newcontent+i) = '\0';
        }
        tree[fd].path->size = tree[fd].offset + count;
    }
    memcpy(tree[fd].path->content+tree[fd].offset,buf,count);
    tree[fd].offset += count;
    return count;
}

ssize_t rread(int fd, void *buf, size_t count) {

    if(check_fd(fd)==-1)
        return -1;
    if(tree[fd].Is_using == 0||tree[fd].Can_read==0||tree[fd].path->type==dd)
        return -1;
    if(tree[fd].offset + count > tree[fd].path->size){
        count = tree[fd].path->size - tree[fd].offset;
    }
    memcpy(buf,tree[fd].path->content+tree[fd].offset,count);
    tree[fd].offset += count;
    return count;

}

off_t rseek(int fd, off_t offset, int whence) {
    if(check_fd(fd)==-1||tree[fd].Is_using==0)
        return -1;
    if(whence == 0){
        if(offset<0)
            return -1;
        else
            tree[fd].offset = offset;
    }
    else if(whence == 1){
        if(tree[fd].offset+offset<0)
            return -1;
        else
            tree[fd].offset += offset;
    }
    else if(whence == 2){
        if(tree[fd].path->size+offset<0)
            return -1;
        else
            tree[fd].offset = tree[fd].path->size+offset;
    }
    else
        return -1;
    return tree[fd].offset;
}

int rmkdir(const char *pathname) {
    pkg a_name = analyze(pathname);
    if(a_name.num==-1||a_name.num==0)
        return -1;
    node *start = &root;
    for(int i = 0;i < a_name.num-1;i++){
        if(search(a_name.pack[i],start)==NULL){
            release_pkg(a_name);
            return -1;//找不到路径
        }

        else
            start = search(a_name.pack[i],start);
    }//此时的start是目标上级目录
    if (search(a_name.pack[a_name.num-1],start)!=NULL||start->type==ff){
        release_pkg(a_name);
        return -1;//已经存在或者上级是文件
    }

    else{
        creat_dir(start,a_name.pack[a_name.num-1]);
        release_pkg(a_name);
        return 0;
    }

}

int rrmdir(const char *pathname) {
    pkg a_name = analyze(pathname);
    if(a_name.num==-1||a_name.num==0)
        return -1;
    node *start = &root;
    for(int i = 0;i < a_name.num-1;i++){
        if(search(a_name.pack[i],start)==NULL){
            release_pkg(a_name);
            return -1;//找不到路径
        }

        else
            start = search(a_name.pack[i],start);
    }//此时的start是目标上级目录
    if (search(a_name.pack[a_name.num-1],start)==NULL){
        release_pkg(a_name);
        return -1;//不存在
    }

    node *father = start;
    start = search(a_name.pack[a_name.num-1],start);//start为要删除的目录
    if(start->type == ff||start->first_child != NULL){
        release_pkg(a_name);
        return -1;//是文件或者不是空目录
    }


    if(father->first_child == start){
        father->first_child = start->next_brother;
    }
    else {
        node *newstart = father->first_child;
        while(newstart->next_brother != start){
            newstart = newstart->next_brother;
        }
        newstart->next_brother = start->next_brother;
    }//维护链表

    free(start->name);
    free(start);
    release_pkg(a_name);
    return 0;
}

int runlink(const char *pathname) {

    pkg a_name = analyze(pathname);
    if(a_name.num==-1||a_name.num==0)
        return -1;
    if(pathname[strlen(pathname)-1]=='/'){
        release_pkg(a_name);
        return -1;
    }
    node *start = &root;
    for(int i = 0;i < a_name.num-1;i++){
        if(search(a_name.pack[i],start)==NULL){
            release_pkg(a_name);
            return -1;//找不到路径
        }

        else
            start = search(a_name.pack[i],start);
    }//此时的start是目标上级目录
    if (search(a_name.pack[a_name.num-1],start)==NULL){
        release_pkg(a_name);
        return -1;//不存在
    }

    node *father = start;
    start = search(a_name.pack[a_name.num-1],start);//start为要删除的文件
    if(start->type == dd){
        release_pkg(a_name);
        return -1;//是目录
    }


    if(father->first_child == start){
        father->first_child = start->next_brother;
    }
    else {
        node *newstart = father->first_child;
        while(newstart->next_brother != start){
            newstart = newstart->next_brother;
        }
        newstart->next_brother = start->next_brother;
    }//维护链表


    free(start->content);
    free(start->name);
    free(start);
    release_pkg(a_name);
    return 0;
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


