/*
 * binary.c
 * 2-binary, binääridatan käsittely 
 * Sari Vesiluoma */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "template.h"

#define MAXLINE 80

int main(int argc, char **argv)
{
    int sockfd, n;
    char recvline[MAXLINE + 1];  // Merkkipuskuri, johon luetaan tietoa
    struct sockaddr_in servaddr; //tietorakenne, joka esittää osoitetta

    // Ottaa TCP-yhteyden IP-osoitteeseen 130.233.154.208, porttiin 5000.
    const char *address = "130.233.154.208";
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    } 
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5000);
    if (inet_pton(AF_INET, address, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for %s/n", address);
        return 1;
    }
    if (connect(sockfd,
                (struct sockaddr *) &servaddr,
                sizeof(servaddr)) < 0) {
                    perror("connect error");
                    return 1;
    }

    // Lähettää pistokkeeseen oman opiskelijanumerosi ASCII-muodossa ja sen perään rivinvaihtomerkin
    char *opnro = "763088\n";
    n = write(sockfd, opnro, strlen(opnro));
    if (n < 0) {
        perror("write1 error\n");
        return 1;
    }

    // Lähettää pistokkeeseen tehtävän tunnisteen ja perään rivinvaihdon
    char *viesti = "2-binary\n";
    n = write(sockfd, viesti, strlen(viesti));
    if (n < 0) {
        perror("write2 error\n");
        return 1;
    }

    // Lukea rivi, minkä serveri lähettää. Rivi loppuu aina rivinvaihtomerkkiin
    printf("-1\n");
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        printf("n read = %d\n", n);
        if (fputs(recvline, stdout) == EOF) {
            fprintf(stderr, "fputs error\n");
            return 1;
        }
        printf("\n");
        if(strchr(recvline, '\n') != NULL) {
            break;
        }
    }
    printf("0\n");
    if (n < 0) {
        perror("read error");
        return 1;
    }
    strtok (recvline,"\n");
    printf("vastaanotettu merkkijono: %s***\n", recvline);

    // Kutsua funktiota parse_str äsken vastaanotetulle riville
    printf("1\n");
    struct numbers *numbr, numbrs;
    numbr = &numbrs;
    n = parse_str(recvline, numbr);
    if (n <= 0) {
        perror("parse error");
        return 1;
    }

    // Lähettää parse_str-funktion luoman tietorakenteen 
    // takaisin serverille binäärimuodossa verkkotavujärjestyksessä
    // ja ilman tyhjää tilaa kenttien välillä
    printf("2\n");
    // uint8_t a;
    uint8_t *buf_a, aa;
    buf_a = &aa;
    aa = numbr->a;
    n = write(sockfd, &aa, sizeof(aa));
    if (n < 0) {
        perror("write aa error\n");
        return 1;
    }    
	// uint32_t b;
    uint32_t *buf_b, bb;
    buf_b = &bb;
    bb = htonl(numbr->b);
    n = write(sockfd, &bb, sizeof(bb));
    if (n < 0) {
        perror("write bb error\n");
        return 1;
    }    
	// uint8_t c;
    uint8_t *buf_c, cc;
    buf_c = &cc;
    cc = numbr->c;
    n = write(sockfd, &cc, sizeof(cc));
    if (n < 0) {
        perror("write cc error\n");
        return 1;
    }    
	// uint16_t d;
    uint16_t *buf_d, dd;
    buf_d = &dd;
    dd = htons(numbr->d);
    n = write(sockfd, &dd, sizeof(dd));
    if (n < 0) {
        perror("write dd error\n");
        return 1;
    }    	
    // uint32_t e;
    uint32_t *buf_e, ee;
    buf_e = &ee;
    ee = htonl(numbr->e);
    n = write(sockfd, &ee, sizeof(ee));
    if (n < 0) {
        perror("write ee error\n");
        return 1;
    }    
    printf("3\n");

    // Lukee viisi etumerkitöntä binäärimuotoista numeroa serveriltä.
    // Nämä numerot ovat kooltaan 8, 32, 8, 16 ja 32 bittiä. 
    // Ohjelman tulisi lukea tasan 12 tavua.
	// uint8_t a;
    uint8_t item8 = 0;
    n = read(sockfd, &item8, sizeof(1));
    if (n < 0) {
        perror("read aa error\n");
        return 1;
    }    
    numbr->a = item8;
	// uint32_t b;
    uint32_t item32 = 0;
    n = read(sockfd, &item32, sizeof(4));
    if (n < 0) {
        perror("read bb error\n");
        return 1;
    }    
    numbr->b = ntohl(item32);
	// uint8_t c;
    n = read(sockfd, &item8, sizeof(1));
    if (n < 0) {
        perror("read cc error\n");
        return 1;
    }    
    numbr->c = item8;
	// uint16_t d;
    uint16_t item16 = 0;
    n = read(sockfd, &item16, sizeof(2));
    if (n < 0) {
        perror("read dd error\n");
        return 1;
    }    
    numbr->d = ntohs(item16);
	// uint32_t e;
    n = read(sockfd, &item32, sizeof(4));
    if (n < 0) {
        perror("read ee error\n");
        return 1;
    }    
    numbr->e = ntohl(item32);

    printf("4\n");

    // Seuraavaksi ohjelma kutsuu output_str-funktiota antaen
    // sille äsken vastaanotettujen 12 tavun luoman numbers-
    // tietorakenteen.Lähetä tämä merkkijono takaisin palvelimelle.
    printf("Saadut numerot ovat: %d, %d, %d, %d, %d\n", numbr->a, numbr->b, numbr->c, numbr->d, numbr->e);
    char viestistr[100];
    output_str(viestistr, sizeof(viestistr), numbr);
    strcat(viestistr, "\n");
    printf("viestistr on: %s", viestistr);
    //char *viesti2 = viestistr;
    n = write(sockfd, &viestistr, strlen(viestistr));
    printf("Viimeisen lähetyksen jälkeen n on %d\n", n);
    if (n < 0) {
        perror("write viimeinen lähetys error\n");
        return 1;
    }

    printf("5\n");

    // Viimeisenä serveri vastaa OK tai FAIL
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        if (fputs(recvline, stdout) == EOF) {
            fprintf(stderr, "fputs error\n");
            return 1;
        }
    }
    if (n < 0) {
        perror("read error");
        return 1;
    }
    return 0;

     printf("\n6\n");
   
    // Sulkee pistokkeen
    close(sockfd);
}
