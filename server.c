/*
 * server.c
 * 5-server - Harjoitellaan TCP-palvelinohjelman toimintaa yksi asiakas kerrallaan
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
    char *viesti = "5-server\n";
    n = write(sockfd, viesti, strlen(viesti));
    if (n < 0) {
        perror("write2 error\n");
        return 1;
    }

    // Sinun tulee hyväksyä satunnainen määrä tulevia yhteyksiä. 
    // Aina kun tehtäväpalvelin lähettää ensimmäiseen pistokkeeseen “MORE” + rivinvaihto, 
    // luo palvelinpistoke joka kuuntelee määräämääsi porttia (ja osoitetta). 
    // Sitten lähetä tehtäväpalvelimelle takaisin viesti “SERV <IP-osoite> <portti>” + rivinvaihto. 
    // Nyt tehtäväpalvelin ottaa yhteyttä antamaasi osoitteeseen.

    // Kun tehtäväpalvelin on yhdistänyt palvelinpistokkeeseesi, 
    // se lähettää sinulle 32-bittisen luvun. 
    // Seuraavaksi tehtäväpalvelin odottaa, että lähetät tuon luvun verran 
    // tavuja takaisin palvelinpistokkeen kautta. 
    // Odota seuraavaa 32-bittistä lukua ja toista prosessi, 
    // kunnes tehtäväpalvelin sulkee palvelinpistokkeessa olevan yhteyden. 
    // Jos tämän jälkeen saat uuden “MORE” + rivinvaihto - viestin 
    // tehtäväpalvelimelta ensimmäiseen pistokkeeseen, 
    // tulisi sinun luoda uusi kuunteleva pistoke ja toimia kuten aiemmin.

    // Testi päättyy kun palvelin lähettää ensimmäiseen pistokkeeseen 
    // “OK” tai “FAIL” riippuen siitä, onnistuiko testi.
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
