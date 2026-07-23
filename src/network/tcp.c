
#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "tcp.h"
#include "../utils/utils.h"

/// The socket used to communicate via TCP.
static int sock = 0;

/**
 * Get the filedescriptor for socket.
 *
 * @return The socket filedescriptor.
 */
int get_socket()
{
    return sock;
}

/**
 * Close socket with proper error checking.
 */
void close_socket()
{
    if (close(sock) == -1)
        fprintf(stderr, "Failed to close socket, reason: %s.\n",
                strerror(errno));
}

/**
 * Looks up host IP from system, allows connection via hostname.
 *
 * If used as a server it will start to listen on the socket, else
 * it will connect to as a normal client.
 *
 * @param hints Hints used while running getaddrinfo.
 * @param found The addressinfo used to start.
 * @param host The server to connect to, can be NULL when we are a server.
 * @param port Connect to this port.
 * @return The the first resulting adressinfo struct in chain.
 *         Must be freed by freeaddrinfo(..).
 */
struct addrinfo* lookup_host_and_start(
    struct addrinfo *hints, struct addrinfo **found,
    const char *host, const char *port
) {
    if (!isnumber(port)) {
        fprintf(stderr, "Port must be a number!\n");
        exit(EXIT_FAILURE);
    }

    struct addrinfo *ai_res, *rp;
    hints->ai_family = AF_INET; // Only allow IPv4
    hints->ai_socktype = SOCK_STREAM; // only TCP

    int is_host = (hints->ai_flags & AI_PASSIVE) != 0;

    int ret = getaddrinfo(host, port, hints, &ai_res);
    if (ret != 0) {
        fprintf(stderr, "Failed to lookup: %s  %s\nReason: %s\n",
                host, port, gai_strerror(ret));
        exit(EXIT_FAILURE);
    }

    // try the list of addresses until we can successfully bind/connet.
    for (rp = ai_res; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1)
            continue;

        if (is_host) {
            // Had to set socket option to properly release socket.
            // Just using signal handlers for that task worked intermittently.
            const int en = 1;
            if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(int)) < 0 &&
                setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &en, sizeof(int)) < 0)
            {
                fprintf(stderr,
                        "Warning: Failed to set socketoptions: %s\n",
                        strerror(errno));
            }

            if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
                // man listen states that backlog is actually truncated
                // /proc/sys/net/core/somaxconn. Lecture states 5, we can use more.
                if (listen(sock, SOMAXCONN) == 0)
                    break; // success!
            }

        } else {
            // it's a client, just connect
            if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0)
                break; // success!
        }

        close(sock); // failed, close this socket
    }

    if (rp == NULL) {
        const char *method = is_host ? "connect" : "bind";
        fprintf(stderr, "Failed to %s to %s  %s\n", method, host, port);
        exit(EXIT_FAILURE);
    }

    *found = rp;

    return ai_res;
}

/**
 * Sends msg to server and reads the response.
 *
 * @param req The message to send.
 * @param req_sz The request size.
 * @param resp The response buffer.
 * @param resp_sz Size of response buffer.
 * @param host The host to connect to.
 * @param port Connect to this port.
 * @param verbose Print extra debug info.
 * @return 1 on success, 0 on failure.
 */
int send_to_server(
    const char *req, uint32_t req_sz,
    char *resp, uint32_t resp_sz,
    const char *host, const char *port,
    int verbose
) {

    int res = 1;
    struct addrinfo hints = {0},
                    *first = NULL,
                    *found = NULL;
    hints.ai_flags = AI_CANONNAME; // include hostname as canonical name


    first = lookup_host_and_start(&hints, &found, host, port);
    if (verbose) {
        char ip4[INET_ADDRSTRLEN];
        getnameinfo(found->ai_addr, found->ai_addrlen,
                    ip4, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        printf("[verbose] Connected to: %s[%s]  %s\n", found->ai_canonname, ip4, port);
    }
    freeaddrinfo(first); // done with this info

    if (verbose)
        printf("[verbose] Sending %s\n", req);

    write(get_socket(), req, strnlen(req, req_sz));

    int nread = read(get_socket(), resp, resp_sz);
    resp[nread] = '\0';
    if (nread < 1) {
        fprintf(stderr, "Failed to read response.\n");
        res = 0;
    }

    if (verbose) {
        const char *type = resp[0] == '0' ? "error " : "";
        printf("[verbose] Got %sresponse: %s\n", type, resp+1);
    }

    close(get_socket());
    return res;
}
