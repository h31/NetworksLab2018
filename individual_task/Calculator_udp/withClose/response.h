#ifndef RESPONSE_H
#define RESPONSE_H

// make datagram with server-response from datagram with client-request
// return: NULL on error or pointer to autoallocated response if success
void *makeResponse(const void *request, int requestLen, int *responseLen);

#endif
