#ifndef QLSG_H
#define QLSG_H

// type of structure for a hash database
typedef struct _TCHDB
{
    unsigned char type;                          // database type
    unsigned char flags;                         // additional flags
    unsigned int bnum;                         // number of the bucket array
    unsigned char apow;                          // power of record alignment
    unsigned char fpow;                          // power of free block pool number
    unsigned char opts;                          // options
    char *path;                            // path of the database file
    int fd;                                // file descriptor of the database file
    unsigned int omode;                        // connection mode
    unsigned int rnum;                         // number of the records
    unsigned int fsiz;                         // size of the database file
    unsigned int frec;                         // offset of the first record
    unsigned int lrec;                         // offset of the last record
    unsigned int iter;                         // offset of the iterator
    char *map;                             // pointer to the mapped memory
    unsigned int msiz;                         // size of the mapped memory
    unsigned int *ba32;                        // 32-bit bucket array    
    unsigned int align;                        // record alignment
    unsigned int runit;                        // record reading unit
    int fbpmax;                        // maximum number of the free block pool
    int fbpsiz;                        // size of the free block pool
    void *fbpool;                          // free block pool
    int fbpnum;                        // number of free block pool
    int fbpmis;                        // number of missing retrieval of free block pool
    int ecode;                             // last happened error code
    int fatal;                            // whether a fatal error occured
    unsigned int inode;                        // inode number
    time_t mtime;                          // modification time
}TCHDB;

// enumeration for additional flags
enum 
{                                   
    HDBFOPEN = 1 << 0,                     // whether opened
        HDBFFATAL = 1 << 1                     // whetehr with fatal error
};

// enumeration for open modes
enum 
{                                   
    HDBOREADER = 1 << 0,                   // open as a reader
        HDBOWRITER = 1 << 1,                   // open as a writer
        HDBOCREAT = 1 << 2,                    // writer creating
        HDBOTRUNC = 1 << 3,                    // writer truncating
};


TCHDB *tchdbnew(void);
void tchdbdel(TCHDB *hdb);
int tchdbopen(TCHDB *hdb, const char *path, int omode);
int tchdbclose(TCHDB *hdb);
int tchdbput(TCHDB *hdb, const char *kbuf, int ksiz, const char *vbuf, int vsiz);
char* tchdbget(TCHDB *hdb, const char *kbuf, int ksiz, int *sp);
int tchdbout(TCHDB *hdb, const char *kbuf, int ksiz);
int tchdbiterinit(TCHDB *hdb);
char* tchdbiternext(TCHDB *hdb, int *sp);
const char* tchdbpath(TCHDB *hdb);
int tchdbexist(const char *path);


#endif