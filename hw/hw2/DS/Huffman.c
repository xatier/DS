#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "heap.h"

void buildtree (const char *filename, const char *txt) {

    FILE *fp = fopen(filename, "rb");
    FILE *fout = fopen(txt, "w");

    if (fp == NULL) {
        fprintf(stderr, "no such file :%s !", filename);
        exit(1);
    }
    if (fout == NULL) {
        fprintf(stderr, "Can't write file: %s!", txt);
        exit(1);
    }

    // read the image file bytes by bytes
    // symbol[] for 0~255 hex number
    // output record[] 
    // bytes counter, i.e. file size
    unsigned char c;
    int sym[256];
    struct record rec[256];
    memset(sym, 0, sizeof(sym));
    unsigned long long all = 0;

    
    // read in the whole file and record the symbols' frequency
    // and count the file size
    while (fread(&c, sizeof(unsigned char), 1, fp) != 0) {
        sym[c]++; all++;
    }


    // using a heap to manage the symbol frequency
    struct heap *h = init_heap();
    mk_heap(h, sym, 256);

    // remove root twice and insert the merged node to the heap
    // till there's only a root node in the heap
    while (h->nodes > 1) {
        struct node *a, *b;
        a = rm_root(h);
        b = rm_root(h);
        insert_end(h, new_node(a->weight + b->weight, -1, a, b));
    }

    // traverse the heap and set huffman code for each node
    set_node_code(h->arr[0], 0, rec, 0);

    // output the records to the file
    for (int i = 0; i < 256; i++) {
        rec[i].prob = (double)rec[i].weight / all;
        fprintf(fout, "%d %10f %s\n", rec[i].index, rec[i].prob, rec[i].str);
    }


    // destructure
    del_heap(h);

    for (int i = 0; i < 256; i++)
        free(rec[i].str);
    fclose(fp);
    fclose(fout);
}


void encode (const char *txt, const char *img, const char *huf_filename) {

    FILE *fp = fopen(txt, "r");
    FILE *fin = fopen(img, "rb");
    FILE *fout = fopen(huf_filename, "wb");

    if (fp == NULL) {
        fprintf(stderr, "Can't open file :%s !", txt);
        exit(1);
    }
    if (fin == NULL) {
        fprintf(stderr, "Can't open file :%s !", img);
        exit(1);
    }
    if (fout == NULL) {
        fprintf(stderr, "Can't write file :%s !", huf_filename);
        exit(1);
    }

    // read buffer
    // data record from the txt
    char buf[100];
    struct record rec[256];

    // read the txt file and store the data
    for (int i = 0; i < 256; i++) {
        fgets(buf, 99, fp);
        rec[i].str = (char *)malloc(30);
        sscanf(buf, "%d%lf%s", &rec[i].index, &rec[i].prob, rec[i].str);
    }

    // read the image file byte by byte
    // write buffer, flush a byte once
    // digit count, 0 to 7
    unsigned char c;
    unsigned char write_buf = 0;
    char digit = 0;
    int b1, b2;
    b1 = b2 = 0;

    while (fread(&c, sizeof(unsigned char), 1, fin) != 0) {
        b1++;
        for (int i = 0; rec[c].str[i] != '\0'; i++) {
            write_buf = (write_buf << 1) | (rec[c].str[i] & 1);
            digit++;
            if (digit == (sizeof(unsigned char) << 3)) {
                fwrite(&write_buf, sizeof(unsigned char), 1, fout);
                digit = 0;
                write_buf = 0;
                b2++;
            }
        }
    }

    printf("compression ratio: (%s/%s): %.5f\n", huf_filename, img, (double)b2/b1);

    // release resources
    for (int i = 0; i < 256; i++)
        free(rec[i].str);

    fclose(fp);
    fclose(fin);
    fclose(fout);
}


void decode (const char *txt, const char *huf_filename, const char *decode_filename) {

    FILE *fp = fopen(txt, "r");
    FILE *fin = fopen(huf_filename, "rb");
    FILE *fout = fopen(decode_filename, "wb");

    if (fp == NULL) {
        fprintf(stderr, "Can't open file :%s !", txt);
        exit(1);
    }
    if (fin == NULL) {
        fprintf(stderr, "Can't open file :%s !", huf_filename);
        exit(1);
    }
    if (fout == NULL) {
        fprintf(stderr, "Can't write file :%s !", decode_filename);
        exit(1);
    }

    // read buffer
    // data record from the txt file
    // build the huffman tree
    // the iterator of the tree
    char buf[100];
    struct record rec[256];
    struct node *root = new_node(0, -1, NULL, NULL);
    struct node *go = root;

    // read in 256 lines
    for (int i = 0; i < 256; i++) {
        fgets(buf, 99, fp);
        rec[i].str = (char *)malloc(30);
        sscanf(buf, "%d%lf%s", &rec[i].index, &rec[i].prob, rec[i].str);

        // traverse the huffman tree
        go = root;
        for (int j = 0; j <= (int)strlen(rec[i].str); j++) {
            // allocate new node  if i reach leaves
            if (rec[i].str[j] == '0') {
                if (go->l == NULL)
                    go->l = new_node(0, -1, NULL, NULL);
                go = go->l;
            }
            else if (rec[i].str[j] == '1') {
                if (go->r == NULL)
                    go->r = new_node(0, -1, NULL, NULL);
                go = go->r;
            }
            // record leaf node's index
            else {
                go->index = rec[i].index;
            }
        }
    }


    // read buffer
    // write buffer
    // timestamp
    unsigned char c;
    unsigned char cc;
    clock_t t1 = clock();

    // from root
    go = root;
    while (fread(&c, sizeof(unsigned char), 1, fin) != 0) {
        for (int i = 0; i < 8; i++) {
            // this is a leaf node
            if (go->index != -1) {
                if (go->l == NULL && go->r == NULL) {
                    cc = go->index;
                    fwrite(&cc, sizeof(unsigned char), 1, fout);
                    // go back
                    go = root;
                    cc = 0;
                    i--;
                }
            }
            else {
                if ((c >> (7-i)) & 1) {
                    go = go->r;
                }
                else {
                    go = go->l;
                }
            }
        }
    }

    fwrite(&cc, sizeof(unsigned char), 1, fout);
    printf("decode time of %s -> %s: %.6f (s)\n", huf_filename, decode_filename, (clock()- t1) / (double)CLOCKS_PER_SEC);

    // release resources
    for (int i = 0; i < 256; i++)
        free(rec[i].str);

    del_node(root);
    fclose(fp);
    fclose(fin);
    fclose(fout);
}


int main (void) {
    char filename[100];

    sprintf(filename, "%s", "Input_Image1.bmp");
    buildtree(filename, "HufT1.txt");
    encode("HufT1.txt", filename, "Output_Image1.huf");
    decode("HufT1.txt", "Output_Image1.huf", "Decode_Image1.bmp");

    sprintf(filename, "%s", "Input_Image2.bmp");
    buildtree(filename, "HufT2.txt");
    encode("HufT2.txt", filename, "Output_Image2.huf");
    decode("HufT2.txt", "Output_Image2.huf", "Decode_Image2.bmp");

    
    
    printf("TA: plz give me your image file name: ");
    scanf("%s", filename);
    getchar();
    buildtree(filename, "HufT.txt");
    encode("HufT.txt", filename, "Output.huf");
    decode("HufT.txt", "Output.huf", "Output.bmp");
    
    puts("done!");
    
    
    
    return 0;
}
