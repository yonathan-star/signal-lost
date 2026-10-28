# Signal Lost engineering prototype

This is an editable engineering prototype. The hardware has not been assembled or physically tested.

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

- `PCB/`: editable KiCad 9 project, schematic, board, custom symbols, footprints, and project-local library tables. Open `PCB/signal-lost.kicad_pro`; keep the folder intact.
- `firmware/`: PlatformIO source for the QT Py ESP32-S3 4 MB flash / 2 MB PSRAM variant.
- `fabrication/gerbers/` and `fabrication/signal-lost-gerbers.zip`: manufacturing outputs.
- `bom-signal-lost.csv`: component quantities, listed prices, and vendor links; see the README for the recorded funding totals. Confirm prices before ordering.
- `images/`: front/back board renders, schematic PNG, and schematic SVG.
- `docs/schematic.pdf` and `docs/verification/`: readable schematic and existing KiCad check reports.
- `JOURNAL.md` and `BOM.md`: records mirrored from Half Life; their pinned historical image URLs remain unchanged.

All paths above are relative to the repository root.

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
