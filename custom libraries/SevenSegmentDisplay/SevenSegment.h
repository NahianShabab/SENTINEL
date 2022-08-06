#pragma once

void printCharacter(unsigned char map){
    for(int i=0;i<8;i++){
        if(map & (1<<i)){
            digitalWrite(9-i,HIGH);
        }else{
            digitalWrite(9-i,LOW);
        }
    }
}