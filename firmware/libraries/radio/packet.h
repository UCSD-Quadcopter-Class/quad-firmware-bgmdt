
#ifndef PACKET_H
#define PACKET_H

#include <radio.h>

void sendPacket(uint8_t * values, uint8_t num){

    rfWrite(values, num);

}

int MAXCHAR = B11111111;
bool receivePacket(int * values){

    char packet[8];
    if(rfAvailable() >= 8){
        for(int i = 0; i < 8; ++i){
            packet[i] = rfRead();
        }
        while(rfAvailable()){
            rfRead();
        }
    } else {
        return 0;
    }

    /*
    for(int i = 0; i <= 10; i+=2){
        int value = *((int*)(packet + i));
        values[i/2] = value;
    }

    values[6] = (packet[11]) ? 1024 : 0;
    values[7] = (packet[12]) ? 1024 : 0;

    return 1;
    */

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
