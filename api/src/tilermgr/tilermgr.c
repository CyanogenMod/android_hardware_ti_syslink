#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* strerror() */
#include <fcntl.h>   /* open() */
#include <stropts.h> /* ioctl() */
#include <unistd.h>  /* close() */
#include <assert.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "tilermgr.h"
/* #include "dmm.h" */

#define DMM_DRIVER_NAME "/dev/tiler"
#define PAGE        4096
#define CONTAINER_W 256
#define CONTAINER_H 128
#define CONTAINER_LEN CONTAINER_W*CONTAINER_H*PAGE
#define IOCSINIT  _IOWR ('z', 100, unsigned long)
#define IOCGALLOC _IOWR ('z', 101, unsigned long)
#define IOCSFREE  _IOWR ('z', 102, unsigned long)

struct dmm_data {
    int pixfmt;
    unsigned short w;
    unsigned short h;
    short seczone;
    unsigned long ssptr;
};

extern int errno;

SSPtr
tilerAlloc(enum kPixelFormat pf, pixels_t w, pixels_t h, short seczone)
{
    SSPtr ptr = NULL;
    int fd;
    int result = -1;
    struct dmm_data *d = NULL;

    fd = open(DMM_DRIVER_NAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open():fail\n");
        fprintf(stderr, "errno(%d) - \"%s\"\n", errno, strerror(errno));
        return ptr;
    }

    d = (struct dmm_data*)malloc(sizeof(struct dmm_data));
    memset(d,0x0,sizeof(struct dmm_data));
    /* store d in a linked list so that we can free it later */

    d->pixfmt  = pf;
    d->w       = w;
    d->h       = h;
    d->seczone = seczone;

    result = ioctl(fd, IOCGALLOC, d);
    if (result < 0) {
        fprintf(stderr, "ioctl():fail\n");
        fprintf(stderr, "errno(%d) - \"%s\"\n", errno, strerror(errno));
        free(d);
        return ptr;
    }

    close(fd);
    return (SSPtr)d->ssptr;
}

void
tilerFree(SSPtr p)
{
    int fd;
    int result = -1;
    struct dmm_data *d = NULL;

    assert(p != NULL);

    fd = open(DMM_DRIVER_NAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open():fail\n");
        fprintf(stderr, "errno(%d) - \"%s\"\n", errno, strerror(errno));
    }

    /* get the corresponding linked list node to this SSPtr instead of this*/
    d = (struct dmm_data*)malloc(sizeof(struct dmm_data));
    memset(d,0x0,sizeof(struct dmm_data));

    d->ssptr = (unsigned long)p;

    result = ioctl(fd, IOCSFREE, d);
    if (result < 0) {
        fprintf(stderr, "ioctl():fail\n");
        fprintf(stderr, "errno(%d) - \"%s\"\n", errno, strerror(errno));
    }

    free(d);
    close(fd);
}

SSPageModePtr
tilerPageModeAlloc(bytes_t len)
{
    SSPtr ptr = NULL;
    int fd;
    int result = -1;
    struct dmm_data *d = NULL;

    assert(len > 0 && len <= CONTAINER_LEN);

    fd = open(DMM_DRIVER_NAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open():fail\n");
        fprintf(stderr, "errno(%d) - \"%s\"\n", errno, strerror(errno));
        return ptr;
    }

    d = (struct dmm_data*)malloc(sizeof(struct dmm_data));
    memset(d,0x0,sizeof(struct dmm_data));
    /* store d in a linked list so that we can free it later */

    d->pixfmt  = 3;   /* MODE_PAGE */
    d->w       = len;
    d->h       = 1; 
    d->seczone = 0;

    result = ioctl(fd, IOCGALLOC, d);
    if (result < 0) {
        fprintf(stderr, "ioctl():fail\n");
        fprintf(stderr, "errno(%d) - \"%s\"\n", errno, strerror(errno));
        free(d);
        close(fd);
        return ptr;
    }

    close(fd);
    return (SSPtr)d->ssptr;
}

void
tilerPageModeFree(SSPageModePtr p)
{
    int fd;
    int result = -1;
    struct dmm_data *d = NULL;

    assert(p != NULL);

    fd = open(DMM_DRIVER_NAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open():fail\n");
        fprintf(stderr, "errno(%d) - \"%s\"\n", errno, strerror(errno));
    }

    /* get the corresponding linked list node to this SSPtr instead of this*/
    d = (struct dmm_data*)malloc(sizeof(struct dmm_data));
    memset(d,0x0,sizeof(struct dmm_data));

    d->ssptr = (unsigned long)p;

    result = ioctl(fd, IOCSFREE, d);
    if (result < 0) {
        fprintf(stderr, "ioctl():fail\n");
        fprintf(stderr, "errno(%d) - \"%s\"\n", errno, strerror(errno));
    }

    free(d);
    close(fd);    
}

/* TODO: stubbing TILER user space APIs for now... */

enum kRefCorner
tilerGetRefCorner(TSPtr p)
{
    return kTopLeft;
}

SSPtr
tilerRealloc(SSPtr p, pixels_t w, pixels_t h)
{
    assert(p != NULL);
    assert(w > 0 && w <= CONTAINER_W);
    assert(h > 0 && h <= CONTAINER_H);

    /* p = tilerAlloc(k8bit, w, h, 0); */
    p = (SSPtr)0x0000da7a;
    return p;
}

SSPageModePtr
tilerPageModeRealloc(SSPageModePtr p, bytes_t len)
{
    assert(p != NULL);
    assert(len > 0 && len <= CONTAINER_LEN);
    /* p = tilerAlloc(0, len, 1, 0); */
    p = (SSPtr)0x0000da7a;
    return p;
}

TSPtr
convertToTilerSpace(SSPtr p, short rotationAndMirroring)
{
    assert(p != NULL);
    return p;
}

TSPageModePtr
convertPageModeToTilerSpace(SSPageModePtr p)
{
    assert(p != NULL);
    return p;
}

