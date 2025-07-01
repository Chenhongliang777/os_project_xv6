#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char *target;

// 获取路径中的文件名
char *filename(char *path) {
    char *p = path + strlen(path);
    while (p != path && *p != '/') p--;
    return p == path ? p : p + 1;
}

// 递归查找文件
void find(char *path) {
    int fd;
    struct stat st;
    struct dirent de;
    char buf[512], *p;

    // 打开目录或文件
    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // 获取路径的状态信息
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // 根据路径类型进行处理
    switch (st.type) {
        case T_FILE:
            // 如果是文件且文件名匹配，打印路径
            if (strcmp(filename(path), target) == 0) {
                printf("%s\n", path);
            }
            break;
        case T_DIR:
            // 如果是目录，递归查找
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
                fprintf(2, "find: path too long\n");
                close(fd);
                return;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                find(buf);
            }
            break;
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(2, "Usage: find path filename\n");
        exit(1);
    }
    target = argv[2];
    find(argv[1]);
    exit(0);
}