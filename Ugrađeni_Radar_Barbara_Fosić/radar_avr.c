#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define OLED_ADDR 0x3C

#define SERVO_MID    22   
#define SERVO_RIGHT  20   
#define SERVO_LEFT   25   

const uint8_t font_digits[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4B, 0x31},
    {0x18, 0x14, 0x12, 0x7F, 0x10}, {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1E}
};

// --- I2C / OLED ---
void i2c_init() { TWBR = 72; TWSR = 0; }
void i2c_start() { TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); while (!(TWCR & (1<<TWINT))); }
void i2c_stop() { TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN); }
void i2c_write(uint8_t data) { TWDR = data; TWCR = (1<<TWINT) | (1<<TWEN); while (!(TWCR & (1<<TWINT))); }
void oled_cmd(uint8_t cmd) { i2c_start(); i2c_write(OLED_ADDR << 1); i2c_write(0x00); i2c_write(cmd); i2c_stop(); }
void oled_data(uint8_t data) { i2c_start(); i2c_write(OLED_ADDR << 1); i2c_write(0x40); i2c_write(data); i2c_stop(); }

void oled_init() {
    _delay_ms(100);
    oled_cmd(0xAE); oled_cmd(0x8D); oled_cmd(0x14); oled_cmd(0xAF);
    oled_cmd(0x20); oled_cmd(0x02);
}

void oled_set_cursor(uint8_t page, uint8_t col) {
    oled_cmd(0xB0 + page); oled_cmd(0x00 + (col & 0x0F)); oled_cmd(0x10 + (col >> 4));
}

void oled_print_digit_big(uint8_t d, uint8_t col) {
    oled_set_cursor(3, col);
    for(int i=0; i<5; i++) {
        uint8_t b = font_digits[d][i];
        uint8_t r = 0;
        if(b & 0x01) r |= 0x03; if(b & 0x02) r |= 0x0C; if(b & 0x04) r |= 0x30; if(b & 0x08) r |= 0xC0;
        oled_data(r); oled_data(r);
    }
    oled_set_cursor(4, col);
    for(int i=0; i<5; i++) {
        uint8_t b = font_digits[d][i];
        uint8_t r = 0;
        if(b & 0x10) r |= 0x03; if(b & 0x20) r |= 0x0C; if(b & 0x40) r |= 0x30; if(b & 0x80) r |= 0xC0;
        oled_data(r); oled_data(r);
    }
}

// --- SHIFT REGISTER ---
void shift_out(uint8_t data) {
    PORTD &= ~(1 << 4);
    for (int i = 0; i < 8; i++) {
        if (data & (1 << (7 - i))) PORTD |= (1 << 2); else PORTD &= ~(1 << 2);
        PORTD |= (1 << 3); _delay_us(2); PORTD &= ~(1 << 3);
    }
    PORTD |= (1 << 4);
}

// --- SENZOR ---
uint16_t get_dist() {
    PORTB &= ~(1 << 1); _delay_us(2);
    PORTB |= (1 << 1); _delay_us(10);
    PORTB &= ~(1 << 1);
    uint32_t d = 0;
    while (!(PINB & (1 << 2)) && d < 20000) d++;
    d = 0;
    while (PINB & (1 << 2) && d < 20000) { d++; _delay_us(1); }
    return d / 58;
}

void skeniraj_i_ispisi() {
    uint16_t d = get_dist();
    
    // OLED ispis
    if (d > 99) d = 99;
    for(uint8_t p=3; p<=4; p++) {
        oled_set_cursor(p, 45);
        for(int i=0; i<30; i++) oled_data(0x00);
    }
    oled_print_digit_big((d/10)%10, 50);
    oled_print_digit_big(d%10, 62);

    // Lampice i Buzzer
    if (d > 0 && d < 15) {
        shift_out(0b00000100); // Crvena
        PORTB |= (1 << 0);
    } else {
        PORTB &= ~(1 << 0);
        if (d >= 15 && d <= 30) shift_out(0b00000010); // Žuta
        else shift_out(0b00000001); // Zelena
    }
}

int main(void) {
    DDRB |= (1 << 1) | (1 << 0) | (1 << 3);
    DDRD |= (1 << 2) | (1 << 3) | (1 << 4);

    
    TCCR2A = (1 << WGM21) | (1 << WGM20) | (1 << COM2A1); 
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);    
    
    i2c_init();
    oled_init();
    
    // Clear OLED
    for(uint8_t p=0; p<8; p++) {
        oled_set_cursor(p, 0);
        for(int i=0; i<128; i++) oled_data(0x00);
    }

    while (1) {
        
        for (uint8_t pos = SERVO_MID; pos >= SERVO_RIGHT; pos--) { OCR2A = pos; skeniraj_i_ispisi(); _delay_ms(80); }
        for (uint8_t pos = SERVO_RIGHT; pos <= SERVO_MID; pos++) { OCR2A = pos; skeniraj_i_ispisi(); _delay_ms(80); }
        for (uint8_t pos = SERVO_MID; pos <= SERVO_LEFT; pos++)  { OCR2A = pos; skeniraj_i_ispisi(); _delay_ms(80); }
        for (uint8_t pos = SERVO_LEFT; pos >= SERVO_MID; pos--)  { OCR2A = pos; skeniraj_i_ispisi(); _delay_ms(80); }
    }
}