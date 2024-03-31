#include "client.h"

char buffer[BUFFER_SIZE] = {0};
size_t l_buffer;
int hold_flag = 0;

void *receiver(void *arg)
{
    int sock = (int)(intptr_t)arg;
    char rbuffer[BUFFER_SIZE] = {0};
    while (1)
    {
        int err = receive_message(sock, rbuffer, BUFFER_SIZE);
        if (err < 0)
        {
            printf("\ndisconnected from server\n");
            exit(err);
        }

        add_message(rbuffer, strlen(rbuffer));

        hold_flag = 1;
        render_ui(buffer);
        hold_flag = 0;
    }
}

int main(int argc, char const *argv[])
{
    int sock = 0, client_fd;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Enter your name: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    int err = send_message(sock, buffer, strlen(buffer) - 1);
    if (err)
    {
        perror("write() failed");
        exit(err);
    }

    memset(buffer, 0, BUFFER_SIZE);

    message_init();

    pthread_t tid;
    pthread_create(&tid, NULL, receiver, (void *)(intptr_t)sock);

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        l_buffer = get_input_noncanon(buffer, BUFFER_SIZE, &hold_flag);

        int err = send_message(sock, buffer, l_buffer);
        if (err)
        {
            perror("write() failed");
            exit(err);
        }
    }

    close(client_fd);
    return 0;
}