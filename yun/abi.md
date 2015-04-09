# YUN ABI

**Optimized MQTT library for the Arduino Yun that uses a client process on the linux processor.**

Command | Description | Format
---|---|---
→ | connect | `c:(host):(port):(user):(pass)`
← | connection approved | `ca`
← | connection denied | `cd`
→ | connected? | `c?`
← | is connected | `c+`
← | not connected | `c-`
→ | subscribe | `s:(topic)`
→ | unsubscribe | `u:(topic)`
→ | publish | `s:(topic):(data)`
← | message | `m`
→ | disconnect | `d`
