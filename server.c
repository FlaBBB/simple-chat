#include "server.h"

int running = 1;
int server_fd;

// initialize the connections struct
struct
{
    char **name;
    pthread_t *threads;
    int *sockets;
    int size;
} connections;

int num_con = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void int_handler(int _)
{
    running = 0;

    puts("\rstopping...");
    shutdown(server_fd, SHUT_RDWR);
}

void broadcast_message(int con, char *msg1, size_t l_msg1, char *msg2, size_t l_msg2)
{
    for (int i = 0; i < num_con; i++)
    {
        // check bad fd (disconected client)
        if (connections.sockets[i] < 0 || (connections.sockets[i] == con && msg2 == NULL && l_msg2 == -1))
            continue;

        if (connections.sockets[i] == con && msg2 != NULL && l_msg2 != -1)
        {
            send_message(connections.sockets[i], msg2, l_msg2);
            continue;
        }
        send_message(connections.sockets[i], msg1, l_msg1);
    }
}

void *handle_connection(void *__ncon)
{
    int ncon = (int)(intptr_t)__ncon;
    int new_socket = connections.sockets[ncon];

    char rbuffer[BUFFER_SIZE];
    char wbuffer1[BUFFER_SIZE];
    char wbuffer2[BUFFER_SIZE];
    size_t l_wbuffer1;
    size_t l_wbuffer2;

    int err = receive_message(new_socket, rbuffer, BUFFER_SIZE);
    if (err < 0)
    {
        perror("receive() failed");
        pthread_exit((void *)EXIT_FAILURE);
    }

    size_t lenname = strlen(rbuffer);

    connections.name[ncon] = (char *)malloc(sizeof(char) * lenname);
    memcpy(connections.name[ncon], rbuffer, lenname);

    printf("new connection %s#%d\n", connections.name[ncon], ncon + 1);

    sprintf(wbuffer1, "%s#%d join the server", connections.name[ncon], ncon + 1);
    l_wbuffer1 = strlen(wbuffer1);

    pthread_mutex_lock(&lock);
    broadcast_message(new_socket, wbuffer1, l_wbuffer1, NULL, 0);
    pthread_mutex_unlock(&lock);

    while (1)
    {
        // clear rbuffer
        memset(rbuffer, 0, BUFFER_SIZE);

        err = receive_message(new_socket, rbuffer, BUFFER_SIZE);
        if (err < 0)
        {
            printf("client %s - %d disconnected\n", connections.name[ncon], ncon + 1);

            sprintf(wbuffer1, "%s#%d leave the server", connections.name[ncon], ncon + 1);
            l_wbuffer1 = strlen(wbuffer1);

            pthread_mutex_lock(&lock);
            broadcast_message(new_socket, wbuffer1, l_wbuffer1, NULL, -1);
            pthread_mutex_unlock(&lock);

            break;
        }

        printf("%s - %d says \"%s\n\"", connections.name[ncon], ncon + 1, rbuffer);

        sprintf(wbuffer1, "%s#%d : %s", connections.name[ncon], ncon + 1, rbuffer);
        sprintf(wbuffer2, "%s#%d [You]: %s", connections.name[ncon], ncon + 1, rbuffer);
        l_wbuffer1 = strlen(wbuffer1);
        l_wbuffer2 = strlen(wbuffer2);

        pthread_mutex_lock(&lock);
        broadcast_message(new_socket, wbuffer1, l_wbuffer1, wbuffer2, l_wbuffer2);
        pthread_mutex_unlock(&lock);
    }

    close(new_socket);
    connections.sockets[ncon] = -1;

    return NULL;
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }

    int port = atoi(argv[1]);

    int new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    connections.size = 10;
    connections.threads = (pthread_t *)malloc(sizeof(pthread_t) * connections.size);
    connections.sockets = (int *)malloc(sizeof(int) * connections.size);
    connections.name = (char **)malloc(sizeof(char *) * connections.size);

    printf("listening with port %d...\n", port);

    signal(SIGINT, int_handler);

    while (running)
    {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0)
        {
            continue;
        }

        if (num_con >= connections.size)
        {
            pthread_mutex_lock(&lock);
            connections.size *= 2;
            connections.threads = (pthread_t *)realloc(connections.threads, sizeof(pthread_t) * connections.size);
            connections.sockets = (int *)realloc(connections.sockets, sizeof(int) * connections.size);
            connections.name = (char **)realloc(connections.name, sizeof(char *) * connections.size);
            pthread_mutex_unlock(&lock);
        }

        pthread_mutex_lock(&lock);
        connections.sockets[num_con] = new_socket;
        pthread_create(&connections.threads[num_con], NULL, handle_connection, (void *)(intptr_t)num_con);
        num_con++;
        pthread_mutex_unlock(&lock);
    }

    puts("stopped");

    return 0;
}