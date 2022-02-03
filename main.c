#include <stdio.h>
#include "trade.h"

#define COUNT_ARGUMENTS 2

int main(int argc, char const *argv[])
{
    int ret = 0;
    queuenode_t queue;
    FILE *fp_out;
    input_t in;

    // инициализация очереди
    queue = create_queue();

    switch (argc)
    {
    case 1:
        ret = in_stream(&queue); // входные данные из потока ввода
        break;
    case 2:
        fp_out = fopen("output.txt", "wt");
        if (fp_out != NULL)
        {
            ret = in_file(&queue, argv[1], fp_out); // входные данные из файла
            fclose(fp_out);
        }
        else
        {
            fprintf(stderr, "Error open out file: %s\n", "output.txt");
            ret = -1;
        }
        break;
    default:
        fprintf(stderr, "Error run. (Count arguments = %d)\n", argc);
        ret = -1;
        break;
    }

    // освобождение очереди
    while (queue != NULL)
        dequeue(&queue, &in);

    return ret;
}
