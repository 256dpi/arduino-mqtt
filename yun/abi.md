# YUN ABI

**Optimized MQTT library for the Arduino Yun that uses a client process on the linux processor.**

Command | Description | Format
-|-|-|-
OUT | connect | c:(host):(port):(user):(pass)
IN | connection approved | ca
IN | connection denied | cd
OUT connected? | c?
IN | is connected | c+
IN | not connected | c-
-|-|-|-
OUT | subscribe | s:(topic)
OUT | unsubscribe | u:(topic)
-|-|-|-
OUT | publish | s:(topic):(data)
IN | message | m
-|-|-|-
OUT | disconnect | d
