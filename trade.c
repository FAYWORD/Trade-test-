#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trade.h"

unsigned int numT = 1; // номер сделки

int in_file(queuenode_t *q, const char *filePath, FILE *fp_out)
{
    int ret = 0;
    FILE *fp;
    input_t in;
    char str[MAX_LEN_IN_STR];
    unsigned int nStr = 0;

    fp = fopen(filePath, "rt"); // открытие файла на чтение
    if (fp != NULL)
    {
        // цикл по строкам файла
        do
        {
            if (fgets(str, MAX_LEN_IN_STR, fp) != NULL)
            {
                nStr++;
                if (strlen(str) < 2)
                    continue;
                ret = strToStruct(str, &in); // получение данных из строки
                if (ret == 0)
                    ret = trade(q, &in, fp_out); // проверка обмена и обмен
                else
                {
                    fprintf(stderr, "Err read str = %d\n", nStr);
                    ret = -1;
                }
            }
        } while ((ret != -1) && (!feof(fp)));
        fclose(fp); // закрытие файла
    }
    else
    {
        fprintf(stderr, "Error open file: %s\n", filePath);
        ret = -1;
    }

    return ret;
}

int in_stream(queuenode_t *q)
{
    int ret = 0;
    unsigned char str[MAX_LEN_IN_STR];
    input_t in;

    scanf("%s", str);

    while (strncmp(str, STR_IN_EXIT, strlen(STR_IN_EXIT))) // бесконечный цикл пока не будет написано слово для выхода (exit)
    {
        ret = strToStruct(str, &in); // получение данных из строки
        if (ret == 0)
            ret = trade(q, &in, NULL); // проверка обмена и обмен
        scanf("%s", str);
    }

    return ret;
}

int strToStruct(char *str, input_t *in)
{
    int ret = 0;
    char *tmp;
    int num = 1;

    tmp = strtok(str, " ,\n");

    // выделение элементов из строки
    while ((tmp != NULL) && (ret == 0))
    {
        switch (num)
        {
        case NUM_IN_TYPE:
            in->type = tmp[0];
            if ((in->type != SYM_REQUEST_O) && (in->type != SYM_REQUEST_C))
            {
                fprintf(stderr, "Err input type = %s\n", tmp);
                ret = -1;
            }
            break;
        case NUM_IN_OID:
            in->OID = atoi(tmp);
            if (in->OID == 0)
            {
                fprintf(stderr, "Err input OID = %s\n", tmp);
                ret = -1;
            }
            break;
        case NUM_IN_SIDE:
            in->Side = tmp[0];
            if ((in->Side != SYM_SIDE_B) && (in->Side != SYM_SIDE_S))
            {
                fprintf(stderr, "Err input side = %s\n", tmp);
                ret = -1;
            }
            break;
        case NUM_IN_QTY:
            in->Qty = atoi(tmp);
            if (in->Qty == 0)
            {
                fprintf(stderr, "Err input Qty = %s\n", tmp);
                ret = -1;
            }
            break;
        case NUM_IN_PRICE:
            in->Price = atof(tmp);
            if (in->Price == 0)
            {
                fprintf(stderr, "Err input Price = %s\n", tmp);
                ret = -1;
            }
            break;
        }
        num++;
        tmp = strtok(NULL, " ,\n");
    }

    return ret;
}

int trade(queuenode_t *q, input_t *in, FILE *fp_out)
{
    int ret = 0;

    // проверка типа операции и выполнение
    switch (in->type)
    {
    case SYM_REQUEST_O:
        switch (in->Side)
        {
        case SYM_SIDE_B:
            wtb(q, in, fp_out); // покупка
            if (in->Qty > 0)
                enqueue(q, in);
            break;
        case SYM_SIDE_S:
            wts(q, in, fp_out); // продажа
            if (in->Qty > 0)
                enqueue(q, in);
            break;
        }
        break;
    case SYM_REQUEST_C:
        ret = del_OID(q, in->OID, fp_out); // снятие с торгов
        break;
    default:
        fprintf(stderr, "Error type: %c\n", in->type);
        break;
    }

    return ret;
}

