## Files
The *.f3d files are Fusion archives. 
The STLs are as I printed them. 
The SVGs are for the laser. 
    In the PCB dxf file, red is used in vector mode on the laser cutter to cut the protoboard to size.
    In the ButtonLettering dxf file, red is used to etch the letters on the button matrix STL. 
    Similarly, in the Fascia Lettering, the red is used to etch the Fascia STL.

Note about materials: I'll try to give a search term and a link to an AliExpress source.

## Bill of Materials:

I'd say the total cost here is around $30. The screen at $15, the dual encoder at $5, the switches at about $3, protoboard at $2, and the rp2040 at $3, plus misc filament, hardware, wire. (AliExpress shipped prices. Higher for domestic sources.)

1. PLA in white and black.
2. Protoboard at least 170x50mm. I use the brown phenolic protoboard with single-side solderable copper. https://www.aliexpress.us/item/3256806823414882.html Search "prototype board pcb" and look for the brown ones.
3. 10 switches. This design uses "8x8 silent silicone switches" but not sure that I really like those as there's no tactile feedback. Only a mushy press. https://www.aliexpress.us/item/3256804402559566.html Search "8x8 silicone switch"
4. 1 Dual rotary encoder. Search EC11EBB24C03. You can use the green or the yellow ones. Green are more common. Yellow are cheaper.
5. 1 RP2040 microprocessor and USB cable. I use the most common $2-3 type. Aka Raspberry Pi Pico. No wireless necessary. You want one WITHOUT headers.
6. 1 3.12" OLED display. You can use whatever color you want. I like to get the white ones and then cover with ORACAL film to get the proper color display. Search: 3.12 OLED. You must get one with the 4-wire SPI interface. Do NOT just get a bare screen or one with just a dangling ribbon cable. https://www.aliexpress.us/item/3256805975276793.html
6b. Optional: If you want the orange color, get some Oracal 8300, color 033 "red orange" to cover the display. 
7. Hook up wire. Personal preference here, but I like to use 30 AWG wire-wrap wire like this: https://www.aliexpress.us/item/3256802844140015.html but I use 26 awg silicone wire for the display. https://www.aliexpress.us/item/3256807070653563.html
8. 4 m3 x 20 bolts. Head type doesn't really matter. Button head probably the best choice. https://www.aliexpress.us/item/3256806241082021.html search: m3 20 and pick.
9. 4 m3 inserts: I like these as I typically assemble/disassmble a lot during construction. You could skip these, but may need to make the bosses where the screws anchor a bit tighter. https://www.aliexpress.us/item/3256803396040989.html search: m3 brass insert

## Instructions: 
1. 3d print white: Fascia. 3d print black, buttons, backing plate, base plate, knobs. 
2. Spray paint the fascia black and the buttons white. I use 3-5 light coats, 10-15 mintues apart.
3. Laser cut the protoboard. Alignment of the holes in the protoboard with the template is critical here. Cut it so the copper side is face down!
4. Laser etch the buttons and fascia, using care to center lettering.
5. Use the backing plate as a template to place the switches and encoder on the cut protoboard. Double check the alignment. Get the switches all the way down on the board with the connected terminals on the switches aligned horizontally.
6. Start soldering! 
    Start with the display. Tricky!
    Using Hardware SPI:
    | RP2040 pin | OLED pin | Comment |
    | --- | --- | --- |
    | 18 | D0: Pin4 | SCK/Clock |
    | 19 | D1: Pin5 | Data (MOSI/MISO) |
    | 17 | 16 | CS |
    | 20 | 14 | DC |
    | 21 | 15 | Reset |
    | GND | 1 | Ground |
    | VBUS | 2 | 5v |

    Solder the switches
    I use a bare piece of single-strand wire to run a ground line across all the switches, then run awg30 to each switch/encoder pin to the RP2040
    You must use the correct pins for the display, however you can use whatever pins you want to connect the switches, but if you want to use my Mobiflight config, you'll need to follow this config:

```
<CustomDevice Name="KAP140_LCD" CustomType="CC_KAP140_LCD" Pins="18|19|17|20|21" Config="">
    <ConfiguredPins>
      <string>18</string>
      <string>19</string>
      <string>17</string>
      <string>20</string>
      <string>21</string>
    </ConfiguredPins>
</CustomDevice>
<Button Name="ALT_ENC_BUTTON" Pin="2" />
<Encoder Name="ALT_ENC_OUTER" PinLeft="4" PinRight="3" EncoderType="2" />
<Encoder Name="ALT_ENC_INNER" PinLeft="1" PinRight="15" EncoderType="2" />
<Button Name="BARO_BUTTON" Pin="5" />
<Button Name="ARM_BUTTON" Pin="6" />
<Button Name="UP_BUTTON" Pin="7" />
<Button Name="DN_BUTTON" Pin="8" />
<Button Name="ALT_BUTTON" Pin="9" />
<Button Name="REV_BUTTON" Pin="10" />
<Button Name="APR_BUTTON" Pin="11" />
<Button Name="NAV_BUTTON" Pin="12" />
<Button Name="HDG_BUTTON" Pin="13" />
<Button Name="AP_BUTTON" Pin="14" />
```

8. Assemble. Should be self explanatory. Note that the rp2040 "floats". I use a little sticky stuff to hold it in place. I also use some kapton tape to make sure the screen is insulated, and I tape up the connector wires into bundles.

9. Flash the rp2040 (I usually do this much earlier in the project!) 
    Copy contents of the Community folder (under CC_KAP140_LCD) to your mobiflight community directory.
    Plug in the rp2040 and start mobiflight
    Configure the module using the mfmc in the "Mobiflight Runtime Config" directory (if you didn't follow my pin numbering above for the buttons, adjust there)
    Open the KAP140LCD.mcc file in Mobiflight and give it a test. 
    NOTE the mcc file is written for the WBSim C172! If you are using the base C172 from Asobo you will need to adjust, and some features will not be available.

## Photos
![Image]

    
