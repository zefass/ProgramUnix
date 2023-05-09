#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <linux/ioctl.h>

#define DEVICE_PATH "/dev/pci_device"
#define IOCTL_MAC_ADDR _IOR('mac', 1, char*)

int main() {
    unsigned char port_addr[6];

    int fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        printf("Failed to open device file.\n");
        return EXIT_FAILURE;
    }
    
    if (ioctl(fd, IOCTL_MAC_ADDR, port_addr) < 0) {
        printf("Failed to perform ioctl operation.\n");
        close(fd);
        return EXIT_FAILURE;
    }
    
    printf("MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           port_addr[0], port_addr[1], port_addr[2], port_addr[3], port_addr[4], port_addr[5]);

    close(fd);
    return 0;
}
