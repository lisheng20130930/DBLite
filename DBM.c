#include "libos.h"
#include "DBM.h"
#ifdef WIN32
#include "io.h"
#else
#include "sys/io.h"
#include <sys/time.h>
#include <sys/mman.h>
#endif
#include "errno.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "assert.h"

#define HDBFILEMODE    00644             // permission of created files
#define HDBIOBUFSIZ    8192              // size of an I/O buffer

#define HDBMAGICNUM    "ToKyO CaBiNeT"   // magic number on environments of big endian
#define HDBHEADSIZ     128               // size of the reagion of the header
#define HDBTYPEOFF     32                // offset of the region for the database type
#define HDBFLAGSOFF    33                // offset of the region for the additional flags
#define HDBAPOWOFF     34                // offset of the region for the alignment power
#define HDBFPOWOFF     35                // offset of the region for the free block pool power
#define HDBOPTSOFF     36                // offset of the region for the options
#define HDBBNUMOFF     40                // offset of the region for the bucket number
#define HDBRNUMOFF     48                // offset of the region for the record number
#define HDBFSIZOFF     56                // offset of the region for the file size
#define HDBFRECOFF     64                // offset of the region for the first record offset
#define HDBOPAQUEOFF   80                // offset of the region for the opaque field

#define HDBDEFBNUM     16381             // default bucket number
#define HDBDEFAPOW     4                 // default alignment power
#define HDBMAXAPOW     16                // maximum alignment power
#define HDBDEFFPOW     10                // default free block pool power
#define HDBMAXFPOW     20                // maximum free block pool power
#define HDBMINRUNIT    48                // minimum record reading unit
#define HDBMAXHSIZ     32                // minimum record header size
#define HDBFBPBSIZ     64                // base region size of the free block pool
#define HDBFBPESIZ     4                 // size of each region of the free block pool
#define HDBFBPMGFREQ   256               // frequency to merge the free block pool
#define HDBDRPUNIT     65536             // unit size of the delayed record pool
#define HDBDRPLAT      2048              // latitude size of the delayed record pool

typedef struct _TCHREC
{                         // type of structure for a record
    unsigned int off;                          // offset of the record
    unsigned int rsiz;                         // size of the whole record
    unsigned char magic;                         // magic number
    unsigned char hash;                          // second hash value
    unsigned int left;                         // offset of the left child record
    unsigned int right;                        // offset of the right child record
    unsigned int ksiz;                         // size of the key
    unsigned int vsiz;                         // size of the value
    unsigned short psiz;                         // size of the padding
    const char *kbuf;                      // pointer to the key
    const char *vbuf;                      // pointer to the value
    unsigned int boff;                         // offset of the body
    char *bbuf;                            // buffer of the body
}TCHREC;

typedef struct _HDBFB
{                         // type of structure for a free block
    unsigned int off;                          // offset of the block
    unsigned int rsiz;                         // size of the block
}HDBFB;

enum 
{                                   // enumeration for magic numbers
    HDBMAGICREC = 0xc8,                    // for data block
    HDBMAGICFB = 0xb0                      // for free block
};

enum 
{                                   // enumeration for duplication behavior
    HDBPDOVER,                             // overwrite an existing value
    HDBPDKEEP,                             // keep the existing value
};


//////////////////////////////////////////////////////////////////////////
#define QP_UINT16_MAX   65535

#define _TC_VERSION    "0.2.9"
#define _TC_LIBVER     103
#define _TC_FORMATVER  "0.1"

/* Get the larger value of two integers. */
#define tclmax(a,b)  (((a) > (b)) ? (a) : (b))
#define tclmin(a,b)  (((a) < (b)) ? (a) : (b))

/* Duplicate a string on memory. */
char *tcstrdup(const char *str)
{
    int size = strlen(str);
    char *p = (char*)malloc(size + 1);
    memcpy(p, str, size);
    p[size] = '\0';
    return p;
}

void *tcmemdup(const void *ptr, size_t size)
{
    char *p = NULL;
    assert(ptr && size >= 0);
    p = (char*)malloc(size + 1);
    memcpy(p, ptr, size);
    p[size] = '\0';
    return p;
}

#define TC_SETVNUMBUF(TC_len, TC_buf, TC_num) \
    do { \
    int _TC_num = (TC_num); \
    if(_TC_num == 0){ \
    ((char*)(TC_buf))[0] = 0; \
    (TC_len) = 1; \
    } else { \
    (TC_len) = 0; \
    while(_TC_num > 0){ \
    int _TC_rem = _TC_num & 0x7f; \
    _TC_num >>= 7; \
    if(_TC_num > 0){ \
    ((char*)(TC_buf))[(TC_len)] = -_TC_rem - 1; \
    } else { \
    ((char*)(TC_buf))[(TC_len)] = _TC_rem; \
    } \
    (TC_len)++; \
    } \
    } \
    }while(0)

/* set a buffer for a variable length number of 64-bit */
#define TC_SETVNUMBUF64(TC_len, TC_buf, TC_num) \
    do { \
    int _TC_num = (TC_num); \
    if(_TC_num == 0){ \
    ((char*)(TC_buf))[0] = 0; \
    (TC_len) = 1; \
    } else { \
    (TC_len) = 0; \
    while(_TC_num > 0){ \
    int _TC_rem = _TC_num & 0x7f; \
    _TC_num >>= 7; \
    if(_TC_num > 0){ \
    ((char*)(TC_buf))[(TC_len)] = -_TC_rem - 1; \
    } else { \
    ((char*)(TC_buf))[(TC_len)] = _TC_rem; \
    } \
    (TC_len)++; \
    } \
    } \
    }while(0)

/* read a variable length buffer */
#define TC_READVNUMBUF(TC_buf, TC_num, TC_step) \
    do { \
    int _TC_base = 1; \
    int _TC_i = 0; \
    TC_num = 0; \
    while(1){ \
    if(((char*)(TC_buf))[_TC_i] >= 0){ \
    TC_num += ((char*)(TC_buf))[_TC_i] * _TC_base; \
    break; \
    } \
    TC_num += _TC_base * (((char*)(TC_buf))[_TC_i] + 1) * -1; \
    _TC_base *= 128; \
    _TC_i++; \
    } \
    (TC_step) = _TC_i + 1; \
    }while(0)

/* read a variable length buffer */
#define TC_READVNUMBUF64(TC_buf, TC_num, TC_step) \
    do { \
    int _TC_base = 1; \
    int _TC_i = 0; \
    TC_num = 0; \
    while(1){ \
    if(((char*)(TC_buf))[_TC_i] >= 0){ \
    TC_num += ((char*)(TC_buf))[_TC_i] * _TC_base; \
    break; \
    } \
    TC_num += _TC_base * (((char*)(TC_buf))[_TC_i] + 1) * -1; \
    _TC_base *= 128; \
    _TC_i++; \
    } \
    (TC_step) = _TC_i + 1; \
    }while(0)

