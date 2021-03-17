/*
 * large.c
 * 3-large, suurempien tietomäärien käsittely pistokkeessa
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
    char *viesti = "3-large\n";
    n = write(sockfd, viesti, strlen(viesti));
    if (n < 0) {
        perror("write2 error\n");
        return 1;
    }

    // Tämän jälkeen palvelin alkaa lähettää sinulle tietolohkoja.
    while (1) {
        // Kukin tietolohko alkaa 32-bittisellä etumerkittömällä (unsigned) 
        // kokonaisluvulla, joka kertoo kuinka monta tavua tietolohko 
        // sisältää (muista verkon tavujärjestys!).
        uint32_t item32 = 0;
        n = read(sockfd, &item32, sizeof(uint32_t));
        if (n < 0) {
            perror("read item32 error\n");
            return 1;
        }    
        uint32_t result = ntohl(item32);
        printf("Tavujen määrä: %d\n", result);
        if (result == 0) {
            printf("Nyt tulos on nolla: %d\n", result);
            // Kun saat takaisin luvun nolla, lähetä takaisin luku 0 
            uint32_t *buf, it, item32;
            item32 = 0;
            buf = &it;
            it = htonl(item32);
            n = write(sockfd, &it, sizeof(uint32_t));
            if (n < 0) {
                perror("write last 0 error\n");
                return 1;
            }         
            break;
        }

        // Tämän jälkeen seuraa kyseinen tavumäärä, 
        // joka sinun pitää lukea pistokkeesta.
        char *returndata = malloc(result * sizeof(char));
        //returndata = malloc(sizeof(char)*result);
        // char returndata[result+1];
        uint32_t i = 0;
        while (i<result) {
            n = read(sockfd, returndata, sizeof(char)*(result));
            printf("n tavuluvun jälkeen: %d\n", n);
            i += n;
            //if (fputs(returndata, stdout) == EOF) {
            //    fprintf(stderr, "fputs error\n");
            //    return 1;
            //}
        }
        if (n < 0) {
            perror("read error");
            return 1;
        }
        free(returndata);
        returndata = NULL;

        // Kun olet lukenut koko lohkon, lähetä takaisin 
        // palvelimelle lukemiesi tavujen määrä (eli lohkon koko), 
        // jälleen 32-bittisenä kokonaislukuna, verkkotavujärjestyksessä.
        uint32_t *buf, it;
        buf = &it;
        it = htonl(result);
        n = write(sockfd, &it, sizeof(uint32_t));
        if (n < 0) {
            perror("write it error\n");
            return 1;
        }  
    }

    // Viimeisenä serveri vastaa OK tai FAIL
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
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
   
    // Sulkee pistokkeen
    close(sockfd);
}
