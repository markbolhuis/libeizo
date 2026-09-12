# libeizo

libeizo is a small linux library to introspect and control Eizo Monitors.

## eizoctl

```
Usage: ./eizoctl <option> [monitor]

Options:
        list            - List all available monitors.
        identify        - Identify the monitor.
        poll            - Poll the monitor for events.
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