int wtb(queuenode_t *q, input_t *inB, FILE *fp_out)
{
    int ret = 0;
    input_t inS;
    int i;
    queuenode_t qS;

    qS = create_queue();

    sort_min(q, &qS);

    // цикл по удовлетворяющим предложениям
    while (!is_empty(&qS))
    {
        dequeue(&qS, &inS);
        if (inB->Price >= inS.Price)
        {
            if (inB->Qty > 0)
            {
                // совершиние сделки
                if (inS.Qty >= inB->Qty)
                {
                    inS.Qty -= inB->Qty;
                    if (fp_out == NULL)
                        printf("%c,%u,%c,%u,%u,%u,%.2f\n", SYM_DEAL_T, numT++, SYM_SIDE_S, inS.OID, inB->OID, inB->Qty, inS.Price);
                    else
                        fprintf(fp_out, "%c,%u,%c,%u,%u,%u,%.2f\n", SYM_DEAL_T, numT++, SYM_SIDE_S, inS.OID, inB->OID, inB->Qty, inS.Price);
                    inB->Qty = 0;
                }
                else
                {
                    inB->Qty -= inS.Qty;
                    if (fp_out == NULL)
                        printf("%c,%u,%c,%u,%u,%u,%.2f\n", SYM_DEAL_T, numT++, SYM_SIDE_S, inS.OID, inB->OID, inS.Qty, inS.Price);
                    else
                        fprintf(fp_out, "%c,%u,%c,%u,%u,%u,%.2f\n", SYM_DEAL_T, numT++, SYM_SIDE_S, inS.OID, inB->OID, inS.Qty, inS.Price);
                    inS.Qty = 0;
                }
            }
        }
        if (inS.Qty > 0)
            enqueue(q, &inS);
    }

    return ret;
}

int sort_min(queuenode_t *q, queuenode_t *qS)
{
    int ret = -1;
    input_t in;
    int i;
    unsigned int c = 0;
    input_t min;
    int pSort = 0;
    queuenode_t tmp_q;

    tmp_q = create_queue();

    // разделение списков заявок
    while (!is_empty(q))
    {
        dequeue(q, &in);
        (in.Side == SYM_SIDE_S) ? (enqueue(qS, &in)) : (enqueue(&tmp_q, &in));
    }
    // запись списка заявок на покупку обратно в основной список
    while (!is_empty(&tmp_q))
    {
        dequeue(&tmp_q, &in);
        enqueue(q, &in);
    }

    // сортировка списка продажи простым выбором
    if (!is_empty(qS))
    {
        do
        {
            for (i = 0; i < c; i++)
            {
                dequeue(qS, &in);
                enqueue(&tmp_q, &in);
            }

            dequeue(qS, &in);

            memcpy(&min, &in, sizeof(input_t));
            if (is_empty(qS))
                pSort = 1;
            while (!is_empty(qS))
            {
                dequeue(qS, &in);
                if (in.Price < min.Price)
                {
                    enqueue(&tmp_q, &min);
                    memcpy(&min, &in, sizeof(input_t));
                }
                else
                    enqueue(&tmp_q, &in);
            }

            for (i = 0; i < c; i++)
            {
                dequeue(&tmp_q, &in);
                enqueue(qS, &in);
            }
            enqueue(qS, &min);
            while (!is_empty(&tmp_q))
            {
                dequeue(&tmp_q, &in);
                enqueue(qS, &in);
            }

            c++;
        } while (pSort == 0);
    }

    return ret;
}

