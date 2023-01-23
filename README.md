# Aviation_Synchro_Multiturn_Indicator
Aircraft pressure gauge, synchro repeater. 115V 400Hz power, 26V Synchro input

![Front](./images/Front.jpg)

![Simplified Schematic](./images/Simplified_Schematic.png)


|Pin|Function   | |
|---|-----------|-|
|A  |165 R to B |Synchro R1, 115V 400Hz live|
|B  |Chassis Common  |SYnchro R2, 115v 400Hz return|
|C  |624R to D&E|Synchro S1 (26v)|
|D  |624R to C&E|Synchro S2 (26v)|
|E  |624R to C&D|Synchro S3 (26v)|
|F  |n/c||
|G  |n/c||
|H  |n/c||
|I  |n/c||
|J  |n/c||
|K  |n/c||
|L  |n/c||
|M  |n/c||
|N  |n/c||

[photos](./images/)

[Overhaul Manual](./documents/Overhaul-SynchroMultiturnIndicator.pdf)

[Youtube video](https://youtu.be/Hs32SN3N2VU)

[More instruments](https://github.com/DavidJRichards/Aviation_Instruments)

![Notes](./images/Notes.jpg)

![Connector](./images/Connector.jpg)




## Arduino sketch to drive indicator (WIP)

This arduino sketch generates three phase 400Hz sine waves to feed into an amplifier, the output is then fed into the indicator sysnchro.

The output signals are PWM using 256 step approximation to sine waves.

A potentiometer is used to supply a variable DC voltage to the ADC0 input of the Arduino, this is used to provide the simulated shaft angle of the synchro generator.

The shaft angle control varies the amplitude and polarity of the pwm sine waves to simulate a synchro transmitter.

A means is neded to synchronise the main system 400Hz power supply to the sysnch simulation is needed.

The sine wave PWM outputs are filtered with 47K and 100n RC lowpass filter before being fed to a stereo power amplifier.

Two of the variable sinewave outputs are fed into the S1 and S2 synchro coils, the S3 coil is the common ground terminal of the amplifier. The third output is not used.

A 400 Hz square wave is available on Pin 8 to synchronise the power supply (todo)

[Arduino synchro transmitter sketch](./pwm_synchro_transmit/pwm_synchro_transmit.ino)

![Arduino prototype](./images/arduino_synchro_gen.jpg)




