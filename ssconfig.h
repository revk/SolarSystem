// Config settings

#ifndef s
#define s(n,d,h)
#endif
#ifndef i
#define i(n,d,h)
#endif

s(keyfile,keyfile.pem,MQTT TLS key file);
s(certfile,certfile.pem,MQTT TLS cert file);
s(cafile,cafile.pem,MQTT TLS client CA file);
i(port,8883,MQTT port);

#undef s
#undef i

