#include <avr/io.h>
#include "i2c.h"
#include "SSD1306.h"
#include <util/delay.h>
#include <avr/io.h>
#include <string.h> // Defines strlen() function
#include <stdlib.h> // Defines itoa() function 
#include <stdio.h>

// Created by Jacob Constable, student of Umass Amherst 4/1/24
// Majority of functions provided by Professor D. McLaughlin
// This code is a combination of two projects I finished in Professor D McLaughlin's 231 class. 
// Project 1 Temperature Sensor- Tmp36 & Seven-Segment Display
// Project 2 Range Finder- Ultrasonic Range Finder and Oled Display
// This Project Temperature Sensor- Tmp36, Oled Display, ATmega328p, AVR Pocket Programmer, Sparkfun Serial to USB 

//           Left     ATmega328p pins     Right
// PC6 (Reset) - AVR (Reset)   |  PC5 (SCL) - Oled (SCL)
// PD0 (RXD)   - None          |  PC4 (SDA) - Oled (SDA)
// PD1 (TXD)   - None          |  PC3       - None
// PD2         - None          |  PC2       - Tmp36 (Input)
// PD3         - Button(Input) |  PC1       - None
// PD4         - None          |  PC0       - None
// VCC         -               |  GND       - 
// GND         -               |  AREF      - None
// PB6         - 16MHz Clock   |  AVCC      - None
// PB7         - 16MHz Clock   |  PB5 (SCK) - AVR (SCK)
// PD5         - None          |  PB4 (MISO)- AVR (MISO) 
// PD6         - None          |  PB3 (MOSI)- AVR (MOSI)
// PD7         - None          |  PB2 (SS)  - None
// PB0         - None          |  PB1       - None

#define PERSISTENCE 5 // Delay varaible

void uart_init(void); // For Putty, use Sparkfun Serial to USB
void uart_send(unsigned char); // For Putty, use Sparkfun Serial to USB
void send_string (char *stringAddress);
void timer0_init(void);
void send_to_monitor (unsigned char, unsigned char, unsigned char); // Orders characters for Oled
void adc_init(void); // Select analog input pin (ADC2, PC2) and reference voltage (5V), Tmp36 output connects here
unsigned int get_adc(void); // Read from Tmp36

int main(void){
    unsigned int digitalValue, voltInt;
    unsigned char DIG1, DIG2, DIG3;
    char buffer[6];
    adc_init(); // For Putty
    uart_init(); // For Putty

    PORTD = 1<<PORTD3; // Set pullup resistor for button (PD3)

    timer0_init(); // Initialize timer
    OLED_Init(); // Function in SSD1306.c

    while (1) {
        if (((PIND & (1<<PIN3)) == 0 )) { // Check if PIN3 is 0 when pressed (PD3)
            digitalValue = get_adc(); // Read from Tmp36
            itoa (digitalValue, buffer, 100);
            send_string(buffer);
            voltInt = digitalValue*100; // Temp36 runs 10mV/degree C

            int i = 0;
            while(i < 5){ // will update once i = 6, helps stabilize output 

                DIG3 = (voltInt/100)%10; // __.#
                DIG2 = (voltInt/1000)%10; // _#._
                DIG1 = (voltInt/10000)%10; // #_._
            
                uart_send(DIG1+'0'); // For Putty
                _delay_ms(PERSISTENCE);

                uart_send(DIG2+'0'); // For Putty
                _delay_ms(PERSISTENCE);

                uart_send(DIG3+'0'); // For Putty
                _delay_ms(PERSISTENCE); 

                uart_send(13); //tx carriage return 
                uart_send(10); //tx line feed
                _delay_ms(PERSISTENCE);
                send_to_monitor(DIG1, DIG2, DIG3); // For OLED
                i++;
            }
            
        } else {
            digitalValue = get_adc(); // Improves read from Tmp36
            itoa (digitalValue, buffer, 100);
            send_string(buffer);
            voltInt = digitalValue*180 + 100000; // Temp36 mV*180 then add 32 in binary 
            
            int i = 0;
            while(i < 5){ // will update once i = 6, helps stabilize output 

                DIG3 = (voltInt/100)%10; // __.#
                DIG2 = (voltInt/1000)%10; // _#._
                DIG1 = (voltInt/10000)%10; // #_._

                uart_send(DIG1+ '0'); // For Putty
                _delay_ms(PERSISTENCE);

                uart_send(DIG2+'0'); // For Putty
                _delay_ms(PERSISTENCE);

                uart_send(DIG3+'0'); // For Putty
                _delay_ms(PERSISTENCE); 

                uart_send(13); //tx carriage return 
                uart_send(10); //tx line feed
                _delay_ms(PERSISTENCE);
                send_to_monitor(DIG1, DIG2, DIG3);
                i++;
            }
            
        }
    }
    return 0;
}

void send_to_monitor(unsigned char t1, unsigned char t2, unsigned char t3){
    char buffer[10]; 

    utoa(t1, buffer, 10); //send the delay count to comm port
    send_string(buffer);
   
    utoa(t2, buffer, 10); //send the delay count to comm port
    send_string(buffer);
    
    utoa(t2-t1, buffer, 10);
    send_string (buffer);
    
    dtostrf(t3, 3, 0, buffer);
    send_string (buffer);

    dtostrf(t3, 3, 0, buffer); //send the delay count to comm port
    send_string (buffer);
    
    uart_send(13); //tx carriage return
    uart_send(10); //tx new line
    
    OLED_GoToLine(1); // Line 1 of OLED
    OLED_DisplayNumber(10,t1,1); // Display range in decimal with 3 digits
    OLED_DisplayNumber(10,t2,1); // Display range in decimal with 3 digits
    OLED_DisplayString("."); 
    OLED_DisplayNumber(10,t3,1); // Display range in decimal with 3 digits
    if (((PIND & (1<<PIN3)) == 0 )) { // Button connects here (PD3)
        OLED_DisplayString(" C ");
    } else {
        OLED_DisplayString(" F ");
        if (t1 > 7){ // if #_._ is greater than 5, output (Ex 80.0 will read "Too Hot" & 79.9 will read "      ")
            OLED_GoToLine(2);
            OLED_DisplayString("Too Hot"); // Triggered at 80 degrees fahrenheit
        } else {
            OLED_GoToLine(2);
            OLED_DisplayString("          ");
        }
    } 

} 
// Initialize ADC peripheral: select ADC2; Vref=AVCC=5V;  
void adc_init(void){ 
    ADMUX = (1 << REFS0) | (1 << MUX1); // Set Vref to AVCC (5V) and select ADC2
    ADCSRA = 0x87;
}
// Read ADC value
unsigned int get_adc(){ // Read from Tmp36
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADIF))==0);
    return ADCL | (ADCH << 8);
}
void timer0_init(){
    TCCR0A = 0; // Timer 1 Normal mode (count up)
    TCCR0B = 5; // Divide clock by 1024
    TCNT0=0; // Start the timer at 0
}

void send_string(char *stringAddress){
    unsigned char i;
    for (i = 0; i < strlen(stringAddress); i++)
        uart_send(stringAddress[i]);
}

void uart_init(void){
    UCSR0B = (1 << TXEN0); //enable the UART transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); //set 8 bit character size
    UBRR0L = 103; //set baud rate to 9600 for 16 MHz crystal
}

void uart_send(unsigned char ch){
    while (!(UCSR0A & (1 << UDRE0))); //wait til tx data buffer empty
    UDR0 = ch; //write the character to the USART data register
}
