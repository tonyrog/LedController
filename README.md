# LedController

Arduino UNO driver for
Biltema LED controller 46-3384/46-3385

The 46-3384 is 20 chips controlling 3 RGB leds each.
The 46-3385 is an extension with 30 chips controlling 3 RGB each.

The hookup is super simple. Drive the LEDs with 12v on the
specified pin. And connect the Din pin to MOSI (pin 11 or on
the SPI pin header) 5v. The ground must be connected and
bridge to a common ground (I belive)

