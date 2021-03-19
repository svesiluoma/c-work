/*
 * names.c
 * 4-names - DNS-hakuja ja rinnakkaisten yhteyksien ylläpitämistä
 * Sari Vesiluoma */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "template.h"
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>

#define MAXLINE 80

// Apufunktio osoitteen tulostamiseen
void print_address(const char *prefix, const struct addrinfo *res)
{
        char outbuf[80];
        struct sockaddr_in *sin = (struct sockaddr_in *)res->ai_addr;
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)res->ai_addr;
        void *address;

        if (res->ai_family == AF_INET)
                address = &(sin->sin_addr);
        else if (res->ai_family == AF_INET6)
                address = &(sin6->sin6_addr);
        else {
                printf("Unknown address\n");
                return;
        }

        const char *ret = inet_ntop(res->ai_family, address, outbuf, sizeof(outbuf));
        printf("%s %s\n", prefix, ret);
}

// host: kohdesolmun nimi (tai IP-osoite tekstimuodossa)
// serv: kohdepalvelun nimi (tai porttinumero tekstimuodossa)
int tcp_connect(const char *host, const char *serv)
{
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;

    // Ensiksi kerrotaan hints-rakenteessa, että osoiteperhettä ei ole
    // rajoitettu, eli sekä IPv4 ja IPv6 - osoitteet kelpaavat.
    // Lisäksi sanomme, että olemme pelkästään kiinnostuneita TCP-yhteyksistä
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Tehdään nimikysely käyttäen ylläolevaa hints-rakennetta
    // Funktio varaa vastaukselle tilan itse, osoitin palautuu res-muuttujaan
    if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) {
            fprintf(stderr, "tcp_connect error for %s, %s: %s\n",
                    host, serv, gai_strerror(n));
            return -1;
    }
    ressave = res; // so that we can release the memory afterwards

    // res-rakenne osoittaa linkitettyyn listaan. Käydään läpi linkitettyä
    // listaa yksi kerrallaan ja yritetään yhdistää saatuun osoitteeseen.
    // res-rakenne sisältää kaikki parameterit mitä tarvitsemme socket-
    // ja connect - kutsuissa.
    do {
           sockfd = socket(res->ai_family, res->ai_socktype,
                            res->ai_protocol);
            if (sockfd < 0)
                    continue;       /* ignore this one */

            print_address("Trying to connect", res);

            // Mikäli yhteys onnistuu, silmukka keskeytetään välittömästi,
            // koska meillä on toimiva yhteys, eikä loppuja osoitteita
            // tarvitse kokeilla
            if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
                  break;          /* success */

            printf("connect failed\n");

            close(sockfd);  /* ignore this one */
    } while ( (res = res->ai_next) != NULL);

    // Päästiinkö linkitetyn listan loppuun, mutta yhteys ei onnistunut?
    // ==> virhe
    if (res == NULL) {      /* errno set from final connect() */
            fprintf(stderr, "tcp_connect error for %s, %s\n", host, serv);
            sockfd = -1;
    } else {
            print_address("We are using address", res);
    }

    // Järjestelmä on varannut muistin linkitetylle listalle, se pitää vapauttaa
    freeaddrinfo(ressave);

    return sockfd;
}


int main(int argc, char **argv)
{
    int sockfd, sockfd2, n;
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
    char *viesti = "4-names\n";
    n = write(sockfd, viesti, strlen(viesti));
    if (n < 0) {
        perror("write2 error\n");
        return 1;
    }

    // Tämän jälkeen palvelin lähettää komennon merkkijonona, 
    // jossa on DNS-nimi ja portti tässä formaatissa: CONN <DNS-nimi> <portti>. 
    // Merkkijono loppuu rivinvaihtomerkkiin. 
    // Esimerkiksi: CONN www.google.fi 80.
    printf("Luetaan palvelimelta tuleva rivi\n");
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        if (fputs(recvline, stdout) == EOF) {
            fprintf(stderr, "fputs error\n");
            return 1;
        }
        if(strchr(recvline, '\n') != NULL) {
            break;
        }
    }
    if (n < 0) {
        perror("read error");
        return 1;
    }
    strtok (recvline,"\n");

    // Luetaan osoite ja portti
    char got_address[100];
    char got_port[10];
    sscanf(recvline, "CONN %s %s", got_address, got_port);
    printf("Osoite on %s ja portti on %s\n", got_address, got_port);

    // Tämän jälkeen sinun tulisi tehdä DNS-kysely 
    // ja ottaa yhteys nimeä vastaavaan palvelimeen annettuun porttiin, 
    // sulkematta kuitenkaan ensimmäistä pistoketta. 
    // Luo siis toinen pistoke (pistoke-B), josta otat yhteyden.
    int fd = tcp_connect(got_address, got_port);
    if (fd >= 0) {
        printf("success!\n");
    } else {
        perror("fail: \n");
    }

    // Kun olet avannut yhteyden nimeä vastaavaan osoitteeseen, 
    // lähetä oma IP-osoitteesi ja käyttämäsi portti 
    struct sockaddr_in own;
    socklen_t ownlen = sizeof(struct sockaddr_in);
    if (getsockname(fd, (struct sockaddr *)&own, &ownlen) < 0) {
        perror("getsockname error: ");
    }
    char outbuf[80];
    inet_ntop(AF_INET, &(own.sin_addr), outbuf, sizeof(outbuf));
    // char ownaddress = outbuf;
    uint16_t ownport = ntohs(own.sin_port);
    printf("own address: %s  port: %d\n", outbuf, ownport);
    // printf("own address: %s  port: %d\n", ownaddress, ownport);
    // (jota käytät tässä toisessa pistokkeessa) pistoke-B:hen 
    // palvelimelle formaatissa: ADDR <IP-osoite> <portti> <Opiskelijanro>, 
    // jonka perään tulee rivinvaihtomerkki.  
    //Tämän jälkeen pistoke-B:n palvelin sulkee yhteyden.
    
    char *eka = "ADDR ";
    n = write(fd, eka, strlen(eka));
    if (n < 0) {
        perror("write1 error\n");
        return 1;
    }
    char *osoite = outbuf;
    n = write(fd, osoite, strlen(osoite));
    if (n < 0) {
        perror("write2 error\n");
        return 1;
    }
    n = write(fd, " ", sizeof(char));
    if (n < 0) {
        perror("write3 error\n");
        return 1;
    }
    
    uint16_t *buf, pp;
    buf = &pp;
    pp = htons(ownport);
    printf("ownport: %d, pp: %d\n", ownport, pp);
    n = write(fd, buf, sizeof(uint16_t));
    if (n < 0) {
        perror("write4 error\n");
        return 1;
    }    
   
    n = write(fd, " ", sizeof(char));
    if (n < 0) {
        perror("write5 error\n");
        return 1;
    }
    n = write(fd, opnro, strlen(opnro));
    if (n < 0) {
        perror("write6 error\n");
        return 1;
    }
    
    // Jos yhteydenotto tähän osoitteeseen ei onnistu, 
    // lähetä pistoke-A:han merkkijono FAIL, joka loppuu rivinvaihtomerkkiin. 
    // Tämä tehtävä ei toimi NATin takaa. Miksi?


    // Tämän jälkeen odota uutta komentoa palvelimelta (pistoke-A:sta), 
    // kunnes pistoke-A:sta tulee joko “OK” tai “FAIL”.
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
