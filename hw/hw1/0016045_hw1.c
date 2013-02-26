/* *********************************************************
 *                                                         *
 * To compile this program, please use C99 (gcc -std=c99)  *
 *                                                         *
 * ******************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 5000
#define MAX_WORD_LENGTH 100
#define MAX_HASH_MAMBERS 100000
#define LINK_LENGTH 50

#define MALLOC_CHK(PTR, MSG) do {                           \
    if (PTR == NULL) {                                      \
            fprintf(stderr, "Molloc error: %s! :(!", MSG);  \
            exit(1);                                        \
    }                                                       \
} while (0)



// structure pair of each elements in the hash
// to record the string and its freqency
struct pair {
    char *key;
    int value;
};


// hash
struct hash {
    struct pair *pool[MAX_HASH_MAMBERS];
    int members;
    int record_num;
};


struct hash words;


// token to split words in a line
int _get_token_ptr;

// record which index in the pool is used
int pool_record[MAX_HASH_MAMBERS] = {0};


// hash constructure
void init_hash (void) {
    words.members = 0;
    words.record_num = 0;
    for (int i = 0; i < MAX_HASH_MAMBERS; i++)
        words.pool[i] = NULL;
}


// hash destructure
void del_hash (void) {
    for (int i = 0; i < MAX_HASH_MAMBERS; i++) {
        if (words.pool[i]) {
            for (int j = 0; j < LINK_LENGTH; j++) {
                // XXX debug info.
                //if (words.pool[i][j].key)
                //    printf("delete (%s => %d), \n", words.pool[i][j].key, words.pool[i][j].value);
                free(words.pool[i][j].key);
            }
            free(words.pool[i]);
        }
    }
}


// generate a hash number for index from a string
// _index = hash(string)
int _index (const char *str) {
    long long ans = 0;
    int ascii = 0;
    int len = strlen(str);
    
    for (int i = 0; i < len ; i++) {
        if ('0' <= str[i] && str[i] <= '9')
            ascii = str[i] - '0' + 26;
        else if ('a' <= str[i] && str[i] <= 'z')
            ascii = str[i] - 'a';
        else if ('A' <= str[i] && str[i] <= 'Z')        // never happend
            ascii = str[i] - 'A';
        else {}                                         // ignore other characters
        ans += i << (ascii >> 1);
    }
    return ans * 2 % MAX_HASH_MAMBERS;
}


// push an element in the hash
void push_hash (const char *str) {
    int idx = _index(str);
    words.members++;
    pool_record[words.record_num] = idx;

    // this record in the pool is not used
    if (words.pool[idx] == NULL) {
        words.record_num++;

        words.pool[idx] = (struct pair *)malloc(sizeof(struct pair)  * LINK_LENGTH);
        MALLOC_CHK(words.pool[idx], "malloc for hash elements");

        words.pool[idx][0].key = (char *)malloc(sizeof(char) * MAX_WORD_LENGTH);
        MALLOC_CHK(words.pool[idx][0].key, "malloc for keys");

        sprintf(words.pool[idx][0].key, "%s", str);
        words.pool[idx][0].value = 1;

        // initialize the linear link
        for (int i = 1; i < LINK_LENGTH; i++) {
            words.pool[idx][i].key = NULL;
            words.pool[idx][i].value = 0;
        }
    }
    else {
        // collision case of the _index()
        int i = 0;
        for (; words.pool[idx][i].key && i <= LINK_LENGTH; i++);
        if (i == LINK_LENGTH) {      // space error! :(
            fprintf(stderr, "space error! :(!");
            exit(1);
        }
        words.pool[idx][i].key = (char *)malloc(sizeof(char) * MAX_WORD_LENGTH);
        MALLOC_CHK(words.pool[idx][i].key, "malloc for hash elements");

        sprintf(words.pool[idx][i].key, "%s", str);
        words.pool[idx][i].value = 1;
    }
}


// fine the element in the hash
struct pair *find_pair (const char *str) {
    int idx = _index(str);
    if (words.pool[idx])
        for (int i = 0; i < LINK_LENGTH && words.pool[idx][i].key; i++)
            if (!strcmp(str, words.pool[idx][i].key)) {
                return &(words.pool[idx][i]);
            }
                
    return NULL;
}


// get all elements in the hash
struct pair *get_all (void) {
    struct pair *p, *head = (struct pair *)malloc(sizeof(struct pair) * (words.members + 10));
    MALLOC_CHK(head, "malloc for sorting temperate space");
    int c = 0;
    p = head;
    for (int i = 0; i < words.record_num; i++) {
        int idx = pool_record[i];
        for (int j = 0; j < LINK_LENGTH && words.pool[idx][j].key; j++) {
            p->key = words.pool[idx][j].key;
            p->value = words.pool[idx][j].value;
            p++;
            c++;
        }
    }
    return head;
}


// split tokens in a line
char *get_token (const char *str) {
    char *tmp = NULL;
    int i = 0;
    int c;
next:
    tmp = (char *)malloc(sizeof(char) * MAX_WORD_LENGTH);
    MALLOC_CHK(tmp, "tamp space for split tokens");

    i = 0;

    while ((c = str[_get_token_ptr++])) {
        if (('0' <= c && c <= '9') ||
            ('A' <= c && c <= 'Z') ||
            ('a' <= c && c <= 'z')) {
            tmp[i++] = tolower(c);      // ignore case
        }
        else break;
    }
    if (i == 0) {
        free(tmp);
        while ((c = str[_get_token_ptr++])) {
            if (('0' <= c && c <= '9') ||
                ('A' <= c && c <= 'Z') ||
                ('a' <= c && c <= 'z')) {
                _get_token_ptr--;
                break;
            }
        }
        if (str[_get_token_ptr - 1] == '\0')
            return NULL;
        goto next;
    }
    tmp[i] = '\0';
    return tmp;
}


// compare functoin for qsort
int cmp (const void *a, const void *b) {
    struct pair *c = (struct pair *)a;
    struct pair *d = (struct pair *)b;
    if (c->value < d->value)
        return 1;
    else if (c->value == d->value)
        return 0;
    else
        return -1;
}


int main (int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "invalid arguments");
        exit(1);
    }
    char *tmp;
    // threshold of the output
    int threshold;

    // record all lines!
    char *article[MAX_LINES];
    int lines = 0;

    sscanf(argv[2], "%d", &threshold);

    FILE *IN = fopen(argv[1], "r");
    lines = 0;

    tmp = (char *)malloc(sizeof(char) * MAX_LINE_LENGTH);
    MALLOC_CHK(tmp, "tamp space for getline");

    while (fgets(tmp, MAX_LINE_LENGTH - 10, IN)) {
        article[lines++] = tmp;
        tmp = (char *)malloc(sizeof(char) * MAX_LINE_LENGTH);
        MALLOC_CHK(tmp, "tamp space for getline");
    }

    struct timeval start, end;
    long mtime, seconds, useconds;

    init_hash();
    gettimeofday(&start, NULL);

    for (int i = 0; i < lines; i++) {
        char *c;
        struct pair *p;
        _get_token_ptr = 0;
        while ((c = get_token(article[i]))) {
            if ((p = find_pair(c))) {
                p->value++;
            }
            else {
                push_hash(c);
            }
            free(c);
        }
    }


    for (int i = 0; i < lines; i++)
        free(article[i]);

    struct pair *head = get_all();
    qsort(head, words.members, sizeof(struct pair), cmp);
    int rank = head[0].value;
    

    for (int i = 0; i < words.members; i++) {
        if (rank != head[i].value) {
            threshold--;
            rank = head[i].value;
            if (!threshold)
                break;
        }
        printf("(%s,%d)\n", head[i].key, head[i].value);
    }
    free(head);


    del_hash();

    gettimeofday(&end, NULL);

    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0);

    printf("time: %.03f(s)\n", mtime / 1000.0);

    return 0;
}
