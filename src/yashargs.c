/* Placed in the public domain 2009 by Sam Trenholme */

/* Yash: A usable command shell in under 4k (yet another shell)
 *
 * This is a simple thought experiment: How small can we make a *NIX command
 * line shell?  This is as simple as it gets; it will run a command typed
 * in if it's an external command like "ls" or "cc", with arguments separated
 * by space; if one types in "-cd {directory}", it will go to that directory.
 *
 * CNTL-C exits the shell
 *
 * BUGS: CNTL-D reruns the previous command instead of exiting the shell.
 */

#include "yashargs.h"
#include <unistd.h>
#include <stdlib.h>

/* Given a pointer to arguments, destroy the string */
void yash_zap_args(char **zap) {
    int a = 0;

    if(zap == 0) {
        return;
    }

    for(a = 0; a < ARGMAX; a++) {
        if(zap[a] != 0) {
            free(zap[a]);
            zap[a] = 0;
        }
    }

    free(zap);
}

/* Given a line separated by whitespace, return an array of strings
 *  * of the individual arguments */

char **yash_args(char *in) {
    char **out = 0;
    int a = 0, b = 0;

    /* Sanity checks */
    if(in == 0) {
        return 0;
    }

    out = malloc(ARGMAX * sizeof(char *));
    for(a = 0; a < ARGMAX; a++) {
        out[a] = 0;
    }
    a = 0;

    if(out == 0) {
        return 0;
    }

    while(*in != 0) {
        if(*in != ' ' && *in != '\n') {
            if((out[a] = malloc(LINEMAX)) == 0) {
                goto catch_yash_args;
            }
            b = 0;
            while(b < LINEMAX - 2 && *in != ' ' && *in != 0 &&
                    *in != '\n') {
                out[a][b] = *in;
                b++;
                in++;
            }
            out[a][b] = 0;
            a++;
            if(a >= ARGMAX) {
                goto catch_yash_args;
            }
        }
        in++;
    }   

    return out;

catch_yash_args:
    yash_zap_args(out);
    return 0;
}
