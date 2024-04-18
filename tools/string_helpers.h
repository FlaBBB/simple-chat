#include <stdlib.h>

char **clone_string_array(char **original, size_t size)
{
    char **clone = (char **)malloc(size * sizeof(char *));
    if (clone == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < size; i++)
    {
        if (original[i] == NULL)
            continue;

        int length = strlen(original[i]);

        clone[i] = malloc((length + 1) * sizeof(*clone[i]));
        if (clone[i] == NULL)
        {
            printf("Memory allocation failed\n");
            exit(1);
        }

        memcpy(clone[i], original[i], length);
    }

    return clone;
}

void free_string_array(char **buff, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (buff[i] == NULL)
            continue;

        free(buff[i]);
        buff[i] = NULL;
    }
    free(buff);
    buff = NULL;
}