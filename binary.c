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
    

    // Kutsua funktiota parse_str äsken vastaanotetulle riville
    struct numbers *numbr
    n = parse_str(recvline, numbr);
    if (n <= 0) {
        perror("parse error");
        return 1;
    }
    // Välitulostus tarkistuksen vuoksi
    output_str(char *str, size_t len, const struct numbers *n);

    // Lähettää parse_str-funktion luoman tietorakenteen 
    // takaisin serverille binäärimuodossa verkkotavujärjestyksessä
    // ja ilman tyhjää tilaa kenttien välillä


    // Lukee viisi etumerkitöntä binäärimuotoista numeroa serveriltä.
    // Nämä numerot ovat kooltaan 8, 32, 8, 16 ja 32 bittiä. 
    // Ohjelman tulisi lukea tasan 12 tavua.


    // Seuraavaksi ohjelma kutsuu output_str-funktiota antaen
    // sille äsken vastaanotettujen 12 tavun luoman numbers-
    // tietorakenteen.Lähetä tämä merkkijono takaisin palvelimelle.


    // Viimeisenä serveri vastaa OK tai FAIL

    
    return 0;
    // Sulkee pistokkeen
    close(sockfd);
}