//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include "windows.h"

#define MAP_FAILED (NULL)

#define PROT_READ 0x1
#define PROT_WRITE 0x2

#define MAP_SHARED 1

void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
    HANDLE hmap;
    void *temp;
    off_t len;
    struct stat st;
    unsigned int o = offset;
    unsigned int l = o & 0xFFFFFFFF;
    unsigned int h = (o >> 32) & 0xFFFFFFFF;
    
    if (!fstat(fd, &st))
        len = st.st_size;
    else
        return NULL;
    
    if ((int)(length + offset) > len)
        length = (size_t)(len - offset);
        
    hmap = CreateFileMapping((HANDLE)_get_osfhandle(fd), 0, PAGE_READWRITE,0, 0, 0);
    
    if (!hmap)
        return MAP_FAILED;
    
    temp = MapViewOfFileEx(hmap, FILE_MAP_ALL_ACCESS, h, l, length, start);
    
    if (!CloseHandle(hmap))
        return NULL;
    
    return temp ? temp : MAP_FAILED;
}

int munmap(void *start, size_t length)
{    
    return !UnmapViewOfFile(start);
}

int ftruncate(int fd, off_t size)
{
    HANDLE hfile = INVALID_HANDLE_VALUE;
    unsigned int curpos;
    
    if(fd < 0) {
        return -1;
    }
    
    hfile = (HANDLE) _get_osfhandle(fd);
    curpos = SetFilePointer(hfile, 0, NULL, FILE_CURRENT);
    if (curpos == 0xFFFFFFFF
        || SetFilePointer(hfile, size, NULL, FILE_BEGIN) == 0xFFFFFFFF
        || !SetEndOfFile(hfile)) 
    {
        int error = GetLastError();        
        switch (error) {
        case ERROR_INVALID_HANDLE:
            errno = EBADF;
            break;
        default:
            errno = EIO;
            break;
        }        
        return -1;
    }
    return 0;
}
#endif

int mm_phys_flush(int fd, char *map, unsigned int msiz)
{
    int err = FALSE;

#ifndef WIN32
    if(msync(map, msiz, MS_SYNC) == -1)
    {
        err = TRUE;
    }
    if(fsync(fd) == -1)
    {
        err = TRUE;
    }
#else
    if(!FlushViewOfFile(map, msiz))
    {
        err = TRUE;
    }
#endif

    return err;
}

//////////////////////////////////////////////////////////////////////////

static void tchdbclear(TCHDB *hdb)
{
    assert(hdb);
    hdb->type = 0;
    hdb->flags = 0;
    hdb->bnum = HDBDEFBNUM;
    hdb->apow = HDBDEFAPOW;
    hdb->fpow = HDBDEFFPOW;
    hdb->opts = 0;
    hdb->path = NULL;
    hdb->fd = -1;
    hdb->omode = 0;
    hdb->rnum = 0;
    hdb->fsiz = 0;
    hdb->frec = 0;
    hdb->iter = 0;
    hdb->map = NULL;
    hdb->msiz = 0;
    hdb->ba32 = NULL;
    hdb->align = 0;
    hdb->runit = 0;
    hdb->fbpmax = 0;
    hdb->fbpsiz = 0;
    hdb->fbpool = NULL;
    hdb->fbpnum = 0;
    hdb->fbpmis = 0;
    hdb->ecode = 0;
    hdb->fatal = FALSE;
}

TCHDB *tchdbnew(void)
{
    TCHDB *hdb = NULL;
    
    hdb = (TCHDB*)malloc(sizeof(TCHDB));
    memset(hdb, 0x00, sizeof(TCHDB));
    tchdbclear(hdb);

    return hdb;
}

static void tcfbpsortbyoff(HDBFB *fbpool, int fbpnum)
{
    int bottom = 0;
    int top = 0;
    int mybot = 0;
    int i = 0;
    HDBFB swap = {0};

    assert(fbpool && fbpnum >= 0);
    fbpnum--;
    bottom = fbpnum / 2 + 1;
    top = fbpnum;
    while(bottom > 0) {
        bottom--;
        mybot = bottom;
        i = 2 * mybot;
        while(i <= top) {
            if(i < top && fbpool[i+1].off > fbpool[i].off) i++;
            if(fbpool[mybot].off >= fbpool[i].off) 
            {
                break;
            }
            swap = fbpool[mybot];
            fbpool[mybot] = fbpool[i];
            fbpool[i] = swap;
            mybot = i;
            i = 2 * mybot;
        }
    }
    while(top > 0) {
        swap = fbpool[0];
        fbpool[0] = fbpool[top];
        fbpool[top] = swap;
        top--;
        mybot = bottom;
        i = 2 * mybot;
        while(i <= top) {
            if(i < top && fbpool[i+1].off > fbpool[i].off) i++;
            if(fbpool[mybot].off >= fbpool[i].off) 
            {
                break;
            }
            swap = fbpool[mybot];
            fbpool[mybot] = fbpool[i];
            fbpool[i] = swap;
            mybot = i;
            i = 2 * mybot;
        }
    }
}

static void tchdbfbpmerge(TCHDB *hdb)
{
    int onum = 0;
    HDBFB *wp = NULL;
    HDBFB *cur = NULL;
    HDBFB *end = NULL;
    HDBFB *next = NULL;

    assert(hdb);    
    onum = hdb->fbpnum;
    tcfbpsortbyoff((HDBFB*)hdb->fbpool, hdb->fbpnum);
    wp = (HDBFB*)hdb->fbpool;;
    cur = wp;
    end = wp + hdb->fbpnum - 1;
    while(cur < end){
        if(cur->off > 0) {
            next = cur + 1;
            if(cur->off + cur->rsiz == next->off)
            {
                cur->rsiz += next->rsiz;
                next->off = 0;
            }
            *(wp++) = *cur;
        }
        cur++;
    }
    if(end->off > 0) *(wp++) = *end;
    hdb->fbpnum = wp - (HDBFB*)hdb->fbpool;
    hdb->fbpmis = (hdb->fbpnum < onum) ? 0 : hdb->fbpnum * -2;
}

static int tcwrite(int fd, const void *buf, size_t size)
{    
    const char *rp = (const char*)buf;

    assert(fd >= 0 && buf && size >= 0);
    do {
        int wb = write(fd, rp, size);
        switch(wb){
        case -1: if(errno != EINTR) return FALSE;
        case 0: break;
        default:
            rp += wb;
            size -= wb;
            break;
        }
    } while(size > 0);
    return TRUE;
}

