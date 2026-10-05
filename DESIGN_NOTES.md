# Signal Lost engineering prototype

This is an editable, AI-assisted engineering prototype for your inspection and learning. It is not a tested build or a claim of original student work. Read and redraw or substantially adapt the design before using it for a Half Life submission; write your own README and journal entries describing what you actually made and tested.

## What the PCB does

- An 80 × 55 mm, two-layer USB-powered carrier holds an Adafruit QT Py ESP32-S3 and a Bourns PEC11 rotary encoder with push switch.
- A 10 mm exposed copper pad acts as the hidden touch input. Two 0805 LEDs indicate tuning and signal state.
- A five-pin header feeds an external Adafruit MAX98357A I2S amplifier breakout. An 8 Ω speaker attaches to the amplifier, not to the carrier PCB.
- A 128 × 32 OLED attaches to the QT Py's built-in STEMMA QT connector using a 4-pin cable.
- The back silkscreen carries clues for the secret station. The current PCB has no illuminated cutouts or enclosure; those parts of the original idea remain to design.

## Assembly map

| Carrier J1 | Amplifier breakout |
| --- | --- |
| 1 / 5V | VIN |
| 2 / GND | GND |
| 3 / LRC | LRC |
| 4 / BCLK | BCLK |
| 5 / DIN | DIN |

Use five separate jumper wires because the breakout pin order includes other pins between these signals. Leave GAIN and SD/MODE at the breakout's default settings. Connect the speaker to the amplifier's **OUT+ and OUT−** terminals; neither speaker lead connects to ground. Solder the QT Py's 7-pin header rows into the carrier and connect the OLED to the QT Py's STEMMA QT socket. The USB-C port powers the assembly. Check the purchased encoder's pin spacing against the supplied `PEC11_Switch.kicad_mod` before ordering boards.

## Files

- `signal-lost.kicad_sch`, `signal-lost.kicad_pcb`, `signal-lost.kicad_pro`: editable KiCad 9 design.
- `SignalLost.kicad_sym`, `SignalLost.pretty/`, library tables: custom symbol and footprints needed by the design.
- `firmware/`: PlatformIO source for the QT Py ESP32-S3 4 MB flash / 2 MB PSRAM variant.
- `gerbers/` and `signal-lost-gerbers.zip`: fabrication outputs from the board design.
- `bom-signal-lost.csv`: parts list with current individual prices and vendor links; **$46.58 parts subtotal** before PCB fabrication, shipping, tax, USB cable and enclosure. Confirm prices before ordering.
- `board-render.png`, `board-back.png`, `schematic.pdf`: visual references.

## Verification and remaining work

- KiCad schematic ERC: **0 errors, 0 warnings**.
- KiCad PCB DRC: **0 errors, 0 unconnected pads**, and one warning because the front silkscreen is clipped by the intentionally exposed touch pad.
- PlatformIO firmware build: **successful** for `adafruit_qtpy_esp32s3_n4r2`.
- No hardware has been assembled, powered, flashed, or physically tested. Touch thresholds, encoder direction, OLED address, sound level, and cable fit need bench testing. Keep the initial volume low with the 1 W speaker.
- The current drawing is a carrier plus breakout modules, not a complete enclosed pocket device. Design an enclosure, mounting for the display/speaker/amp, and any light windows after measuring the actual parts.

## Technical references

- [Adafruit QT Py ESP32-S3 documentation](https://learn.adafruit.com/adafruit-qt-py-esp32-s3)
- [Adafruit MAX98357A amplifier wiring and output details](https://learn.adafruit.com/adafruit-max98357-i2s-class-d-mono-amp)
- [Adafruit's PEC11 encoder board](https://github.com/adafruit/Adafruit-I2C-QT-Rotary-Encoder-PCB)
- [Same Sky speaker specifications](https://www.digikey.com/en/products/detail/same-sky-formerly-cui-devices/CMS-30204-18L250/24398508)
