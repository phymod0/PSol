


/*

	DESCRIPTION:
		DNS payload parsing functions

*/



#ifndef DNS_C
#define DNS_C



#include "dns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>



static unsigned char *get_url(unsigned char *query_ptr) {

	int url_sz;
	unsigned char *url;

	// Get URL length
	url_sz = 0;
	while (query_ptr[url_sz]) {
		int label_sz = (unsigned int)query_ptr[url_sz];
		url_sz += label_sz + 1;
	}

	url = (char*)malloc(url_sz*sizeof(char));

	// Copy URL string
	url_sz = 0;
	while (query_ptr[url_sz]) {
		int label_sz = (int)query_ptr[url_sz];
		memcpy(
			url+url_sz,
			// Exclude the size byte
			query_ptr+url_sz+1,
			label_sz
		);
		url_sz += label_sz + 1;
		url[url_sz-1] = '.';
	}
	url[url_sz-1] = '\0';

	return url;

}

static int get_query_len(unsigned char *query_ptr) {

	int query_sz = 0;

	// URL field length
	while (query_ptr[query_sz]) {
		int label_sz = (int)query_ptr[query_sz];
		query_sz += label_sz + 1;
	}

	++query_sz; /* Terminating zero byte of URL */

	query_sz += sizeof(q_ftr);

	return query_sz;

}

/*

	@payload:
		DNS data encapsulated by UDP

	Returns:
		List of URLs queried

*/
query_list extract_urls(unsigned char *payload) {

	int n_queries, i;
	unsigned char *query_ptr;
	dnshdr *dns_hdr;
	query_list qlist;

	dns_hdr = (dnshdr*)payload;

	n_queries = qlist.n_queries = ntohs(dns_hdr->n_questions);

	qlist.urls = (char**)malloc(n_queries*sizeof(char*));

	query_ptr = payload + sizeof(dnshdr);
	for (i = 0; i < n_queries; i++) {
		qlist.urls[i] = get_url(query_ptr);
		query_ptr += get_query_len(query_ptr);
	}

	return qlist;

}

/*

	@qlist:
		List of URLs to be freed

*/
void free_urls(query_list qlist) {

	int i;

	for (i = 0; i < qlist.n_queries; i++)
		free(qlist.urls[i]);

	free(qlist.urls);

	return;

}

/*

	@qlist:
		List of URLs to be printed

*/
void print_urls(query_list qlist) {

	int i;

	printf("Printing %d queries:\n", qlist.n_queries);

	for (i = 0; i < qlist.n_queries; i++)
		printf("\tQuery %d: %s\n", i+1, qlist.urls[i]);

	return;

}



#endif /* DNS_C */



