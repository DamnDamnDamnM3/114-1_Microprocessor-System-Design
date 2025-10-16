#include <stdio.h>
#include <stdlib.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "LCD.h"
#include "Scankey.h"
#include "Seven_Segment.h"
#define MINUS_CODE 16
#define SEG_BLANK 0xFF

static void bin_str(char* out, unsigned char v){
  int i;
  out[0]='0'; out[1]='b';
  for(i=0;i<8;i++) out[2+i] = (v & (1<<(7-i)))? '1':'0';
  out[10]='\0';
}

static void lcd_show_bin(unsigned char N){
  char line[17];
  int i;
  char b[11];
  for(i=0;i<16;i++) line[i]=' ';
  line[16]='\0';
  bin_str(b,N);
  for(i=0;i<10;i++) line[3+i]=b[i];
  print_Line(1, line);
}

static unsigned char dig[4]={0,0,0,0};

static unsigned char mask[4]={0,0,0,0};

static void make_digits_U(unsigned int v){
    int i;
    for(i=0;i<4;i++){ dig[i]=0; mask[i]=0; }

    if(v == 0){
        dig[0]  = 0;
        mask[0] = 1;
        return;
    }

    dig[0]=(unsigned char)(v%10); mask[0]=1; v/=10;
    if(v>0){ dig[1]=(unsigned char)(v%10); mask[1]=1; v/=10; }
    if(v>0){ dig[2]=(unsigned char)(v%10); mask[2]=1; v/=10; }
    if(v>0){ dig[3]=(unsigned char)(v%10); mask[3]=1; }
}

// ---------- ???? (S) ----------
static void make_digits_S(int val){
    int neg = (val < 0);
    unsigned int a = (unsigned int)(neg ? -val : val);
    int i, idx;

    for(i=0;i<4;i++){ dig[i]=0; mask[i]=0; }

    if(!neg){
        if(a == 0){
            dig[0]  = 0;
            mask[0] = 1;
            return;
        }
        idx = 0;
        while(a>0 && idx<4){
            dig[idx] = a % 10;
            mask[idx]=1;
            a/=10;
            idx++;
        }
    }else{
        if(a >= 100){
            dig[3]=MINUS_CODE; mask[3]=1;
            dig[2]=(a/100)%10; mask[2]=1;
            dig[1]=(a/10)%10;  mask[1]=1;
            dig[0]=a%10;       mask[0]=1;
        }else if(a >= 10){
            dig[2]=MINUS_CODE; mask[2]=1;
            dig[1]=a/10;       mask[1]=1;
            dig[0]=a%10;       mask[0]=1;
        }else{
            dig[1]=MINUS_CODE; mask[1]=1;
            dig[0]=a;          mask[0]=1;
        }
    }
}

static void make_digits_X(unsigned char v){
  int i;
  for(i=0;i<4;i++){ dig[i]=0; mask[i]=0; }
  dig[0]=(unsigned char)(v & 0x0F);
  dig[1]=(unsigned char)((v>>4) & 0x0F);
  mask[0]=1; mask[1]=1;
}

static void refresh_once(void){
  int i;
  for(i=0;i<4;i++){
    CloseSevenSegment();
    if(mask[i]){
      ShowSevenSegment(i, dig[i]);
    }else{
      PE->DOUT = SEG_BLANK;
    }
    CLK_SysTickDelay(1000);
  }
}

int main(void){
  unsigned char N=0, mode=0;
  unsigned char k=0, prev_k=0;
  unsigned char prev_mode=255, prev_N=255;
  SYS_Init();
  init_LCD();
  clear_LCD();
  OpenKeyPad();
  CloseSevenSegment();
  lcd_show_bin(N);
  make_digits_U(N);
  while(1){
    k = ScanKey();
    
    if(prev_k==2 && k==0){
      N=(unsigned char)(rand()&0xFF);
    }

    if(prev_k==0 && k!=0){
      if(k==1) N=(unsigned char)((N>>1)|0x80); // 1>
      else if(k==3) N=(unsigned char)((N<<1)|0x01); // <1
      else if(k==4) N=(unsigned char)(N>>1); // 0>
      else if(k==6) N=(unsigned char)(N<<1); // <0
      else if(k==5) N=0; // 0???
      else if(k==7) mode=0; // U
      else if(k==8) mode=1; // S
      else if(k==9) mode=2; // X
    }
    
    if(N!=prev_N || mode!=prev_mode){
      lcd_show_bin(N);
      if(mode==0) make_digits_U(N);
      else if(mode==1) make_digits_S((signed char)N);
      else make_digits_X(N);
      prev_N=N; prev_mode=mode;
    }
    refresh_once();
    prev_k=k;
  }
}
