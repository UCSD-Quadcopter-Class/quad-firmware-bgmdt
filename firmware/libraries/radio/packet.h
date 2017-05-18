
#ifndef PACKET_H
#define PACKET_H

#include <radio.h>
//#define PACKET_DEBUG 1

typedef struct structPacket Packet;
struct structPacket {
    int throttle;
    int yaw;
    int pitch;
    int roll;
    int pot1;
    int pot2;
    int btn1;
    int btn2;
    int verify;
};

void sendPacket(uint8_t * values, uint8_t num){

    Serial.print("Writing...\n");
    rfWrite(values, num);

}

bool receivePacket(Packet &pkt){
    char buffer[18];
    int values[9];

    if(rfAvailable() >= sizeof(Packet)){
        rfRead(buffer, sizeof(Packet));
    }

    for(int i = 0; i < 9; ++i){
        int * reference = (int*)(buffer + 2*i);
        values[i] = *reference;
		#ifdef PACKET_DEBUG
        Serial.print(values[i]);
        Serial.print(' ');
		#endif
    }
	#ifdef PACKET_DEBUG
    Serial.print('\n');
    #endif
    pkt.throttle = values[0];
    pkt.yaw = values[1];
    pkt.pitch = values[2];
    pkt.roll = values[3];
    pkt.pot1 = values[4];
    pkt.pot2 = values[5];
    pkt.btn1 = values[6];
    pkt.btn2 = values[7];
    pkt.verify = values[8];

    return;
}

#endif
