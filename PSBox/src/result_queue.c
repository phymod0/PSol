


/*

	DESCRIPTION:
		Main queue for logging traffic information

*/



#ifndef RESULT_QUEUE_C
#define RESULT_QUEUE_C



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "result_queue.h"



result_queue result_queue_create() {

	result_queue rqueue;

	rqueue.n_results = 0;
	rqueue.head = NULL;
	rqueue.tail = NULL;

	return rqueue;

}

void result_queue_push(result_queue *rqueue, char *mac_addr, char *url, char *time) {

	result *res = (result*)malloc(sizeof(result));

	res->next		= NULL;
	res->mac_addr	= strdup(mac_addr);
	res->url		= strdup(url);
	res->time		= strdup(time);

	if (rqueue->n_results) {
		rqueue->tail->next = res;
		rqueue->tail = res;
	} else {
		rqueue->head = rqueue->tail = res;
	}

	++rqueue->n_results;

	return;

}

void result_queue_pop(result_queue *rqueue) {

	if (rqueue->n_results) {

		result *head = rqueue->head;

		free(head->mac_addr);
		free(head->url);
		free(head->time);

		rqueue->head = head->next;

		free(head);

		if (!--rqueue->n_results)
			rqueue->tail = NULL;

	}

	return;

}

result *result_queue_front(result_queue *rqueue) {

	return rqueue->head;

}

void result_queue_print(result_queue *rqueue) {

	result *tmp = rqueue->head;

	printf("Printing %d log records:\n", rqueue->n_results);

	while (tmp) {

		printf(
			"\t[%s] %s->%s\n",
			tmp->time, tmp->mac_addr, tmp->url
		);

		tmp = tmp->next;

	}

	printf("\n");

	return;

}

char *result_queue_str(result_queue *rqueue) {

	result *tmp = rqueue->head;
	char buf[1024], *ret;
	int final_len = 0;

	while (tmp) {

		final_len += sprintf(
			buf, "%s:%s:%s\n",
			tmp->time, tmp->mac_addr, tmp->url
		);

		tmp = tmp->next;

		++final_len;

	}

	ret = (char*)calloc(final_len, sizeof(char));
	tmp = rqueue->head;

	while (tmp) {

		sprintf(
			buf, "%s:%s:%s\n",
			tmp->time, tmp->mac_addr, tmp->url
		);

		strcat(ret, buf);

		tmp = tmp->next;

		if (tmp)
			strcat(ret, "|");

	}

	return ret;

}

void result_queue_destroy(result_queue *rqueue) {

	result *tmp = rqueue->head;

	while (tmp) {

		result *next = tmp->next;

		free(tmp->mac_addr);
		free(tmp->url);
		free(tmp->time);
		free(tmp);

		tmp = next;

	}

	rqueue->n_results = 0;
	rqueue->head = NULL;
	rqueue->tail = NULL;

	return;

}



#endif /* RESULT_QUEUE_C */