static int tcseekwrite(TCHDB *hdb, off_t off, void *buf, size_t size){
    assert(hdb && off >= 0 && buf && size >= 0);
    if(lseek(hdb->fd, off, SEEK_SET) == -1){        
        return FALSE;
    }
    if(!tcwrite(hdb->fd, buf, size)){        
        return FALSE;
    }
    return TRUE;
}

static int tchdbsavefbp(TCHDB *hdb)
{
    int bsiz = 0;
    char *buf = NULL;
    char *wp = NULL;
    HDBFB *cur = NULL;
    HDBFB *end = NULL;
    unsigned int base = 0;

    assert(hdb);
    if(hdb->fbpnum > (hdb->fbpmax >> 1)){
        tchdbfbpmerge(hdb);
    }
    else if(hdb->fbpnum > 1) {
        tcfbpsortbyoff((HDBFB*)hdb->fbpool, hdb->fbpnum);
    }
    bsiz = hdb->fbpsiz;
    buf = (char*)malloc(bsiz);
    wp = buf;
    cur = (HDBFB*)hdb->fbpool;
    end = cur + hdb->fbpnum;    
    bsiz -= sizeof(HDBFB) + sizeof(unsigned char) + sizeof(unsigned char);
    while(cur < end && bsiz > 0){
        unsigned int lnum = 0;
        unsigned int noff = cur->off >> hdb->apow;
        int step;
        unsigned int llnum = noff - base;
        TC_SETVNUMBUF64(step, wp, llnum);
        wp += step;
        bsiz -= step;
        lnum = cur->rsiz >> hdb->apow;
        TC_SETVNUMBUF(step, wp, lnum);
        wp += step;
        bsiz -= step;
        base = noff;
        cur++;
    }
    *(wp++) = '\0';
    *(wp++) = '\0';
    if(!tcseekwrite(hdb, hdb->msiz, buf, wp - buf)){
        free(buf);
        return FALSE;
    }
    free(buf);
    return TRUE;
}

static void tchdbsetflag(TCHDB *hdb, int flag, int sign)
{
    char *fp = (char*)hdb->map + HDBFLAGSOFF;
    if(sign){
        *fp |= (unsigned char)flag;
    } else {
        *fp &= ~(unsigned char)flag;
    }
    hdb->flags = *fp;
}

static void tcdumpmeta(TCHDB *hdb, char *hbuf)
{
    unsigned int llnum;

    memset(hbuf, 0, HDBHEADSIZ);
    sprintf(hbuf, "%s\n%s:%d\n", HDBMAGICNUM, _TC_FORMATVER, _TC_LIBVER);
    memcpy(hbuf + HDBTYPEOFF, &(hdb->type), sizeof(hdb->type));
    memcpy(hbuf + HDBFLAGSOFF, &(hdb->flags), sizeof(hdb->flags));
    memcpy(hbuf + HDBAPOWOFF, &(hdb->apow), sizeof(hdb->apow));
    memcpy(hbuf + HDBFPOWOFF, &(hdb->fpow), sizeof(hdb->fpow));
    memcpy(hbuf + HDBOPTSOFF, &(hdb->opts), sizeof(hdb->opts));
    
    llnum = hdb->bnum;
    memcpy(hbuf + HDBBNUMOFF, &llnum, sizeof(llnum));
    llnum = hdb->rnum;
    memcpy(hbuf + HDBRNUMOFF, &llnum, sizeof(llnum));
    llnum = hdb->fsiz;
    memcpy(hbuf + HDBFSIZOFF, &llnum, sizeof(llnum));
    llnum = hdb->frec;
    memcpy(hbuf + HDBFRECOFF, &llnum, sizeof(llnum));
}

int tchdbmemsync(TCHDB *hdb, int phys)
{
    int err = FALSE;
    char hbuf[HDBHEADSIZ];

    assert(hdb);
    if(hdb->fd < 0 || !(hdb->omode & HDBOWRITER)){
        return FALSE;
    }    
    tcdumpmeta(hdb, hbuf);
    memcpy(hdb->map, hbuf, HDBHEADSIZ);
    if(phys) {
        err = mm_phys_flush(hdb->fd, hdb->map, hdb->msiz);
    }
    return err ? FALSE : TRUE;
}

int tchdbclose(TCHDB *hdb)
{
    int err = FALSE;

    assert(hdb);
    if(hdb->fd < 0) {        
        return FALSE;
    }
    
    if(hdb->omode & HDBOWRITER){
        if(!tchdbsavefbp(hdb)) {
            err = TRUE;
        }
        free(hdb->fbpool);
        tchdbsetflag(hdb, HDBFOPEN, FALSE);
    }
    free(hdb->path);
    if((hdb->omode & HDBOWRITER) && !tchdbmemsync(hdb,TRUE)){
        err = TRUE;
    }
    if(munmap(hdb->map, hdb->msiz) == -1){
        err = TRUE;
    }
    if(close(hdb->fd) == -1){        
        err = TRUE;
    }
    hdb->path = NULL;
    hdb->fd = -1;
    return err ? FALSE : TRUE;
}

/* Delete a hash database object. */
void tchdbdel(TCHDB *hdb)
{
    assert(hdb);
    if(hdb->fd >= 0){
        tchdbclose(hdb);
    }
    free(hdb);
}

static int tchdbpadsize(TCHDB *hdb)
{
    int diff = hdb->fsiz & (hdb->align - 1);
    return diff > 0 ? hdb->align - diff : 0;
}

static int tcread(int fd, void *buf, size_t size)
{    
    char *wp = (char*)buf;

    assert(fd >= 0 && buf && size >= 0);
    do {
        int rb = read(fd, wp, size);
        switch(rb){
        case -1: if(errno != EINTR) return FALSE;
        case 0: return size < 1;
        default:
            wp += rb;
            size -= rb;
        }
    } while(size > 0);
    return TRUE;
}

static int tcseekread(TCHDB *hdb, off_t off, void *buf, size_t size)
{
    assert(hdb && off >= 0 && buf && size >= 0);
    if(lseek(hdb->fd, off, SEEK_SET) == -1){
        return FALSE;
    }
    if(!tcread(hdb->fd, buf, size)){
        return FALSE;
    }
    return TRUE;
}

