


#ifndef RESULT_QUEUE_H
#define RESULT_QUEUE_H



typedef struct result {

	struct result *next;

	char *mac_addr;
	char *url;
	char *time;

} result;

typedef struct {

	int n_results;
	result *head;
	result *tail;

} result_queue;



result_queue result_queue_create();
void result_queue_push(result_queue *rqueue, char *mac_addr, char *url, char *time);
void result_queue_pop(result_queue *rqueue);
result *result_queue_front(result_queue *rqueue);
void result_queue_print(result_queue *rqueue);
void result_queue_destroy(result_queue *rqueue);
char *result_queue_str(result_queue *rqueue);



#endif /* RESULT_QUEUE_H */



