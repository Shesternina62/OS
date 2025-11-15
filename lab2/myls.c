#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>

#define BLUE  "\x1b[34m"
#define GREEN "\x1b[32m"
#define CYAN  "\x1b[36m"
#define RESET "\x1b[0m"

int flag_l = 0;
int flag_a = 0;

typedef struct {
    char *name;
    char *path;
    struct stat st;
    char *linkto;
} Entry;

static int cmp_entries(const void *a, const void *b) {
    Entry *ea = *(Entry**)a;
    Entry *eb = *(Entry**)b;
    return strcmp(ea->name, eb->name);
}

static char* join(const char *a, const char *b) {
    int need = (a[strlen(a)-1] != '/');
    char *res = malloc(strlen(a) + strlen(b) + 2);
    sprintf(res, need ? "%s/%s" : "%s%s", a, b);
    return res;
}

static void print_mode(mode_t m, char *buf) {
    buf[0] = S_ISDIR(m) ? 'd' :
             S_ISLNK(m) ? 'l' :
             S_ISCHR(m) ? 'c' :
             S_ISBLK(m) ? 'b' :
             S_ISFIFO(m)? 'p' :
             S_ISSOCK(m)? 's' : '-';

    buf[1] = (m & S_IRUSR) ? 'r' : '-';
    buf[2] = (m & S_IWUSR) ? 'w' : '-';
    buf[3] = (m & S_IXUSR) ? 'x' : '-';
    buf[4] = (m & S_IRGRP) ? 'r' : '-';
    buf[5] = (m & S_IWGRP) ? 'w' : '-';
    buf[6] = (m & S_IXGRP) ? 'x' : '-';
    buf[7] = (m & S_IROTH) ? 'r' : '-';
    buf[8] = (m & S_IWOTH) ? 'w' : '-';
    buf[9] = (m & S_IXOTH) ? 'x' : '-';
    buf[10] = '\0';
}

static const char* pick_color(const Entry *e) {
    if (S_ISDIR(e->st.st_mode)) return BLUE;
    if (S_ISLNK(e->st.st_mode)) return CYAN;
    if (S_ISREG(e->st.st_mode) && (e->st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))) return GREEN;
    return NULL;
}

static int terminal_width() {
    struct winsize w;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_col==0) return 80;
    return w.ws_col;
}

static void print_long(Entry **arr, int n) {
    int links_w=0, owner_w=0, group_w=0, size_w=0;
    long long total_blocks=0;

    // вычисляем ширину колонок
    for(int i=0;i<n;i++){
        char tmp[32];
        snprintf(tmp,sizeof(tmp),"%lu",(unsigned long)arr[i]->st.st_nlink);
        if(strlen(tmp)>links_w) links_w = strlen(tmp);

        struct passwd *pw = getpwuid(arr[i]->st.st_uid);
        const char *owner = pw ? pw->pw_name : ""; // пусто вместо '?'
        if(strlen(owner)>owner_w) owner_w = strlen(owner);

        struct group *gr = getgrgid(arr[i]->st.st_gid);
        const char *group = gr ? gr->gr_name : ""; // пусто вместо '?'
        if(strlen(group)>group_w) group_w = strlen(group);

        snprintf(tmp,sizeof(tmp),"%lld",(long long)arr[i]->st.st_size);
        if(strlen(tmp)>size_w) size_w = strlen(tmp);

        total_blocks += arr[i]->st.st_blocks;
    }

    printf("total %lld\n", total_blocks/2);

    for(int i=0;i<n;i++){
        Entry *e = arr[i];
        char mode[11];
        print_mode(e->st.st_mode, mode);

        struct passwd *pw = getpwuid(e->st.st_uid);
        const char *owner = pw ? pw->pw_name : "";
        struct group *gr = getgrgid(e->st.st_gid);
        const char *group = gr ? gr->gr_name : "";

        char timebuf[64];
        struct tm *mt = localtime(&e->st.st_mtime);
        strftime(timebuf,sizeof(timebuf),"%b %e %H:%M",mt);

        printf("%s %*lu %-*s %-*s %*lld %s ",
               mode,
               links_w, (unsigned long)e->st.st_nlink,
               owner_w, owner,
               group_w, group,
               size_w, (long long)e->st.st_size,
               timebuf);

        const char *col = pick_color(e);
        if(col) printf("%s", col);

        if(S_ISLNK(e->st.st_mode) && e->linkto)
            printf("%s -> %s", e->name, e->linkto);
        else
            printf("%s", e->name);

        if(col) printf("%s", RESET);
        printf("\n");
    }
}