static void tcloadmeta(TCHDB *hdb, const char *hbuf)
{
    unsigned int llnum = 0;

    memcpy(&(hdb->type), hbuf + HDBTYPEOFF, sizeof(hdb->type));
    memcpy(&(hdb->flags), hbuf + HDBFLAGSOFF, sizeof(hdb->flags));
    memcpy(&(hdb->apow), hbuf + HDBAPOWOFF, sizeof(hdb->apow));
    memcpy(&(hdb->fpow), hbuf + HDBFPOWOFF, sizeof(hdb->fpow));
    memcpy(&(hdb->opts), hbuf + HDBOPTSOFF, sizeof(hdb->opts));    
    memcpy(&llnum, hbuf + HDBBNUMOFF, sizeof(llnum));
    hdb->bnum = llnum;
    memcpy(&llnum, hbuf + HDBRNUMOFF, sizeof(llnum));
    hdb->rnum = llnum;
    memcpy(&llnum, hbuf + HDBFSIZOFF, sizeof(llnum));
    hdb->fsiz = llnum;
    memcpy(&llnum, hbuf + HDBFRECOFF, sizeof(llnum));
    hdb->frec = llnum;
}

static int tchdbloadfbp(TCHDB *hdb)
{
    const char *rp = NULL;
    HDBFB *cur = NULL;
    HDBFB *end = NULL;
    unsigned int base = 0;
    int bsiz = 0;
    char *buf = NULL;

    bsiz = hdb->fbpsiz;
    buf = (char*)malloc(bsiz);
    if(!tcseekread(hdb, hdb->msiz, buf, bsiz)){
        free(buf);
        return FALSE;
    }
    rp = buf;
    cur = (HDBFB*)hdb->fbpool;
    end = cur + hdb->fbpmax;    
    while(cur < end && *rp != '\0'){
        int step;
        unsigned int llnum;
        unsigned int lnum;
        TC_READVNUMBUF64(rp, llnum, step);
        base += llnum << hdb->apow;
        cur->off = base;
        rp += step;        
        TC_READVNUMBUF(rp, lnum, step);
        cur->rsiz = lnum << hdb->apow;
        rp += step;
        cur++;
    }
    hdb->fbpnum = cur - (HDBFB *)hdb->fbpool;
    free(buf);
    return TRUE;
}

int tchdbopen(TCHDB *hdb, const char *path, int omode)
{
    struct stat sbuf;
    char hbuf[HDBHEADSIZ];
    unsigned int fbpmax = 0;
    unsigned int fbpsiz = 0;
    int besiz = 0;
    size_t msiz = 0;
    void *map = NULL;
    int mode = 0;
    int fd = 0;

    assert(hdb && path);
    if(hdb->fd >= 0){
        return FALSE;
    }
    mode = O_RDONLY;
    if(omode & HDBOWRITER){
        mode = O_RDWR;
        if(omode & HDBOCREAT){
            mode |= O_CREAT;
        }
    }
#if(defined(WIN32)||defined(__CYGWIN__))
    mode |= O_BINARY;
#endif
    fd = open(path, mode, HDBFILEMODE);
    if(fd < 0) {
        return FALSE;
    }
   
    if((omode & HDBOWRITER) && (omode & HDBOTRUNC)){
        if(ftruncate(fd, 0) == -1){
            close(fd);
            return FALSE;
        }
    }
          
    if(fstat(fd, &sbuf) == -1){
        close(fd);
        return FALSE;
    }
    
    if((omode & HDBOWRITER) && sbuf.st_size < 1){
        unsigned int fbpmax = 1 << hdb->fpow;
        unsigned int fbpsiz = HDBFBPBSIZ + fbpmax * HDBFBPESIZ;
        int besiz = sizeof(int);
        char pbuf[HDBIOBUFSIZ];
        unsigned int psiz = 0;
        int err = FALSE;

        hdb->align = 1 << hdb->apow;
        hdb->fsiz = HDBHEADSIZ + besiz * hdb->bnum + fbpsiz;
        psiz = tchdbpadsize(hdb);
        hdb->fsiz += psiz;
        hdb->frec = hdb->fsiz;
        tcdumpmeta(hdb, hbuf);
        psiz += besiz * hdb->bnum + fbpsiz;        
        memset(pbuf, 0, HDBIOBUFSIZ);    
        if(!tcwrite(fd, hbuf, HDBHEADSIZ)){
            err = TRUE;
        }
        while(psiz > 0){
            if(psiz > HDBIOBUFSIZ){
                if(!tcwrite(fd, pbuf, HDBIOBUFSIZ)){
                    err = TRUE;
                }
                psiz -= HDBIOBUFSIZ;
            } 
            else {
                if(!tcwrite(fd, pbuf, psiz)){
                    err = TRUE;
                }
                psiz = 0;                
            }
        }
        if(err) { 
            close(fd);
            return FALSE;
        }
        sbuf.st_size = hdb->fsiz;
    }
    hdb->fd = fd;
    if(!tcseekread(hdb, 0, hbuf, HDBHEADSIZ)){
        close(fd);
        hdb->fd = -1;
        return FALSE;
    }
    tcloadmeta(hdb, hbuf);
    fbpmax = 1 << hdb->fpow;
    fbpsiz = HDBFBPBSIZ + fbpmax * HDBFBPESIZ;
    if(memcmp(hbuf, HDBMAGICNUM, strlen(HDBMAGICNUM)) 
        || (hdb->type != 0) 
        || hdb->frec < HDBHEADSIZ + fbpsiz        
        || hdb->frec > hdb->fsiz
        || ((unsigned int)sbuf.st_size) != hdb->fsiz)
    {
        close(fd);
        hdb->fd = -1;
        return FALSE;
    }

    besiz = sizeof(int);
    msiz = HDBHEADSIZ + hdb->bnum * besiz;
    map = mmap(0, msiz, PROT_READ | ((omode & HDBOWRITER) ? PROT_WRITE : 0),MAP_SHARED, fd, 0);
    if(map == MAP_FAILED){
        close(fd);
        hdb->fd = -1;
        return FALSE;
    }
    hdb->fbpmax = fbpmax;
    hdb->fbpsiz = fbpsiz;
    hdb->fbpool = (omode & HDBOWRITER) ? malloc(fbpmax * sizeof(HDBFB)) : NULL;
    hdb->fbpnum = 0;
    hdb->fbpmis = 0;
    hdb->path = tcstrdup(path);
    hdb->omode = omode;
    hdb->map = (char*)map;
    hdb->msiz = msiz;
    hdb->ba32 = (unsigned int*)((char *)map + HDBHEADSIZ);
    hdb->align = 1 << hdb->apow;
    hdb->runit = tclmin(tclmax(hdb->align, HDBMINRUNIT), HDBIOBUFSIZ);
    hdb->ecode = 0;
    hdb->fatal = FALSE;
    if((hdb->omode & HDBOWRITER) && !(hdb->flags & HDBFOPEN) && !tchdbloadfbp(hdb)){
        munmap(hdb->map, hdb->msiz);
        close(fd);
        hdb->fd = -1;
        return FALSE;
    }
    if(hdb->omode & HDBOWRITER) {
        tchdbsetflag(hdb, HDBFOPEN, TRUE);
    }
    hdb->inode = (unsigned int)sbuf.st_ino;
    hdb->mtime = sbuf.st_mtime;

    return TRUE;
}

