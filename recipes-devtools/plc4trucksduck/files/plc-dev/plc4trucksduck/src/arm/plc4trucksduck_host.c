#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <signal.h>
#include <sys/select.h>
#include <time.h>

#define PAYLOAD_LEN 512
#define LED_PATH "/sys/class/leds/uthp:led2/brightness"

int verbose = 0;
#define LOG_INFO(fmt, ...) do { if (verbose) { printf(fmt "\n", ##__VA_ARGS__); fflush(stdout); } } while(0)
#define LOG_ERR(fmt, ...) do { fprintf(stderr, "ERROR: " fmt "\n", ##__VA_ARGS__); } while(0)

int server_port = 6971;
int client_port = 6972;
char* rpmsg_dev = "/dev/rpmsg_pru30";
char* pru_fw_path = "am335x-pru0-fw";
char* remote_proc_path = "/sys/class/remoteproc/remoteproc1";

volatile int exit_event = 0;
pthread_mutex_t rx_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t rx_cond = PTHREAD_COND_INITIALIZER;
int rx_flag = 0;

void signal_handler(int signum) {
    exit_event = 1;
}

void* netdev_leds(void* arg) {
    int fd = open(LED_PATH, O_WRONLY);
    if (fd < 0) {
        exit_event = 1;
        return NULL;
    }
    struct timespec ts;
    while (!exit_event) {
        pthread_mutex_lock(&rx_mutex);
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;
        while (!rx_flag && !exit_event) {
            int ret = pthread_cond_timedwait(&rx_cond, &rx_mutex, &ts);
            if (ret == ETIMEDOUT) break;
        }
        if (rx_flag) {
            rx_flag = 0;
            pthread_mutex_unlock(&rx_mutex);
            write(fd, "1\n", 2);
            usleep(50000);
            write(fd, "0\n", 2);
            usleep(50000);
        } else {
            pthread_mutex_unlock(&rx_mutex);
        }
    }
    close(fd);
    return NULL;
}

void stop_pru() {
    char path[256];
    snprintf(path, sizeof(path), "%s/state", remote_proc_path);
    int fd = open(path, O_RDWR);
    if (fd < 0) return;
    char state[32] = {0};
    read(fd, state, sizeof(state)-1);
    if (strncmp(state, "offline", 7) != 0) {
        write(fd, "stop\n", 5);
    }
    close(fd);
}

void start_pru() {
    char path[256];
    snprintf(path, sizeof(path), "%s/state", remote_proc_path);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        write(fd, "start\n", 6);
        close(fd);
    }
}

void set_firmware() {
    char path[256];
    snprintf(path, sizeof(path), "%s/firmware", remote_proc_path);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        write(fd, pru_fw_path, strlen(pru_fw_path));
        write(fd, "\n", 1);
        close(fd);
    }
}

struct sockaddr_in recv_addrs[16];
int recv_socks[16];
int send_socks[16];
int num_ifaces = 0;

void setup_sockets() {
    struct ifaddrs *ifap, *ifa;
    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
            if (num_ifaces >= 16) break;
            
            int r_sock = socket(AF_INET, SOCK_DGRAM, 0);
            int opt = 1;
            setsockopt(r_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            struct sockaddr_in bind_addr;
            memset(&bind_addr, 0, sizeof(bind_addr));
            bind_addr.sin_family = AF_INET;
            bind_addr.sin_port = htons(server_port);
            bind_addr.sin_addr = sa->sin_addr;
            if (bind(r_sock, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) == 0) {
                fcntl(r_sock, F_SETFL, O_NONBLOCK);
                recv_socks[num_ifaces] = r_sock;
                LOG_INFO("Receive socket bound on interface (IP %s) port %d", inet_ntoa(sa->sin_addr), server_port);
            } else {
                close(r_sock);
                continue;
            }

            int s_sock = socket(AF_INET, SOCK_DGRAM, 0);
            if (ntohl(sa->sin_addr.s_addr) != INADDR_LOOPBACK) {
                setsockopt(s_sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
            }
            bind_addr.sin_port = 0;
            if (bind(s_sock, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) == 0) {
                fcntl(s_sock, F_SETFL, O_NONBLOCK);
                send_socks[num_ifaces] = s_sock;
                LOG_INFO("Send socket bound on interface (IP %s)", inet_ntoa(sa->sin_addr));
            } else {
                close(s_sock);
                close(r_sock);
                continue;
            }
            
            recv_addrs[num_ifaces] = *sa;
            num_ifaces++;
        }
    }
    freeifaddrs(ifap);
}

