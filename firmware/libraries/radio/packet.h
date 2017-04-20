
#ifndef PACKET_H
#define PACKET_H

#include <radio.h>

void sendPacket(int* values){

    char packet[8];
    char MAXCHAR = B11111111;

    packet[0] = (values[0] & MAXCHAR) >> 2;
    packet[1] = ((values[0] & MAXCHAR) << 6) | ((values[1] & MAXCHAR) >> 4);
    packet[2] = ((values[1] & MAXCHAR) << 4) | ((values[2] & MAXCHAR) >> 6);
    packet[3] = ((values[2] & MAXCHAR) << 2) | ((values[3] & MAXCHAR) >> 8);
    packet[4] = ((values[3] & MAXCHAR));
    packet[5] = (values[4] & MAXCHAR) >> 2;
    packet[6] = ((values[4] & MAXCHAR) << 6) | ((values[5] & MAXCHAR) >> 4);
    packet[7] = ((values[5] & MAXCHAR) << 4);
    packet[7] |= values[6] == 1024? 1 << 1 : 0;
    packet[7] |= values[7] == 1024? 1 : 0;

    rfPrint(packet);
}


int MAXCHAR = B11111111;
void receivePacket(int * values){

    char packet[8];
    if(rfAvailable() == 8){
        for(int i = 0; i < 8; ++i){
            packet[i] = rfRead();
        }
    }

    int increment = 2;
    for(int i = 0; i <= 5; ++i){
        int mask = (1 << (8 - ((increment - 2) % 8))) - 1;
        int value = packet[i + (increment - 2)/8] & mask;
        value <<= (increment % 8)? (increment % 8): increment;

        int remainder = (increment % 8)? 
            (MAXCHAR & packet[1 + i + (increment - 2)/8]) >> (8 - (increment % 8)):
            (MAXCHAR & packet[1 + i + (increment - 2)/8]);

        values[i] = value + remainder;
        increment += 2;
    }

    values[6] = packet[7] >> 1 ? 1024 : 0;
    values[7] = packet[7] & 1 ? 1024 : 0;

    return;
}

#endif
