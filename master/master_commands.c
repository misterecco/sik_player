#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include "master.h"

static void set_host_addr_hints(struct addrinfo *host_addr_hints) {
    memset(host_addr_hints, 0, sizeof(struct addrinfo));
    host_addr_hints->ai_flags = 0;
    host_addr_hints->ai_family = AF_INET;
    host_addr_hints->ai_socktype = SOCK_DGRAM;
    host_addr_hints->ai_protocol = IPPROTO_UDP;
}

static void get_server_address(char *server_name, char* port,
                               struct addrinfo *host_addr_hints, struct addrinfo **host_addr_result) {
    set_host_addr_hints(host_addr_hints);
    int rc =  getaddrinfo(server_name, port, host_addr_hints, host_addr_result);
    if (rc != 0) {
        syserr("getaddrinfo: %s", gai_strerror(rc));
    }
}

static int bind_player_socket(player_args *pa) {
    struct addrinfo host_addr_hints, *host_addr_result;
    get_server_address(pa->computer, pa->m_port, &host_addr_hints, &host_addr_result);
    int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connect(sock, host_addr_result->ai_addr, host_addr_result->ai_addrlen) != 0) {
        perror("connect player socket");
        close(sock);
        sock = -1;
    }
    freeaddrinfo(host_addr_result);
    return sock;
}


static int add_player(player_list *pl, player_args *pa, int telnet_id) {
    int sock = bind_player_socket(pa);
    if (sock < 0) {
        return -1;
    }
    return player_list_add(pl, sock, telnet_id);
}

static void send_message_to_player(int sock, char* message) {
    size_t len = strlen(message);
    ssize_t rc = write(sock, message, len);
    if (rc < len) {
        perror("write");
    }
}

static void do_simple_command(player_list *pl, player_args *pa, char* command) {
    int index = player_list_find_by_id(pl, pa->id);
    if (index < 0) {
        return;
    }
    send_message_to_player(pl->data[index].socket, command);
}

static void wait_for_title_response(telnet_list *tl, player_list *pl,
                                    player_args *pa) {
    int index = player_list_find_by_id(pl, pa->id);
    if (index < 0) {
        return;
    }
    int sock = pl->data[index].socket;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, "OK %d ", pa->id);
    struct pollfd pf;
    pf.fd = sock;
    pf.events = POLLIN;
    pf.revents = 0;
    if (poll(&pf, 1, 3000) &&
        read(pf.fd, buffer + strlen(buffer), BUFFER_SIZE - strlen(buffer)) >= 0) {
        sprintf(buffer + strlen(buffer), "\n");
        send_message_to_client(tl, pa->telnet_id, buffer);
    } else {
        sprintf(buffer, "ERROR %d\n", pa->id);
        send_message_to_client(tl, pa->telnet_id, buffer);
    }
}

static void do_pause(telnet_list *tl, player_list *pl, player_args *pa) {
    do_simple_command(pl, pa, "PAUSE");
}

static void do_play(telnet_list *tl, player_list *pl, player_args *pa) {
    do_simple_command(pl, pa, "PLAY");
}

void do_quit(telnet_list *tl, player_list *pl, player_args *pa) {
    do_simple_command(pl, pa, "QUIT");
}

void do_title(telnet_list *tl, player_list *pl, player_args *pa) {
    do_simple_command(pl, pa, "TITLE");
    wait_for_title_response(tl, pl, pa);
}

void send_confirmation_to_client(telnet_list *tl, player_list *pl, player_args *pa) {
    char message[20];
    memset(message, 0, sizeof(message));
    sprintf(message, "OK %d\n", pa->id);
    send_message_to_client(tl, pa->telnet_id, message);
}

void send_error_to_client(telnet_list *tl, player_list *pl, player_args *pa) {
    char message[20];
    memset(message, 0, sizeof(message));
    sprintf(message, "ERROR %d\n", pa->id);
    send_message_to_client(tl, pa->telnet_id, message);
}

void start_command(telnet_list *tl, player_list *pl, player_args *pa) {
    pa->id = add_player(pl, pa, pa->telnet_id);
    pa->index = player_list_find_by_id(pl, pa->id);
    if (pa->id < 0 || pa->index < 0) {
        send_error_to_client(tl, pl, pa);
        return;
    }
    pa->start_time = 0;
    run_start_thread(tl, pl, pa);
    send_confirmation_to_client(tl, pl, pa);
}

void at_command(telnet_list *tl, player_list *pl, player_args *pa) {
    pa->id = add_player(pl, pa, pa->telnet_id);
    pa->index = player_list_find_by_id(pl, pa->id);
    if (pa->id < 0 || pa->index < 0) {
        send_error_to_client(tl, pl, pa);
        return;
    }
    run_at_thread(tl, pl, pa);
    send_confirmation_to_client(tl, pl, pa);
}

void title_command(telnet_list *tl, player_list *pl, player_args *pa) {
    run_title_thread(tl, pl, pa);
}

void quit_command(telnet_list *tl, player_list *pl, player_args *pa) {
    player_list_lock();
    if (pl->data[pa->index].start_thread &&
            pl->data[pa->index].is_running == false) {
        pthread_cancel(pl->data[pa->index].start_thread);
        pl->data[pa->index].start_thread = 0;
        pl->data[pa->index].to_delete = true;
    }
    player_list_unlock();
    do_quit(tl, pl, pa);
    send_confirmation_to_client(tl, pl, pa);
}

void play_command(telnet_list *tl, player_list *pl, player_args *pa) {
    do_play(tl, pl, pa);
    send_confirmation_to_client(tl, pl,pa);
}

void pause_command(telnet_list *tl, player_list *pl, player_args *pa) {
    do_pause(tl, pl, pa);
    send_confirmation_to_client(tl, pl, pa);
}