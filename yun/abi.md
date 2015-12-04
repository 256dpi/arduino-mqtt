# Yun ABI

**Optimized MQTT library for the Arduino Yun that uses a client process on the linux processor.**

The following commands are exchanged between the python script and the arduino library:

   | Command     | Format
---|-------------|------------------------------------
<- | boot        | `b;`
-> | will        | `w:topic:payload_len;(payload)`
-> | connect     | `c:host:port:id:(user):(pass);`
<- | approved    | `a;`
<- | rejected    | `r;`
-> | subscribe   | `s:topic;`
-> | unsubscribe | `u:topic;`
-> | publish     | `p:topic:payload_len;(payload)`
<- | message     | `m:topic:payload_len;(payload)`
-> | disconnect  | `d;`
<- | closed      | `e;`

All commands end with a standard line break `\n`.
