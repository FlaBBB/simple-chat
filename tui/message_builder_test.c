#include "message_builder.h"

int main(int argc, char const *argv[])
{
    message_init();
    char buff[2048];
    int idx = 0;
    char ch;
    printf("Windows Width:  %d\n", term_w);
    printf("Windows Height: %d\n", term_h);
    for (size_t i = 0; i < 11; i++)
    {
        char m[100];
        sprintf(m, "Fikri-%d: hello sir!!", i);
        add_message(m, strlen(m));
    }
    
    render_ui("");
    sleep(2);
    for (size_t i = 11; i < 13; i++)
    {
        char m[100];
        sprintf(m, "Fikri-%d: hello sir!!", i);
        add_message(m, strlen(m));
    }
    
    render_ui("");
    sleep(2);

    return 0;
}