static void print_simple(Entry **arr, int n) {
    int termw = terminal_width();
    int maxlen=0;
    for(int i=0;i<n;i++){
        int l = strlen(arr[i]->name);
        if(arr[i]->linkto) l += 4+strlen(arr[i]->linkto);
        if(l>maxlen) maxlen=l;
    }
    int colw = maxlen+2;
    int cols = termw/colw; if(cols<1) cols=1;
    int rows = (n+cols-1)/cols;
    for(int r=0;r<rows;r++){
        for(int c=0;c<cols;c++){
            int idx = c*rows + r;
            if(idx>=n) continue;
            Entry *e = arr[idx];
            const char *col = pick_color(e);
            if(col) printf("%s",col);
            if(S_ISLNK(e->st.st_mode) && e->linkto)
                printf("%s -> %s",e->name,e->linkto);
            else
                printf("%s",e->name);
            if(col) printf("%s",RESET);
            int len = strlen(e->name); if(e->linkto) len += 4+strlen(e->linkto);
            for(int sp=len;sp<colw;sp++) putchar(' ');
        }
        printf("\n");
    }
}

static int list_dir(const char *path){
    DIR *d = opendir(path);
    if(!d){ fprintf(stderr,"myls: %s: %s\n",path,strerror(errno)); return -1;}
    Entry **arr=NULL; int cap=0,n=0;
    struct dirent *de;
    while((de=readdir(d))!=NULL){
        if(!flag_a && de->d_name[0]=='.') continue;
        if(n==cap){ cap=cap?cap*2:32; arr=realloc(arr,cap*sizeof(Entry*));}
        Entry *e = calloc(1,sizeof(Entry));
        e->name = strdup(de->d_name);
        e->path = join(path,de->d_name);
        lstat(e->path,&e->st);
        if(S_ISLNK(e->st.st_mode)){
            char buf[1024]; ssize_t r = readlink(e->path,buf,sizeof(buf)-1);
            if(r>0){ buf[r]=0; e->linkto=strdup(buf);}
        }
        arr[n++] = e;
    }
    closedir(d);
    qsort(arr,n,sizeof(Entry*),cmp_entries);
    if(flag_l) print_long(arr,n);
    else print_simple(arr,n);
    for(int i=0;i<n;i++){ free(arr[i]->name); free(arr[i]->path); if(arr[i]->linkto) free(arr[i]->linkto); free(arr[i]);}
    free(arr);
    return 0;
}

int main(int argc,char **argv){
    int opt;
    while((opt=getopt(argc,argv,"la"))!=-1){
        if(opt=='l') flag_l=1; else if(opt=='a') flag_a=1;
    }

    if(optind==argc){ list_dir("."); return 0;}

    int printed=0;
    for(int i=optind;i<argc;i++){
        struct stat st; if(lstat(argv[i],&st)<0){ perror(argv[i]); continue;}
        if(S_ISDIR(st.st_mode)){
            if(argc-optind>1){ if(printed) printf("\n"); printf("%s:\n",argv[i]);}
            list_dir(argv[i]);
        } else {
            Entry e={0}; e.name=argv[i]; e.path=argv[i]; e.st=st;
            if(S_ISLNK(st.st_mode)){
                char buf[1024]; ssize_t r = readlink(e.path,buf,sizeof(buf)-1);
                if(r>0){ buf[r]=0; e.linkto=strdup(buf);}
            }
            Entry *arr[1]={&e};
            if(flag_l) print_long(arr,1); else print_simple(arr,1);
            if(e.linkto) free(e.linkto);
        }
        printed=1;
    }
    return 0;
}
