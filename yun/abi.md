# YUN ABI

**Optimized MQTT library for the Arduino Yun that uses a client process on the linux processor.**

Command | Description | Format
---|---|---
→ | connect | `c:(host):(port):(user):(pass)`
← | connection approved | `ca`
← | connection denied | `cd`
→ | subscribe | `s:(topic)`
→ | unsubscribe | `u:(topic)`
→ | publish | `p:(topic):(data)`
← | message | `m:(topic):(data)`
→ | disconnect | `d`

Todo:

- length prefix data (may contain `:`)
- add client id
