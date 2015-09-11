# Yun ABI

**Optimized MQTT library for the Arduino Yun that uses a client process on the linux processor.**

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
