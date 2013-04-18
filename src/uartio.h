#ifndef UARTIO_H
#define UARTIO_H
void print(char *s);
int getuartchar(/*@out@*/char *storeLocation) /*@modifies *storeLocation@*/;
void printhex(unsigned int h);
#endif /* UARTIO_H */