void* read_from_pru(void* arg) {
    int rpmsg_fd = *(int*)arg;
    char data[PAYLOAD_LEN];
    while (!exit_event) {
        int n = read(rpmsg_fd, data, PAYLOAD_LEN);
        if (n > 0) {
            pthread_mutex_lock(&rx_mutex);
            rx_flag = 1;
            pthread_cond_signal(&rx_cond);
            pthread_mutex_unlock(&rx_mutex);

            if (verbose) {
                printf("PRU->UDP: ");
                for(int j=0; j<n; j++) printf("%02x", (unsigned char)data[j]);
                printf("\n");
                fflush(stdout);
            }

            for (int i = 0; i < num_ifaces; i++) {
                struct sockaddr_in dest;
                memset(&dest, 0, sizeof(dest));
                dest.sin_family = AF_INET;
                dest.sin_port = htons(client_port);
                if (ntohl(recv_addrs[i].sin_addr.s_addr) == INADDR_LOOPBACK) {
                    dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                } else {
                    dest.sin_addr.s_addr = htonl(INADDR_BROADCAST);
                }
                sendto(send_socks[i], data, n, 0, (struct sockaddr*)&dest, sizeof(dest));
            }
        } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            exit_event = 1;
            break;
        } else {
            usleep(1000);
        }
    }
    return NULL;
}

void* write_to_pru(void* arg) {
    int rpmsg_fd = *(int*)arg;
    char data[PAYLOAD_LEN];
    while (!exit_event) {
        fd_set readfds;
        FD_ZERO(&readfds);
        int max_fd = 0;
        for (int i = 0; i < num_ifaces; i++) {
            FD_SET(recv_socks[i], &readfds);
            if (recv_socks[i] > max_fd) max_fd = recv_socks[i];
        }
        struct timeval tv = {1, 0};
        int ready = select(max_fd + 1, &readfds, NULL, NULL, &tv);
        if (ready > 0) {
            for (int i = 0; i < num_ifaces; i++) {
                if (FD_ISSET(recv_socks[i], &readfds)) {
                    int n = recvfrom(recv_socks[i], data, PAYLOAD_LEN, 0, NULL, NULL);
                    if (n > 0) {
                        if (write(rpmsg_fd, data, n) < 0) {
                            exit_event = 1;
                            break;
                        }
                        if (verbose) {
                            printf("UDP->PRU: ");
                            for(int j=0; j<n; j++) printf("%02x", (unsigned char)data[j]);
                            printf("\n");
                            fflush(stdout);
                        }
                    }
                }
            }
        } else if (ready < 0 && errno != EINTR) {
            exit_event = 1;
            break;
        }
    }
    return NULL;
}

void flush_rpmsg(int fd) {
    char data[PAYLOAD_LEN];
    while(read(fd, data, PAYLOAD_LEN) > 0);
}

int main(int argc, char** argv) {
    if (strstr(argv[0], "j17084")) {
        server_port = 6969;
        client_port = 6970;
        rpmsg_dev = "/dev/rpmsg_pru31";
        pru_fw_path = "am335x-pru1-fw";
        remote_proc_path = "/sys/class/remoteproc/remoteproc2";
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        }
    }

    LOG_INFO("Starting %s communication...", pru_fw_path);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    setup_sockets();
    
    stop_pru();
    set_firmware();
    start_pru();
    
    // Wait up to 10 seconds for the RPMsg device to appear
    int rpmsg_fd = -1;
    for (int i = 0; i < 100; i++) {
        rpmsg_fd = open(rpmsg_dev, O_RDWR | O_NONBLOCK);
        if (rpmsg_fd >= 0) break;
        usleep(100000); // 100ms
    }
    
    if (rpmsg_fd < 0) {
        LOG_ERR("Timed out waiting for PRU RPMsg device at %s", rpmsg_dev);
        perror("open");
        return 1;
    }

    LOG_INFO("RPMsg device opened: %s", rpmsg_dev);

    char dummy = 0;
    write(rpmsg_fd, &dummy, 1);
    flush_rpmsg(rpmsg_fd);
    LOG_INFO("RPMsg buffer flushed.");

    pthread_t r_thread, w_thread, l_thread;
    pthread_create(&r_thread, NULL, read_from_pru, &rpmsg_fd);
    pthread_create(&w_thread, NULL, write_to_pru, &rpmsg_fd);
    pthread_create(&l_thread, NULL, netdev_leds, NULL);

    LOG_INFO("Read/Write/LED threads started.");

    pthread_join(r_thread, NULL);
    pthread_join(w_thread, NULL);
    
    LOG_INFO("Shutting down...");
    exit_event = 1;
    pthread_cond_signal(&rx_cond);
    pthread_join(l_thread, NULL);

    close(rpmsg_fd);
    return 0;
}
