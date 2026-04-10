#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv) {
    int show_checksums = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--show-checksums=true") == 0 || strcmp(argv[i], "--checksums=true") == 0) {
            show_checksums = 1;
        } else if (strcmp(argv[i], "--show-checksums") == 0 || strcmp(argv[i], "--checksums") == 0) {
            if (i + 1 < argc && strcmp(argv[i+1], "true") == 0) {
                show_checksums = 1;
            }
        }
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6972);
    addr.sin_addr.s_addr = INADDR_ANY; 
    
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        return 1;
    }
    
    unsigned char buf[512];
    while(1) {
        int n = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
        if (n > 0) {
            int print_len = n;
            if (!show_checksums && n > 1) {
                print_len = n - 1; // strip checksum byte
            }
            for (int i = 0; i < print_len; i++) {
                printf("%02x", buf[i]);
            }
            printf("\n");
            fflush(stdout);
        }
    }
    close(sock);
    return 0;
}
