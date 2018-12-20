#ifndef INPUT_H
#define INPUT_H

// return: 1 if the user typed number;
//         0 if the user typed not a number;
//         -1 on error
int inputDouble(const char *prompt, double *val);

// return: -1 on error
//         0 on success
int tryInputDouble(const char *prompt, double *val);

#endif

