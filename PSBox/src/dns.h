


#ifndef DNS_H
#define DNS_H



#include <sys/types.h>



typedef struct {

	int n_queries;
	char **urls;

} query_list;

typedef struct __attribute__((__packed__)) {

	u_int16_t t_id;
	u_int16_t flags;
	u_int16_t n_questions;
	u_int16_t n_answer_rrs;
	u_int16_t n_authority_rrs;
	u_int16_t n_additional_rrs;

} dnshdr;

typedef struct __attribute__((__packed__)) {

	u_int16_t q_type;
	u_int16_t q_class;

} q_ftr;



query_list extract_urls(unsigned char *payload);
void print_urls(query_list qlist);
void free_urls(query_list qlist);



#endif /* DNS_H */