static unsigned int tchdbbidx(TCHDB *hdb, const char *kbuf, int ksiz)
{    
    unsigned int hash = 751;
    assert(hdb && kbuf && ksiz >= 0);
    for(hash = ksiz; ksiz--;){
        hash = hash * 37 + *(unsigned char *)kbuf++;
    }
    return hash % hdb->bnum;
}

static unsigned int tchdbgetbucket(TCHDB *hdb, unsigned int bidx)
{    
    unsigned int lnum = 0;
    assert(hdb && bidx >= 0);
    lnum = hdb->ba32[bidx];
    return (unsigned int)lnum << hdb->apow;
}

static unsigned char tcrechash(const char *kbuf, int ksiz)
{
    unsigned int hash = 0;
    assert(kbuf && ksiz >= 0);
    kbuf += ksiz - 1;
    hash = 19780211;
    for(hash = ksiz; ksiz--;){
        hash = hash * 31 + *(unsigned char *)kbuf--;
    }
    return hash;
}

static int tchdbreadrec(TCHDB *hdb, TCHREC *rec, char *rbuf)
{
    unsigned int rsiz = 0;
    const char *rp = NULL;
    unsigned int lnum = 0;
    unsigned short snum = 0;
    int step = 0;
    int hsiz = 0;

    assert(hdb && rec && rbuf);    
    rsiz = tclmin(hdb->fsiz - rec->off, hdb->runit);
    if(rsiz < (sizeof(unsigned char) + sizeof(unsigned int))){
        return FALSE;
    }
    if(!tcseekread(hdb, rec->off, rbuf, rsiz)){
        return FALSE;
    }
    rp = rbuf;
    rec->magic = *(unsigned char*)(rp++);
    if(rec->magic == HDBMAGICFB){
        memcpy(&lnum, rp, sizeof(lnum));
        rec->rsiz = lnum;
        return TRUE;
    } 
    else if(rec->magic != HDBMAGICREC){
        return FALSE;
    }
    rec->hash = *(unsigned char*)(rp++);
    memcpy(&lnum, rp, sizeof(lnum));
    rec->left = (unsigned int)lnum << hdb->apow;
    rp += sizeof(lnum);
    memcpy(&lnum, rp, sizeof(lnum));
    rec->right = (unsigned int)lnum << hdb->apow;
    rp += sizeof(lnum);
    
    memcpy(&snum, rp, sizeof(snum));
    rec->psiz = snum;
    rp += sizeof(snum);
       
    TC_READVNUMBUF(rp, lnum, step);
    rec->ksiz = lnum;
    rp += step;
    TC_READVNUMBUF(rp, lnum, step);
    rec->vsiz = lnum;
    rp += step;
    hsiz = rp - rbuf;
    rec->rsiz = hsiz + rec->ksiz + rec->vsiz + rec->psiz;
    rec->kbuf = NULL;
    rec->vbuf = NULL;
    rec->boff = rec->off + hsiz;
    rec->bbuf = NULL;
    rsiz -= hsiz;
    if(rsiz >= rec->ksiz){
        rec->kbuf = rp;
        rsiz -= rec->ksiz;
        rp += rec->ksiz;
        if(rsiz >= rec->vsiz){
            rec->vbuf = rp;
        }
    }
    return TRUE;
}

static int tchdbreadrecbody(TCHDB *hdb, TCHREC *rec)
{
    int bsiz = 0;
    assert(hdb && rec);
    bsiz = rec->ksiz + rec->vsiz;
    rec->bbuf = (char*)malloc(bsiz + 1);
    if(!tcseekread(hdb, rec->boff, rec->bbuf, bsiz)){
        return FALSE;
    }
    rec->kbuf = rec->bbuf;
    rec->vbuf = rec->bbuf + rec->ksiz;
    return TRUE;
}

static int tcreckeycmp(const char *abuf, int asiz, const char *bbuf, int bsiz)
{
    assert(abuf && asiz >= 0 && bbuf && bsiz >= 0);
    if(asiz > bsiz) return 1;
    if(asiz < bsiz) return -1;
    return memcmp(abuf, bbuf, asiz);
}

static int tchdbfbpsplice(TCHDB *hdb, TCHREC *rec, unsigned int nsiz)
{
    unsigned int off = 0;
    unsigned int rsiz = 0;
    HDBFB *pv = NULL;
    HDBFB *ep = NULL;

    assert(hdb && rec && nsiz > 0);
    if(hdb->fbpnum < 1) {
        return FALSE;
    }
    off = rec->off + rec->rsiz;
    rsiz = rec->rsiz;
    pv = (HDBFB*)hdb->fbpool;
    ep = pv + hdb->fbpnum;
    while(pv < ep){
        if(pv->off == off && rsiz + pv->rsiz >= nsiz){
            if(hdb->iter == pv->off){
                hdb->iter += pv->rsiz;
            }
            rec->rsiz += pv->rsiz;
            ep--;
            pv->off = ep->off;
            pv->rsiz = ep->rsiz;
            hdb->fbpnum--;
            return TRUE;
        }
        pv++;
    }
    return FALSE;
}

static int tchdbwritefb(TCHDB *hdb, unsigned int off, unsigned int rsiz)
{
    unsigned int lnum = 0;
    char rbuf[HDBMAXHSIZ];
    char *wp = NULL;

    assert(hdb && off > 0 && rsiz > 0);    
    wp = rbuf;
    *(unsigned char *)(wp++) = HDBMAGICFB;
    lnum = rsiz;
    memcpy(wp, &lnum, sizeof(lnum));
    wp += sizeof(lnum);
    if(!tcseekwrite(hdb,off,rbuf,wp-rbuf)){
        return FALSE;
    }
    return TRUE;
}

