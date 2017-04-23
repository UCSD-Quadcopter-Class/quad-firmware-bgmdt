
#ifndef PACKET_H
#define PACKET_H

#include <radio.h>

char* sendPacket(int* values){

    char * packet = malloc(sizeof(char)*8);
    char MAXCHAR = B11111111;

    char packet2[13];
    for(int i = 0 ; i < 6; ++i){
        char * valuesRef = (char*) values;
        char idx1 = *(valuesRef + i);
        char idx2 = *(valuesRef + i + 1);
        packet2[2*i] = idx1;
        packet2[2*i+1] = idx2;
    }
    packet2[11] = values[6] ? 1 : 0;
    packet2[12] = values[7] ? 1 : 0;

    for(int i = 0; i < 13; ++i){
        rfWrite(packet2[i]);
    }
    return;

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

    for(int i = 0; i < 8; ++i){
        rfWrite(packet[i]);
    }

    return packet;
}

int MAXCHAR = B11111111;
bool receivePacket(int * values){

    char packet[13];
    if(rfAvailable() >= 13){
        for(int i = 0; i < 13; ++i){
            packet[i] = rfRead();
        }
        while(rfAvailable()){
            rfRead();
        }
    } else {
        return 0;
    }

    for(int i = 0; i <= 10; i+=2){
        int value = *((int*)(packet + i));
        values[i/2] = value;
    }

    values[6] = (packet[11]) ? 1024 : 0;
    values[7] = (packet[12]) ? 1024 : 0;

    return 1;

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

    values[6] = (packet[7] >> 1) & 1 ? 1024 : 0;
    values[7] = packet[7] & 1 ? 1024 : 0;

    return 1;
}

#endif
