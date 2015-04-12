# Yun ABI

**Optimized MQTT library for the Arduino Yun that uses a client process on the linux processor.**

  | Command     | Format
--|-------------|-----------------------------------------
→ | connect     | `c:(host):(port):(id):(user):(pass)`
← | approved    | `ca`
← | denied      | `cd`
→ | subscribe   | `s:(topic)`
→ | unsubscribe | `u:(topic)`
→ | publish     | `p:(topic):(data)`
← | message     | `m:(topic):(data)`
→ | disconnect  | `d`
← | closed      | `e`

ToDo:
- length prefix data (may contain `:`)
