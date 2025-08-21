#include "socket_client.h"
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

std::string send_to_daemon(const std::string &cmd) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1)
        return "ERROR socket failed";

    struct sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    std::string socket_path = "/tmp/nanocontainer/run/daemon.sock";
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        close(sock);
        return "ERROR connect failed";
    }

    write(sock, cmd.c_str(), cmd.length());

    char buffer[1024];
    ssize_t n = read(sock, buffer, sizeof(buffer) - 1);
    std::string response = "OK";
    if (n > 0) {
        buffer[n] = '\0';
        response = std::string(buffer);
    }

    close(sock);
    return response;
}