static void tcfbpsortbyrsiz(HDBFB *fbpool, int fbpnum)
{
    int bottom = 0;
    int top = 0;
    int mybot = 0;
    int i = 0;
    HDBFB swap = {0};

    assert(fbpool && fbpnum >= 0);
    fbpnum--;
    bottom = fbpnum / 2 + 1;
    top = fbpnum;
    while(bottom > 0){
        bottom--;
        mybot = bottom;
        i = 2 * mybot;
        while(i <= top){
            if(i<top&&fbpool[i+1].rsiz>fbpool[i].rsiz){
                i++;
            }
            if(fbpool[mybot].rsiz >= fbpool[i].rsiz){
                break;
            }
            swap = fbpool[mybot];
            fbpool[mybot] = fbpool[i];
            fbpool[i] = swap;
            mybot = i;
            i = 2 * mybot;
        }
    }
    while(top > 0) {
        swap = fbpool[0];
        fbpool[0] = fbpool[top];
        fbpool[top] = swap;
        top--;
        mybot = bottom;
        i = 2 * mybot;
        while(i <= top){
            if(i < top && fbpool[i+1].rsiz > fbpool[i].rsiz){
                i++;
            }
            if(fbpool[mybot].rsiz >= fbpool[i].rsiz){
                break;
            }
            swap = fbpool[mybot];
            fbpool[mybot] = fbpool[i];
            fbpool[i] = swap;
            mybot = i;
            i = 2 * mybot;
        }
    }
}


static void tchdbfbpinsert(TCHDB *hdb, unsigned int off, unsigned int rsiz)
{
    HDBFB *pv = NULL;

    assert(hdb && off > 0 && rsiz > 0);    
    if(hdb->fpow < 1){
        return;
    }
    pv = (HDBFB*)hdb->fbpool;
    if(hdb->fbpnum >= hdb->fbpmax){
        tchdbfbpmerge(hdb);
        tcfbpsortbyrsiz((HDBFB*)hdb->fbpool, hdb->fbpnum);
        if(hdb->fbpnum >= hdb->fbpmax){
            int dnum = (hdb->fbpmax >> 2) + 1;
            memmove(pv, pv + dnum, (hdb->fbpnum - dnum) * sizeof(*pv));
            hdb->fbpnum -= dnum;
        }
        hdb->fbpmis = 0;
    }
    pv = pv + hdb->fbpnum;
    pv->off = off;
    pv->rsiz = rsiz;
    hdb->fbpnum++;
}

static int tchdbfbpsearch(TCHDB *hdb, TCHREC *rec)
{
    unsigned int rsiz = 0;
    HDBFB *pv = NULL;
    HDBFB *ep = NULL;
    unsigned int fsiz = 0;
    unsigned int psiz = 0;
    unsigned int noff = 0;

    assert(hdb && rec);    
    if(hdb->fbpnum < 1){
        rec->off = hdb->fsiz;
        rec->rsiz = 0;
        return TRUE;
    }
    rsiz = rec->rsiz;
    pv = (HDBFB*)hdb->fbpool;
    ep = pv + hdb->fbpnum;
    while(pv < ep){
        if(pv->rsiz > rsiz){
            if(pv->rsiz > (rsiz << 1)){
                fsiz = hdb->fsiz;
                hdb->fsiz = pv->off + rsiz;
                psiz = tchdbpadsize(hdb);
                hdb->fsiz = fsiz;
                noff = pv->off + rsiz + psiz;
                rec->off = pv->off;
                rec->rsiz = noff - pv->off;
                pv->off = noff;
                pv->rsiz -= rec->rsiz;
                return tchdbwritefb(hdb, pv->off, pv->rsiz);
            }
            rec->off = pv->off;
            rec->rsiz = pv->rsiz;
            ep--;
            pv->off = ep->off;
            pv->rsiz = ep->rsiz;
            hdb->fbpnum--;
            return TRUE;
        }
        pv++;
    }
    rec->off = hdb->fsiz;
    rec->rsiz = 0;
    hdb->fbpmis++;
    if(hdb->fbpmis >= HDBFBPMGFREQ){
        tchdbfbpmerge(hdb);
        tcfbpsortbyrsiz((HDBFB*)hdb->fbpool, hdb->fbpnum);
    }
    return TRUE;
}

static void tchdbsetbucket(TCHDB *hdb, unsigned int bidx, unsigned int off)
{
    unsigned int lnum = 0;
    assert(hdb && bidx >= 0);
    lnum = off >> hdb->apow;
    hdb->ba32[bidx] = lnum;
}

static int tchdbwriterec(TCHDB *hdb, TCHREC *rec, unsigned int bidx, unsigned int entoff)
{
    unsigned int ofsiz = 0;
    char stack[HDBIOBUFSIZ];
    int bsiz = 0;
    char *rbuf = NULL;
    char *wp = NULL;
    unsigned int lnum = 0;
    unsigned short snum = 0;
    char *pwp = NULL;
    int step;
    int hsiz = 0;
    unsigned int rsiz = 0;

    assert(hdb && rec);    
    ofsiz = hdb->fsiz;
    
    bsiz = rec->rsiz > 0 ? rec->rsiz : HDBMAXHSIZ + rec->ksiz + rec->vsiz + hdb->align;
    rbuf = (bsiz <= HDBIOBUFSIZ) ? stack : (char*)malloc(bsiz);
    wp = rbuf;
    *(unsigned char*)(wp++) = HDBMAGICREC;
    *(unsigned char*)(wp++) = rec->hash;
    lnum = rec->left >> hdb->apow;
    lnum = lnum;
    memcpy(wp, &lnum, sizeof(lnum));
    wp += sizeof(lnum);
    lnum = rec->right >> hdb->apow;
    lnum = lnum;
    memcpy(wp, &lnum, sizeof(lnum));
    wp += sizeof(lnum);
    
    pwp = wp;
    wp += sizeof(snum);
    
    TC_SETVNUMBUF(step, wp, rec->ksiz);
    wp += step;
    TC_SETVNUMBUF(step, wp, rec->vsiz);
    wp += step;
    hsiz = wp - rbuf;
    rsiz = hsiz + rec->ksiz + rec->vsiz;
    if(rec->rsiz < 1){
        unsigned short psiz = 0;
        hdb->fsiz += rsiz;
        psiz = tchdbpadsize(hdb);
        rec->rsiz = rsiz + psiz;
        rec->psiz = psiz;
        hdb->fsiz += psiz;
    }
    else if(rsiz > rec->rsiz){
        if(rbuf != stack) {
            free(rbuf);
        }
        if(tchdbfbpsplice(hdb, rec, rsiz)){            
            return tchdbwriterec(hdb, rec, bidx, entoff);
        }        
        if(!tchdbwritefb(hdb, rec->off, rec->rsiz)){
            return FALSE;
        }
        tchdbfbpinsert(hdb, rec->off, rec->rsiz);
        rec->rsiz = rsiz;
        if(!tchdbfbpsearch(hdb, rec)){
            return FALSE;
        }
        return tchdbwriterec(hdb, rec, bidx, entoff);
    } 
    else{        
        unsigned int fsiz = 0;        
        unsigned int noff = 0;
        unsigned int nsiz = 0;
        unsigned int psiz = rec->rsiz - rsiz;

        if(psiz > QP_UINT16_MAX){            
            unsigned int fsiz = hdb->fsiz;
            hdb->fsiz = rec->off + rsiz;
            psiz = tchdbpadsize(hdb);
            hdb->fsiz = fsiz;
            noff = rec->off + rsiz + psiz;
            nsiz = rec->rsiz - rsiz - psiz;
            rec->rsiz = noff - rec->off;
            rec->psiz = psiz;
            if(!tchdbwritefb(hdb, noff, nsiz)){
                if(rbuf != stack){
                    free(rbuf);
                }
                return FALSE;
            }
            tchdbfbpinsert(hdb, noff, nsiz);
        }
        rec->psiz = psiz;
    }
    snum = rec->psiz;
    snum = snum;
    memcpy(pwp, &snum, sizeof(snum));
    rsiz = rec->rsiz;
    rsiz -= hsiz;
    memcpy(wp, rec->kbuf, rec->ksiz);
    wp += rec->ksiz;
    rsiz -= rec->ksiz;
    memcpy(wp, rec->vbuf, rec->vsiz);
    wp += rec->vsiz;
    rsiz -= rec->vsiz;
    memset(wp, 0, rsiz);
    if(!tcseekwrite(hdb, rec->off, rbuf, rec->rsiz)){
        if(rbuf != stack) {
            free(rbuf);
        }
        hdb->fsiz = ofsiz;
        return FALSE;
    }
    if(hdb->fsiz != ofsiz){
        unsigned int llnum = hdb->fsiz;
        llnum = llnum;
        memcpy(hdb->map + HDBFSIZOFF, &llnum, sizeof(llnum));
    }
    if(rbuf != stack) {
        free(rbuf);
    }
    if(entoff > 0){
        unsigned int lnum = rec->off >> hdb->apow;
        lnum = lnum;
        if(!tcseekwrite(hdb, entoff, &lnum, sizeof(unsigned int))){
            return FALSE;
        }
    } 
    else {
        tchdbsetbucket(hdb, bidx, rec->off);
    }
    return TRUE;
}

