
#ifndef TRADE_H
#define TRADE_H

#define MAX_LEN_IN_STR 256 // максимальная длина строки ввода

#define SYM_REQUEST_O 'O' // заявка
#define SYM_REQUEST_C 'C' // отмена заявки

#define SYM_DEAL_T 'T' // сделка
#define SYM_DEAL_X 'X' // информация об отмене заявки

#define SYM_SIDE_B 'B' // покупка
#define SYM_SIDE_S 'S' // продажа

#define STR_IN_EXIT "exit" // строка для окончания ввода

// перечисление порядка данных в строке
enum NUM_IN
{
    NUM_IN_TYPE = 1,
    NUM_IN_OID,
    NUM_IN_SIDE,
    NUM_IN_QTY,
    NUM_IN_PRICE
};

// входные данные
typedef struct
{
    unsigned char type; // тип операции
    unsigned int OID;   // уникальный номер
    unsigned char Side; // тип сделки (покупка/продажа)
    unsigned int Qty;   // количество
    double Price;       // цена
} input_t;

// очередь
typedef struct queueNode
{
    input_t in;
    struct queueNode *next;
    struct queueNode *tail;
} * queuenode_t;

int in_file(queuenode_t *q, const char *filePath, FILE *fp_out); // обработка данных из файла
int in_stream(queuenode_t *q);                                   // обработка данных из потока ввода

int strToStruct(char *str, input_t *in); // получение данных из строки вода

int trade(queuenode_t *q, input_t *in, FILE *fp_out);        // проверка обмена и обмен
int wtb(queuenode_t *q, input_t *inB, FILE *fp_out);         // покупка
int sort_min(queuenode_t *q, queuenode_t *qS);               // сортировка очереди для покупки
int wts(queuenode_t *q, input_t *inS, FILE *fp_out);         // продажа
int sort_max(queuenode_t *q, queuenode_t *qB);               // сортировка очереди дял продажи
int del_OID(queuenode_t *q, unsigned int OID, FILE *fp_out); // удаление из очереди по идентификатору

queuenode_t create_queue(void);            // иниицализация очереди
void enqueue(queuenode_t *q, input_t *in); // добавление элемента в конец очереди
void dequeue(queuenode_t *q, input_t *in); // получение элемента из начала очереди
int is_empty(queuenode_t *q);              // проверка на пустую очередь

#endif /* TRADE_H */
