#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

unsigned char checksum(unsigned char *msg, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) sum += msg[i];
    return (unsigned char)(~(sum & 0xFF) + 1);
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    char *hexstr = argv[1];
    
    // basic parsing for "--checksums false" etc just ignore for now or check
    int do_checksum = 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--checksums") == 0 && i + 1 < argc) {
            if (strcmp(argv[i+1], "false") == 0) do_checksum = 0;
        } else if (strncmp(argv[i], "--checksums=", 12) == 0) {
            if (strcmp(argv[i]+12, "false") == 0) do_checksum = 0;
        } else if (argv[i][0] != '-') {
            hexstr = argv[i];
        }
    }

    unsigned char msg[512];
    int len = 0;
    for (int i = 0; i < strlen(hexstr) && len < 510; i+=2) {
        sscanf(&hexstr[i], "%2hhx", &msg[len++]);
    }
    
    if (do_checksum) {
        msg[len] = checksum(msg, len);
        len++;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(6971);
    dest.sin_addr.s_addr = inet_addr("127.0.0.1");
    sendto(sock, msg, len, 0, (struct sockaddr*)&dest, sizeof(dest));
    close(sock);
    return 0;
}