static int tchdbputimpl(TCHDB *hdb, const char *kbuf, int ksiz, const char *vbuf, int vsiz,int over)
{    
    unsigned int bidx = tchdbbidx(hdb, kbuf, ksiz);
    unsigned int off = tchdbgetbucket(hdb, bidx);
    unsigned char hash = tcrechash(kbuf, ksiz);
    unsigned int entoff = 0;
    TCHREC rec;
    char rbuf[HDBIOBUFSIZ];
    unsigned int llnum = 0;

    assert(hdb && kbuf && ksiz >= 0 && vbuf && vsiz >= 0);
    while(off > 0) {
        rec.off = off;
        if(!tchdbreadrec(hdb, &rec, rbuf)) {
            return FALSE;
        }
        if(hash > rec.hash) {
            off = rec.left;
            entoff = rec.off + (sizeof(unsigned char) + sizeof(unsigned char));
        }
        else if(hash < rec.hash){
            off = rec.right;
            entoff = rec.off + (sizeof(unsigned char) + sizeof(unsigned char)) +sizeof(unsigned int);
        } 
        else {
            int kcmp = 0;

            if(!rec.kbuf && !tchdbreadrecbody(hdb, &rec)){
                return FALSE;
            }
            
            kcmp = tcreckeycmp(kbuf, ksiz, rec.kbuf, rec.ksiz);
            if(kcmp > 0){
                off = rec.left;
                free(rec.bbuf);
                rec.kbuf = NULL;
                rec.bbuf = NULL;
                entoff = rec.off + (sizeof(unsigned char) + sizeof(unsigned char));
            } 
            else if(kcmp < 0){
                off = rec.right;
                free(rec.bbuf);
                rec.kbuf = NULL;
                rec.bbuf = NULL;
                entoff = rec.off + (sizeof(unsigned char) + sizeof(unsigned char)) +sizeof(unsigned int);
            } 
            else {
                int rv = FALSE;
                int nvsiz = 0;

                switch(over)
                {
                case HDBPDKEEP:
                    free(rec.bbuf);
                    return FALSE;
                default:
                    break;
                }
                free(rec.bbuf);
                rec.ksiz = ksiz;
                rec.vsiz = vsiz;
                rec.kbuf = kbuf;
                rec.vbuf = vbuf;
                return tchdbwriterec(hdb, &rec, bidx, entoff);
            }
        }
    }
    rec.rsiz = HDBMAXHSIZ + ksiz + vsiz;
    if(!tchdbfbpsearch(hdb, &rec)){
        return FALSE;
    }
    rec.hash = hash;
    rec.left = 0;
    rec.right = 0;
    rec.ksiz = ksiz;
    rec.vsiz = vsiz;
    rec.psiz = 0;
    rec.kbuf = kbuf;
    rec.vbuf = vbuf;
    if(!tchdbwriterec(hdb, &rec, bidx, entoff)){
        return FALSE;
    }
    hdb->rnum++;
    llnum = hdb->rnum;
    llnum = llnum;
    memcpy(hdb->map + HDBRNUMOFF, &llnum, sizeof(llnum));
    return TRUE;
}

int tchdbput(TCHDB *hdb, const char *kbuf, int ksiz, const char *vbuf, int vsiz)
{
    assert(hdb && kbuf && ksiz >= 0 && vbuf && vsiz >= 0);
    if(hdb->fd < 0 || !(hdb->omode & HDBOWRITER)) {
        return FALSE;
    }    
    return tchdbputimpl(hdb, kbuf, ksiz, vbuf, vsiz, HDBPDOVER);
}


char* tchdbget(TCHDB *hdb, const char *kbuf, int ksiz, int *sp)
{
    TCHREC rec = {0};
    char rbuf[HDBIOBUFSIZ];
    unsigned int bidx = 0;
    unsigned int off = 0;
    unsigned char hash = 0;

    assert(hdb && kbuf && ksiz >= 0 && sp);
    if (hdb->fd < 0) {
        return NULL;
    }
    
    bidx = tchdbbidx(hdb, kbuf, ksiz);
    off = tchdbgetbucket(hdb, bidx);
    hash = tcrechash(kbuf, ksiz);
    while(off > 0) {
        rec.off = off;
        if(!tchdbreadrec(hdb, &rec, rbuf)){
            return NULL;
        }
        if (hash > rec.hash){
            off = rec.left;
        } 
        else if(hash < rec.hash){
            off = rec.right;
        } 
        else {
            int kcmp = 0;
            if(!rec.kbuf && !tchdbreadrecbody(hdb, &rec)) {
                return NULL;
            }
            kcmp = tcreckeycmp(kbuf, ksiz, rec.kbuf, rec.ksiz);
            if(kcmp > 0) {
                off = rec.left;
                free(rec.bbuf);
                rec.kbuf = NULL;
                rec.bbuf = NULL;
            } 
            else if(kcmp < 0){
                off = rec.right;
                free(rec.bbuf);
                rec.kbuf = NULL;
                rec.bbuf = NULL;
            }
            else {
                if (!rec.vbuf&&!tchdbreadrecbody(hdb,&rec)) {
                    return NULL;
                }
                if(NULL!=rec.bbuf){
                    memmove(rec.bbuf, rec.vbuf, rec.vsiz);
                    rec.bbuf[rec.vsiz] = '\0';
                    *sp = rec.vsiz;
                    return rec.bbuf;
                }
                *sp = rec.vsiz;
                return (char*)tcmemdup(rec.vbuf, rec.vsiz);
            }
        }
    }    
    return NULL;
}


