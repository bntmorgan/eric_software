#ifndef __CHALLENGE_H
#define __CHALLENGE_H

int challenge_dl(char *name, unsigned char *mac, unsigned char *lip, unsigned
    char *rip);
void challenge_init(void);
void challenge_run(void);

#endif /* __CHALLENGE_H */