int wts(queuenode_t *q, input_t *inS, FILE *fp_out)
{
    int ret = 0;
    input_t inB;
    int i;
    queuenode_t qB;

    qB = create_queue();

    sort_max(q, &qB);

    // цикл по удовлетворяющим предложениям
    while (!is_empty(&qB))
    {
        dequeue(&qB, &inB);

        if (inS->Price <= inB.Price)
        {
            if (inS->Qty > 0)
            {
                // совершиние сделки
                if (inB.Qty >= inS->Qty)
                {
                    inB.Qty -= inS->Qty;
                    if (fp_out == NULL)
                        printf("%c,%u,%c,%u,%u,%u,%.2f\n", SYM_DEAL_T, numT++, SYM_SIDE_B, inB.OID, inS->OID, inS->Qty, inB.Price);
                    else
                        fprintf(fp_out, "%c,%u,%c,%u,%u,%u,%.2f\n", SYM_DEAL_T, numT++, SYM_SIDE_B, inB.OID, inS->OID, inS->Qty, inB.Price);
                    inS->Qty = 0;
                }
                else
                {
                    inS->Qty -= inB.Qty;
                    if (fp_out == NULL)
                        printf("%c,%u,%c,%u,%u,%u,%.2f\n", SYM_DEAL_T, numT++, SYM_SIDE_B, inB.OID, inS->OID, inB.Qty, inB.Price);
                    else
                        fprintf(fp_out, "%c,%u,%c,%u,%u,%u,%.2f\n", SYM_DEAL_T, numT++, SYM_SIDE_B, inB.OID, inS->OID, inB.Qty, inB.Price);
                    inB.Qty = 0;
                }
            }
        }
        if (inB.Qty > 0)
            enqueue(q, &inB);
    }

    return ret;
}

int sort_max(queuenode_t *q, queuenode_t *qB)
{
    int ret = -1;
    input_t in;
    int i;
    unsigned int c = 0;
    input_t max;
    int pSort = 0;
    queuenode_t tmp_q;

    tmp_q = create_queue();

    // разделение списков заявок
    while (!is_empty(q))
    {
        dequeue(q, &in);
        (in.Side == SYM_SIDE_B) ? (enqueue(qB, &in)) : (enqueue(&tmp_q, &in));
    }
    // запись списка заявок на покупку обратно в основной список
    while (!is_empty(&tmp_q))
    {
        dequeue(&tmp_q, &in);
        enqueue(q, &in);
    }

    // сортировка списка покупки простым выбором
    if (!is_empty(qB))
    {
        do
        {
            for (i = 0; i < c; i++)
            {
                dequeue(qB, &in);
                enqueue(&tmp_q, &in);
            }

            dequeue(qB, &in);

            memcpy(&max, &in, sizeof(input_t));
            if (is_empty(qB))
                pSort = 1;
            while (!is_empty(qB))
            {
                dequeue(qB, &in);
                if (in.Price > max.Price)
                {
                    enqueue(&tmp_q, &max);
                    memcpy(&max, &in, sizeof(input_t));
                }
                else
                    enqueue(&tmp_q, &in);
            }

            for (i = 0; i < c; i++)
            {
                dequeue(&tmp_q, &in);
                enqueue(qB, &in);
            }
            enqueue(qB, &max);
            while (!is_empty(&tmp_q))
            {
                dequeue(&tmp_q, &in);
                enqueue(qB, &in);
            }

            c++;
        } while (pSort == 0);
    }

    return ret;
}

queuenode_t create_queue(void)
{
    return (queuenode_t)NULL;
}

void enqueue(queuenode_t *q, input_t *in)
{
    queuenode_t node;

    node = (queuenode_t)malloc(sizeof(struct queueNode));
    node->next = NULL;
    memcpy(&node->in, in, sizeof(input_t));

    if (is_empty(q))
    {
        *q = node->tail = node;
    }
    else
    {
        (*q)->tail = (*q)->tail->next = node;
    }
}

void dequeue(queuenode_t *q, input_t *in)
{
    queuenode_t head;

    if (!is_empty(q))
    {
        head = *q;
        *q = head->next;
        *in = head->in;
        if (*q != NULL)
            (*q)->tail = head->tail;

        free(head);
    }
    else
        in = NULL;
}

int is_empty(queuenode_t *q)
{
    return *q == NULL;
}

int del_OID(queuenode_t *q, unsigned int OID, FILE *fp_out)
{
    int ret = 0;
    input_t in;
    unsigned int tailOID;

    if (!is_empty(q))
    {
        tailOID = (*q)->tail->in.OID;
        while (tailOID != in.OID)
        {
            dequeue(q, &in);
            if (in.OID == OID)
            {
                ret = 0;
                if (fp_out == NULL)
                    printf("%c,%u\n", SYM_DEAL_X, OID);
                else
                    fprintf(fp_out, "%c,%u\n", SYM_DEAL_X, OID);
                if (tailOID == in.OID)
                    break;
            }
            else
                enqueue(q, &in);
        }
    }

    return ret;
}
