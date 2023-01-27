/* our main.c */
#include "ramfs.h"
#include <assert.h>
#include <string.h>
int main() {
    init_ramfs(); // 你的初始化操作
    assert(rmkdir("/dir") == 0); // 应当成功
    assert(rmkdir("//dir") == -1); // 应当给出 error，因为⽬录已存在
    assert(rmkdir("/a/b") == -1); // 应当给出 error，因为⽗⽬录不存在
    int fd;
    assert((fd = ropen("//dir///////1.txt", O_CREAT | O_RDWR)) >= 0); // 创建⽂件应当
    assert(rwrite(fd, "hello", 5) == 5); // 应当完整地写⼊
    assert(rseek(fd, 0, SEEK_CUR) == 5); // 当前 fd 的偏移量应该为 5
    assert(rseek(fd, 0, SEEK_SET) == 0); // 应当成功将 fd 的偏移量复位到⽂件开头
    char buf[10];
    assert(rread(fd, buf, 7) == 5); // 只能读到 5 字节，因为⽂件只有 5 字节
    assert(memcmp(buf, "hello", 5) == 0); // rread 应当确实读到 "hello" 5 个字节
    assert(rseek(fd, 3, SEEK_END) == 8); // ⽂件⼤⼩为 5，向后 3 字节则是在第 8 字节
    assert(rwrite(fd, "world", 5) == 5); // 再写 5 字节
    assert(rseek(fd, 5, SEEK_SET) == 5); // 将偏移量重设到 5 字节
    assert(rread(fd, buf, 8) == 8); // 在第 8 字节后写⼊了 5 字节，⽂件⼤⼩ 13 字节；那么从第 5 字节后应当能成功读到 8 字节
    assert(memcmp(buf, "\0\0\0world", 8) == 0); // 3 字节的空隙应当默认填 0
    assert(rclose(fd) == 0); // 关闭打开的⽂件应当成功
    assert(rclose(fd + 1) == -1); //关闭未打开的⽂件应当失败
    return 0;
}