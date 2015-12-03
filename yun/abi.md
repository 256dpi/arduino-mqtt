# Yun ABI

**Optimized MQTT library for the Arduino Yun that uses a client process on the linux processor.**

The following commands are exchanged between the python script and the arduino library:

   | Command     | Format
---|-------------|-----------------------------------------
-> | will        | `w:topic:(payload)`
-> | connect     | `c:host:port:id:(user):(pass)`
<- | approved    | `a`
<- | rejected    | `r`
-> | subscribe   | `s:(topic)`
-> | unsubscribe | `u:(topic)`
-> | publish     | `p:(topic):(data)`
<- | message     | `m:(topic):(data)`
-> | disconnect  | `d`
<- | closed      | `e`

All commands end with a standard line break `\n`.
