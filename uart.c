#include <mega164.h>
#define T 20 // perioada intreruperilor periodice (ms)
int TAB_RATE[4]={600,1200,2400,9600}; // tabela rate de bit (biti / sec)
char TAB_LENGTH[4]= {5,6,7,8}; // tabela lungimi
char length; // lungime caracter (data)
int rate; // rata de bit
char par; // paritate
char N ; // constanta pentru masurarea duratei de bit (durata de bit = N*T)
char Q; // stare
char err; // eroare
char start; //bit de start
char cnt; //contor folosit pentru durata de bit
char cntb; //numarul de biti transmisi
char b; //bitul transmis
char p; //paritatea calculata
char w; //cuvant impachetat / conversie serie-paralel
char r;
char l;
char rx,tx;
char i; //contor pentru conversia paralel-serie
void main(void)
{
#pragma optsize-
CLKPR=(1<<CLKPCE);
CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif
// Port B initialization
DDRB=(1<<DDB7) | (1<<DDB6) | (1<<DDB5) | (1<<DDB4) | (1<<DDB3) | (1<<DDB2) | (1<<DDB1) | (1<<DDB0);
PORTB=(1<<PORTB7) | (1<<PORTB6) | (1<<PORTB5) | (1<<PORTB4) | (1<<PORTB3) | (1<<PORTB2) | (1<<PORTB1) | (1<<PORTB0);
// Port C initialization
DDRC=(0<<DDC7) | (0<<DDC6) | (0<<DDC5) | (0<<DDC4) | (0<<DDC3) | (0<<DDC2) | (0<<DDC1) | (0<<DDC0);
PORTC=(1<<PORTC7) | (1<<PORTC6) | (1<<PORTC5) | (1<<PORTC4) | (1<<PORTC3) | (1<<PORTC2) | (1<<PORTC1) | (1<<PORTC0);
// Port D initialization
DDRD=(0<<DDD7) | (0<<DDD6) | (0<<DDD5) | (0<<DDD4) | (0<<DDD3) | (0<<DDD2) | (0<<DDD1) | (0<<DDD0);
PORTD=(1<<PORTD7) | (1<<PORTD6) | (1<<PORTD5) | (1<<PORTD4) | (1<<PORTD3) | (1<<PORTD2) | (1<<PORTD1) | (1<<PORTD0);
// Timer/Counter 0 initialization
TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
TCCR0B=(0<<WGM02) | (1<<CS02) | (0<<CS01) | (1<<CS00);
TCNT0=0x3C;
OCR0A=0x00;
OCR0B=0x00;
// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (1<<TOIE0);
// Globally enable interrupts
#asm("sei")
while (1)
{
}
}
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
TCNT0=0x3C;
switch(Q)
{
case 0: //se verifica cuvantul de programare transmis paralel pe portul C
{
tx=(PINC&0x80)>>7; //b7->tx , 0=disable, 1-activ
rx=(PINC&0x40)>>6; //b6->rx , 0=disable, 1-activ
r=PINC&0x03; //b1 b0 -> 00=600, 01=1200, 10=2400, 11=9600
l=(PINC&0x0C)>>2; //b3 b2 -> 00=5 ,01=6, 10=7, 11=8
rate=TAB_RATE[r]; //rate = 1200
length=TAB_LENGTH[l]; // length = 8 -> d0 d1 d2 d3 ... d7
par=(PINC&0x10)>>4; // b4 = 1->paritate , 0->fara paritate
N=rate/T; // numarul de intreruperi periodice corespunzatoare duratei de bit
Q=1;
break;
}
case 1 : // asteapta bitul de start
{
cnt=0; // contor pentru masurarea duratei de bit
cntb=0; // contor de biti in cuvantul serial
w=0; // cuvantul serial impachetat w = Dn-1 Dn-2 ... D1 D0
p=0; // paritatea calculata
err=0; // eroare de paritate sau de bit de STOP
start=PIND&0x01; //citeste bit start
if (start==0) Q=2;
break;
}
case 2 : //receptioneaza bitii, calculeaza paritatea, impacheteaza cuvantul(RxD) sau trimite pe PORTB(TxD)
{
cnt++;
if (cnt==N) // asteapta durata de bit
{
cnt=0;
b=PIND&0x01; // citeste bitul curent
if (par==1) p=p^b; // calculeaza paritatea
w = w | (b << cntb); // impacheteaza cuvantul / conversie S-P
cntb++;
if (cntb == length) Q=3;
}
break;
}
case 3 : // verifica paritatea
{
if (par!=1) Q=4; //nu avem paritate
cnt++;
if (cnt==N) // asteapta durata de bit
{
cnt=0;
b=PIND&0x01; // citeste paritatea
if (b!= p) err=1; // verifica paritatea
Q=4;
}
break;
}
case 4 : // verifica bitul de stop
{
cnt++;
if (cnt==N) // asteapta durata de bit
{
cnt=0;
b=PIND&0x01; // citeste bitul de stop
if (b!=1) err=1; // verifica bitul de stop
Q=5;
}
break;
}
case 5: // verifica eroarea
{
if(err==0) {
if(rx==1)PORTB=w;
else if(tx==1){
i++;
PORTB= w%2;
w=w/2; //conversie P-S
}
if(i==length)
Q=0;
break;
}
} }
}