int tchdbout(TCHDB *hdb, const char *kbuf, int ksiz)
{
    unsigned int bidx = tchdbbidx(hdb, kbuf, ksiz);
    unsigned int off = tchdbgetbucket(hdb, bidx);
    unsigned char hash = tcrechash(kbuf, ksiz);
    unsigned int entoff = 0;
    char rbuf[HDBIOBUFSIZ];
    unsigned int llnum = 0;
    TCHREC rec;

    assert(hdb && kbuf && ksiz >= 0);
    if(hdb->fd < 0 || !(hdb->omode & HDBOWRITER)){
        return FALSE;
    }
    bidx = tchdbbidx(hdb, kbuf, ksiz);
    off = tchdbgetbucket(hdb, bidx);
    hash = tcrechash(kbuf, ksiz);
    entoff = 0;
    
    while(off > 0) {
        rec.off = off;
        if(!tchdbreadrec(hdb, &rec, rbuf)){
            return FALSE;
        }
        if(hash > rec.hash){
            off = rec.left;
            entoff = rec.off + (sizeof(unsigned char) + sizeof(unsigned char));
        } 
        else if(hash < rec.hash){
            off = rec.right;
            entoff = rec.off + (sizeof(unsigned char) + sizeof(unsigned char)) + sizeof(unsigned int);
        } 
        else {
            int kcmp = 0;
            if(!rec.kbuf && !tchdbreadrecbody(hdb, &rec)){
                return FALSE;
            }
            kcmp = tcreckeycmp(kbuf, ksiz, rec.kbuf, rec.ksiz);
            if(kcmp > 0){
                off = rec.left;
                free(rec.bbuf);
                rec.kbuf = NULL;
                rec.bbuf = NULL;
                entoff = rec.off + (sizeof(unsigned char) + sizeof(unsigned char));
            } 
            else if(kcmp < 0){
                off = rec.right;
                free(rec.bbuf);
                rec.kbuf = NULL;
                rec.bbuf = NULL;
                entoff = rec.off + (sizeof(unsigned char) + sizeof(unsigned char)) + sizeof(unsigned int);
            } 
            else {
                unsigned int child = 0;

                free(rec.bbuf);
                rec.bbuf = NULL;
                if(!tchdbwritefb(hdb, rec.off, rec.rsiz)){
                    return FALSE;
                }
                tchdbfbpinsert(hdb, rec.off, rec.rsiz);                
                if(rec.left > 0 && rec.right < 1){
                    child = rec.left;
                } 
                else if(rec.left < 1 && rec.right > 0){
                    child = rec.right;
                } 
                else if(rec.left < 1 && rec.left < 1){
                    child = 0;
                } 
                else {
                    unsigned int right = 0;
                    unsigned int toff = 0;
                    unsigned int lnum = 0;

                    child = rec.left;
                    right = rec.right;
                    rec.right = child;
                    while(rec.right > 0){
                        rec.off = rec.right;
                        if(!tchdbreadrec(hdb, &rec, rbuf)){
                            return FALSE;
                        }
                    }
                    toff = rec.off + (sizeof(unsigned char) + sizeof(unsigned char) + sizeof(unsigned int));
                    lnum = right >> hdb->apow;
                    lnum = lnum;
                    if(!tcseekwrite(hdb, toff, &lnum, sizeof(unsigned int))){
                        return FALSE;
                    }
                }
                if(entoff > 0){
                    unsigned int lnum = child >> hdb->apow;
                    if(!tcseekwrite(hdb, entoff, &lnum, sizeof(unsigned int))){
                        return FALSE;
                    }
                } 
                else {
                    tchdbsetbucket(hdb, bidx, child);
                }
                hdb->rnum--;
                llnum = hdb->rnum;
                llnum = llnum;
                memcpy(hdb->map + HDBRNUMOFF, &llnum, sizeof(llnum));
                return TRUE;
            }
        }
    }
    
    return FALSE;
}


int tchdbiterinit(TCHDB *hdb)
{
    assert(hdb);
    if(hdb->fd < 0){
        return FALSE;
    }
    hdb->iter = hdb->frec;
    return TRUE;
}

char *tchdbiternext(TCHDB *hdb, int *sp)
{
    TCHREC rec;
    char rbuf[HDBIOBUFSIZ];

    assert(hdb && sp);
    if (hdb->fd < 0 || hdb->iter < 1) {
        return NULL;
    }   

    while(hdb->iter < hdb->fsiz) {
        rec.off = hdb->iter;
        if(!tchdbreadrec(hdb, &rec, rbuf)) {
            return NULL;
        }
        hdb->iter += rec.rsiz;
        if(rec.magic == HDBMAGICREC) {
            if(rec.kbuf) {
                *sp = rec.ksiz;
                return (char*)tcmemdup(rec.kbuf, rec.ksiz);
            }
            if(!tchdbreadrecbody(hdb, &rec)) {
                return NULL;
            }
            rec.bbuf[rec.ksiz] = '\0';
            *sp = rec.ksiz;
            return rec.bbuf;
        }
    }
    
    return NULL;
}

const char *tchdbpath(TCHDB *hdb)
{
    assert(hdb);
    if(hdb->fd < 0){
        return NULL;
    }
    return hdb->path;
}


#ifndef WIN32
static int is_file_exist(const char *path)  
{  
    struct stat statbuff;  

    if(stat(path, &statbuff) < 0) {  
        return FALSE; 
    }
     
    return TRUE;
}
#else
static int is_file_exist(const char *path)  
{
    int filesize = -1;
    FILE *fp = NULL;  
    fp = fopen(path, "rb");
    if(fp == NULL) { 
        return FALSE;
    }    
    fclose(fp);
    return TRUE;
}
#endif

int tchdbexist(const char *path)
{
    return is_file_exist(path);
}
