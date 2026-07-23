
#ifndef _TCP_H_
#define _TCP_H_

#include <stddef.h>

/// Size of response buffer.
#define SRV_RESP_BUFFER_SIZE 255
/// Size of send buffer.
#define TO_SRV_BUFFER_SIZE 32

// forward declare
struct addrinfo;

/**
 * Get the filedescriptor for socket.
 *
 * @return The socket filedescriptor.
 */
int get_socket();

/**
 * Close socket with proper error checking.
 */
void close_socket();

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
);

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
);

 #endif // _TCP_H_
