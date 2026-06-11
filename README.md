# Ultrazvučni radar za detekciju objekata s rotacijskim mehanizmom

Ovaj repozitorij sadrži cjelovitu implementaciju autonomnog ultrazvučnog radara.

Sustav koristi princip eholokacije za skeniranje prostora u polukrugu od 180 stupnjeva, prepoznaje prepreke u stvarnom vremenu te aktivira svjetlosnu, zvučnu i vizualnu signalizaciju ovisno o udaljenosti najbližeg objekta.

---

## Značajke Sustava i Arhitektura

Glavna posebnost ovog projekta je razvoj u čistom AVR C jeziku izravnim modificiranjem registara mikrokontrolera ATmega328P, potpuno zaobilazeći standardne Arduino biblioteke i
skrivene funkcije (poput digitalWrite, pulseIn ili gotovih servo/I2C klasa). Time je postignut iznimno brz rad, predvidljivo ponašanje i minimalan memorijski otisak (low memory footprint).

* Hardverski PWM (Timer2): Upravljanje zakretanjem servo motora MG90S ostvareno je konfiguracijom 8-bitnog brojača Timer2 u Phase Correct načinu rada s djeliteljem frekvencije (prescaler) 1024,
   generirajući stabilan signal frekvencije 50 Hz.
* Četverofazno skeniranje: Servo motor se ne pomiče naglo, već prati glatku putanju u četiri faze: Sredina -> Desno -> Sredina -> Lijevo -> Sredina uz pauze od 80 ms po koraku kako bi se mehanički
  smirio ultrazvučni senzor i eliminirao šum u mjerenju.
* Proširenje izlaza (Shift Registar 74HC595): Kako bi se uštedjeli pinovi mikrokontrolera, upravljanje trima LED diodama i piezo zvučnikom prebačeno je na shift registar, koji troši svega 3 digitalne
   linije (Data, Clock, Latch).
* Hardverski I2C (TWI) Protokol: Komunikacija s OLED ekranom izvedena je izravnim upravljanjem TWBR, TWSR i TWCR registrima mikrokontrolera.

### Dinamičke Zone Alarma
Ovisno o izmjerenoj udaljenosti, sustav trenutačno reagira kroz tri zone sigurnosti:
1. Kritična zona (< 15 cm): Preko shift registra pali se crvena LED dioda, a pasivni piezo zvučnik počinje neprekidno odašiljati prodoran zvučni signal.
2. Zona upozorenja (15 cm - 30 cm): Potreban oprez. Zvučni signal se gasi, a shift registar prebacuje svjetlo na žutu LED diodu.
3. Sigurna zona (> 30 cm): Put je čist. Svijetli isključivo zelena LED dioda, a alarmni sustav miruje.

---

## Popis Hardverskih Komponenti

| Komponenta | Model / Čip | Svrha i uloga u sustavu |
| :--- | :--- | :--- |
| Mikrokontroler | Arduino Nano (ATmega328P) | Središnja procesorska jedinica s taktom od 16 MHz |
| Senzor udaljenosti | HC-SR04 | Emitira i prima ultrazvučne valove (40 kHz) za izračun udaljenosti |
| Rotacijski pogon | MG90S Servo | Precizno zakreće senzor (opremljen metalnim zupčanicima) |
| Zaslon | 0.96" Monokromatski OLED | Prikaz udaljenosti u realnom vremenu (I2C sučelje, SSD1306) |
| Shift Registar | 74HC595 | Serijsko-paralelni pretvornik za upravljanje signalizacijom |
| Zvučni indikator | Pasivni Piezo Buzzer (5V) | Generira akustični alarm u kritičnoj zoni |
| Svjetlosni indikatori | 3x Standardne LED | Vizualno označavanje zona (Crvena, Žuta, Zelena) |

---

## Električna Shema i Pinout (Raspored Pinova)

Povežite komponente na pločicu Arduino Nano prema sljedećoj konfiguraciji registara:

### 1. Senzori i Aktuatori
* HC-SR04 Trigger -> PORTB1 (Digitalni pin 9)
* HC-SR04 Echo -> PINB2 (Digitalni pin 10)
* MG90S Servo (PWM signal) -> PORTB3 (Digitalni pin 11 / Hardverski izlaz OCR2A)
* Piezo Buzzer (+) -> PORTB0 (Digitalni pin 8)

### 2. Shift Registar (74HC595)
* Data Line (SER) -> PORTD2 (Digitalni pin 2)
* Clock Line (SRCLK) -> PORTD3 (Digitalni pin 3)
* Latch Line (RCLK) -> PORTD4 (Digitalni pin 4)

### 3. OLED Zaslon
* SDA (Data) -> PORTC4 (Analogni pin A4 - Hardverski I2C)
* SCL (Clock) -> PORTC5 (Analogni pin A5 - Hardverski I2C)

---

## Razvojno Okruženje i Pokretanje

Projekt zaobilazi standardno grafičko sučelje Arduina i koristi izravni AVR Toolchain alatnog koda.

### Potrebni alati na računalu:
* avr-gcc (Kompajler za AVR arhitekturu procesora)
* avr-objcopy (Alat za generiranje konačne .hex datoteke)
* avrdude (Alat za prijenos koda na mikrokontroler preko USB veze)

### Automatizirano pokretanje (PowerShell)

U repozitoriju se nalazi skripta pokreni.ps1 koja automatski prepoznaje instalirane kompajlerske alate na sustavu, gradi projekt i šalje ga na uređaj.

1. Otvorite datoteku pokreni.ps1 i prilagodite COM port vašem Arduinu (npr. COM3, COM4...):
   ```powershell
   $MOJ_PORT = "COM3"
