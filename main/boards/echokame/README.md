# EchoKame

An [EchoEar](https://oshwhub.com/esp-college/echoear) (ESP-VoCat) mounted on top of a [Kame32](https://github.com/JavierIH/kame32) quadruped robot. The robot moves according to the emotion shown on the screen, and wakes up or shuts down when the EchoEar is attached or removed.

More details and 3D parts: https://javierih.com/robots/echoearxkame32/

## Hardware

The board is a standard EchoEar, everything in [espressif/esp-vocat](../espressif/esp-vocat/README.md) applies. The only addition is the serial link to the robot, through the UART of the magnetic connector.

| EchoEar | Kame32 |
|---|---|
| UART1 TX (GPIO6 on PCB V1.0, GPIO5 on V1.2) | RX2 (GPIO16) |
| GND | GND |

The PCB version is detected at boot, so the same firmware works on both.

## Protocol

One way, from the EchoEar to the robot. 115200 baud, 8N1, one message per line.

| Message | Sent |
|---|---|
| `E:<emotion>` | every time the screen shows an emotion, for example `E:happy` |
| `H` | every 500 ms |

The robot uses `H` as a heartbeat to know if the EchoEar is attached.

## Build

```bash
python scripts/build.py echokame
```

The robot side is the `echoear` app of the Kame32 firmware.
