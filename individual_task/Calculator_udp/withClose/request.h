#ifndef REQUEST_H
#define REQUEST_H


// get datagram with the help of user
// return: NULL if it is the last datagram for sending
// otherwise, return pointer to datagram for sending
void *getRequest(int *gramLen);

#endif

