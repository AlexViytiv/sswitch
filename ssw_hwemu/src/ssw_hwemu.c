#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdint.h>

#ifndef __KERNEL__
#include <sys/socket.h>			/* for struct sockaddr */
#endif

#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>

#include "../../ssw_utils/ssw_common/ssw_status.h"

/* sudo ip link set dev tap0 down */

// Lots of the following helper code taken from corresponding functions in src/node.
 //
 #define ASCII_a     (0x41)
 #define ASCII_z     (0x5a)
 #define ASCII_A     (0x61)
 #define ASCII_Z     (0x7a)
 #define ASCII_COLON (0x3a)
 #define ASCII_ZERO  (0x30)


uint8_t g_swid = 0;
uint8_t g_swid_ports_num = 0;

char ssw_hwemu_ascii_low_case(char c)
{
    if (c >= ASCII_a && c <= ASCII_z) {
        return c;
    } else if (c >= ASCII_A && c <= ASCII_Z) {
        return c + (ASCII_a - ASCII_A);
    } else {
        return c;
    }
}

void ssw_hwemu_ascii_to_mac(const char *str, uint8_t addr[6])
{
    int i = 0;
    while (*str != 0 && i < 6) {
        uint8_t byte = 0;
        while (*str != ASCII_COLON && *str != 0) {
            byte <<= 4;
            char low = ssw_hwemu_ascii_low_case(*str);
            if (low >= ASCII_a){
                byte |= low - ASCII_a + 10;
            } else {
                byte |= low - ASCII_ZERO;
            }
            str++;
        }
        addr[i] = byte;

        i++;
        if (*str == 0) {
            break;
        }
        str++;
    }
}

void ssw_hw_set_mac_addr(int fd, const char * mac)
{
    struct ifreq ifr;
    memset(&ifr,  0, sizeof(struct ifreq));

    ifr.ifr_hwaddr.sa_family = 1; // this is ARPHRD_ETHER from if_arp.h
    ssw_hwemu_ascii_to_mac(mac, (uint8_t *)ifr.ifr_hwaddr.sa_data);

    if (ioctl(fd, SIOCSIFHWADDR, &ifr) == -1) {
        printf(" Failed to set MAC address %s\n", mac);
        return;
    }

    return;
}

/** Function allocates and configures new TAP device
 *
 * @param dev: the name of an interface (or '\0'). MUST have enough
 *              space to hold the interface name if '\0' is passed
 * @param tap_fd: file descriptor of TAP interface
 *
 * @return: operation status
 */
ssw_status_t ssw_hw_tap_open(char * tap_name, int * tap_fd)
{
    struct ifreq ifr;
    int          fd, err;
    char       * clonedev = "/dev/net/tun";
    char         port_mac[18];
    sprintf(port_mac, "d2:8c:b1:01:%02x:%02x", g_swid, ++g_swid_ports_num);

    if (tap_fd == NULL) {
        return SC_PARAM_NULL;
    }

    /* Open the clone device */
    if ((fd = open(clonedev, O_RDWR)) < 0 ) {
        return SC_FILE_OPEN_FAILED;
    }

    /* Prepare ifr struct (type "struct ifreq") */
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP;   /* IFF_TAP to create TAP interface */

    /* If a device name was specified, put it into the structure;
     * otherwise, the kernel will try to allocate the "next" device
     * of the specified type */
    if (*tap_name) {
        strncpy(ifr.ifr_name, tap_name, IFNAMSIZ);
    }

    /* Create the device with specified name */
    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
        close(fd);
        return SC_FILE_OPEN_FAILED;
    }

    /* Set MAC address to TAP interface */
    ssw_hw_set_mac_addr(fd, port_mac);
    printf(" MAC address %s configured on port %s.\n", port_mac, tap_name);

    /* If the operation was successful, write back the name of the
     * interface to the variable "tap_name", so the caller can know
     * it. Note that the caller MUST reserve space in *tap_name */
    strcpy(tap_name, ifr.ifr_name);
    *tap_fd = fd;

    return SC_OK;
}

/* Close TAP device */
ssw_status_t ssw_hw_tap_close(int fd)
{
    return (close(fd) != 0) ? SC_FILE_CLOSE_FAILED : SC_OK;
}

/* Write frames to TAP device */
ssw_status_t ssw_hw_tap_write(int fd, char *buffer, int len, int *count)
{
    return ((*count = write(fd, buffer, len)) < 0) ? SC_FILE_READ_FAILED : SC_OK;
}

/* Read frames from TAP device */
ssw_status_t ssw_hw_tap_read(int fd, char *buffer, int len, int *count)
{
    return ((*count = read(fd, buffer, len)) < 0) ? SC_FILE_READ_FAILED : SC_OK;
}

ssw_status_t ssw_hw_port_create(uint8_t switch_id, uint8_t port_id, int *port_fd)
{
    ssw_status_t status = SC_OK;
    char port_tap_name[30];

    sprintf(port_tap_name, "sw%dp%u", switch_id, port_id);

    /* port_fd = ssw_hw_tap_open(port_tap_name); */

    status = ssw_hw_tap_open(port_tap_name, port_fd);  /* tap interface */
    if (CHECK_STATUS_FAIL(status) || (*port_fd == -1)) {
        printf(" Failed to create port %u. [sc: %s]\n", port_id, status_get_str(status));
        return status;
    }

    printf(" Port%u created. Name: %s.\n", port_id, port_tap_name);
    return SC_OK;
}

ssw_status_t ssw_hw_port_destroy(int port_fd)
{
    ssw_status_t status = SC_OK;

    status = ssw_hw_tap_close(port_fd);
    if (CHECK_STATUS_FAIL(status)) {
        printf(" Failed to destroy port.\n");
        return status;
    }

    printf(" Port %d destroyed.\n", port_fd);
    return status;
}

ssw_status_t ssw_init(uint8_t switch_id)
{
    ssw_status_t status = SC_OK;
    int port_fd = -1;
    int port_id = 0;


    printf(" Software Switch %u initialized.\n", switch_id);

    status = ssw_hw_port_create(switch_id, port_id++, &port_fd);
    if (CHECK_STATUS_FAIL(status)) {
        printf(" Failed to create switch port %d. [sc: %d]\n", port_id, status);
        return status;
    }
    printf(" Port %d created under FD%d.\n", port_id, port_fd);


    status = ssw_hw_port_create(switch_id, port_id++, &port_fd);
    if (CHECK_STATUS_FAIL(status)) {
        printf(" Failed to create switch port %d. [sc: %d]\n", port_id, status);
        return status;
    }
    printf(" Port %d created under FD%d.\n", port_id, port_fd);

    printf(" > Switch is working. Press any key to exit ...\n");
    getchar();

    status = ssw_hw_port_destroy(port_fd);
    if (CHECK_STATUS_FAIL(status)) {
        printf(" Failed to destroy switch port %d. [sc: %d]\n", port_id, status);
        return status;
    }
    printf(" Software Switch %u uninitialized.\n", switch_id);

    return SC_OK;
}

int main(int argc, char * argv[])
{
    if (argc != 2) {
        printf(" Please provide Switch ID.\n");
        return -1;
    }

    g_swid = atoi(argv[1]);

    ssw_init(g_swid);

    return 0;
}