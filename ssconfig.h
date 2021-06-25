// Config settings

#ifndef s
#define s(p,n,d,h)
#endif
#ifndef s
#define sd(p,n,d,h)
#endif
#ifndef i
#define i(p,n,d,h)
#endif

s(ca, key,, MQTT TLS CA private key PEM);
s(ca, cert,, MQTT TLS CA certificate PEM);
s(mqtt, host,, MQTT hostname);
s(mqtt, key,, MQTT CA key PEM);
s(mqtt, cert,, MQTT CA cert PEM);
s(mqtt, port, 8883, MQTT port name / number);
sd(sql, config,, SQL connection config file);
sd(sql, database, SS, Database name);
s(ota, host,, OTA hostname);

#undef s
#undef sd
#undef i
