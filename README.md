# libeizo

libeizo is a small linux library to introspect and control Eizo Monitors.

## eizoctl

```
Usage: ./eizoctl <option> [monitor] [input-port]

Options:
        list            - List all available monitors.
        identify        - Identify the monitor.
        poll            - Poll the monitor for events.
        input           - Read or set the input port (see 'help input').
        descriptor      - Read the eizo specific HID report descriptor.
        key-pairs       - Read the unknown key value pairs behind usage ff300009.
        read-all        - Read all values for every usage.
        eeprom          - Read the contents of the internal EEPROM.
        custom-key-lock - Read the available custom key locks, and the currently used one.
        gain-definition - Read all available gain definition values.
        edid            - Read the monitor edid.
        usage           - Read the the number of hours and minutes the monitor has been used.
        debug           - Put the monitor into 'debug' mode.
        help            - Show this help message.
```

```
Usage: ./eizoctl input <monitor> [port]

Read or set the active input port of a monitor.
Omit [port] to read the current input.

Available port names:
  dsub1, dsub2, dvi1, dvi2, dp1, dp2,
  hdmi1, hdmi2, sdi1, sdi2, sdiab, sdiba,
  d1, d2, composite1, composite2, tv1, tv2, usbc

Not all ports are supported by every monitor.
Input switching may fail if the requested port is not supported or the monitor is in power-saving mode.
```
