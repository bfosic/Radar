
$MOJ_PORT = "COM3"  


$USER_HOME = $env:USERPROFILE
$STAZA_ALATA = "$USER_HOME\AppData\Local\Arduino15\packages\arduino\tools"


$GCC = Get-ChildItem -Path "$STAZA_ALATA\avr-gcc\*\bin\avr-gcc.exe" | Select-Object -ExpandProperty FullName -First 1
$OBJCOPY = Get-ChildItem -Path "$STAZA_ALATA\avr-gcc\*\bin\avr-objcopy.exe" | Select-Object -ExpandProperty FullName -First 1
$AVRDUDE = Get-ChildItem -Path "$STAZA_ALATA\avrdude\*\bin\avrdude.exe" | Select-Object -ExpandProperty FullName -First 1
$CONF = Get-ChildItem -Path "$STAZA_ALATA\avrdude\*\etc\avrdude.conf" | Select-Object -ExpandProperty FullName -First 1

Clear-Host
Write-Host "--- KREĆEM S RADOM ---" -ForegroundColor Cyan


Write-Host "1. Kompajliram kod..." -ForegroundColor White
& $GCC -Wall -Os -mmcu=atmega328p radar_avr.c -o radar.elf
if ($LASTEXITCODE -ne 0) { Write-Host "GREŠKA: Kompajliranje nije uspjelo!" -ForegroundColor Red; exit }


Write-Host "2. Generiram HEX datoteku..." -ForegroundColor White
& $OBJCOPY -O ihex radar.elf radar.hex

Write-Host "3. Šaljem na Arduino na portu $MOJ_PORT..." -ForegroundColor Yellow
& $AVRDUDE -C "$CONF" -v -patmega328p -carduino -P $MOJ_PORT -b 115200 -D -U flash:w:radar.hex:i

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n--- USPJEH! RADAR JE POKRENUT ---" -ForegroundColor Green
} else {
    Write-Host "`n--- GREŠKA PRI SLANJU! Provjeri COM port i kabel. ---" -ForegroundColor Red
}