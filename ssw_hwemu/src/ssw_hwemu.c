#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdint.h>

#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>

/**
 * Allocate Ethernet TAP device,
 *
 * @param[out] devname: pointer to device name (must be large enough).
 * @return opened file descriptor. Return -1 if failed.
 */
int ssw_hw_tap_open0(char * devname)
{
    char tapname[30];
    int i = 0;
    int fd = 0;

    /* Use provided name */
    if(*devname) {
       sprintf(tapname, "/dev/%s", devname);
       printf(" Creating port %s ...\n", tapname);
       return open(tapname, O_RDWR);
    }

    /* Look for free TAP name */
    for(i = 0; i < 255; i++) {
       sprintf(tapname, "/dev/tap%d", i);

       /* Try to open device */
       if((fd = open(tapname, O_RDWR)) > 0) {
          sprintf(devname, "tap%d", i);
          printf(" Port %s has been created.\n", tapname);
          return fd;
       }
    }

    printf(" Port has not been created.\n");
    return -1;
}

int ssw_hw_tap_open(char *devname)
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open("/dev/net/tun", O_RDWR)) < 0 ) {
        printf("ProblemS.\n");
        return ssw_hw_tap_open0(devname);
    }

    memset(&ifr, 0, sizeof(ifr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
    *        IFF_TAP   - TAP device
    *
    *        IFF_NO_PI - Do not provide packet information
    */
    ifr.ifr_flags = IFF_TAP;
    if (*devname) {
        printf(" Creating port %s ...\n", devname);
        strncpy(ifr.ifr_name, devname, IFNAMSIZ);
    }

    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
        close(fd);
        return err;
    }
    strcpy(devname, ifr.ifr_name);
    return fd;
}

int ssw_hw_tap_close(int fd)
{
    return close(fd);
}

/* Write frames to TAP device */
int ssw_hw_tap_write(int fd, char *buffer, int len)
{
    return write(fd, buffer, len);
}

/* Read frames from TAP device */
int ssw_hw_tap_read(int fd, char *buffer, int len)
{
    return read(fd, buffer, len);
}

int ssw_hw_port_create(uint8_t port_id)
{
    int port_fd = 0;
    char port_tap_name[20];

    sprintf(port_tap_name, "/dev/tap_ssw_port%u", port_id);

    port_fd = ssw_hw_tap_open(port_tap_name);
    if (port_fd == -1) {
        printf(" Failed to create port%u.\n", port_id);
        return -1;
    }

    printf(" Port%u created.\n", port_id);
    return port_fd;
}

int ssw_hw_port_destroy(int port_fd)
{
    int status = 0;

    status = ssw_hw_tap_close(port_fd);
    if (status == -1) {
        printf(" Failed to destroy port.\n");
        return -1;
    }

    printf(" Port destroyed.\n");
    return status;
}

int ssw_init(uint8_t switch_id)
{
    int port_fd = 0;
    int status = 0;
    printf(" Software Switch %u initialized.\n", switch_id);

    port_fd = ssw_hw_port_create(0);
    printf(" Port %d created under FD%d.\n", 0, port_fd);
    printf(" > Switch is working. Press any key to exit ...\n");
    getchar();
    status = ssw_hw_port_destroy(port_fd);

    printf(" Software Switch %u uninitialized.\n", switch_id);
    return 0;
}

int main(int argc, char * argv[])
{
    ssw_init(0);

    return 0;
}