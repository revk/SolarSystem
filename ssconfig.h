// Config settings

#ifndef s
#define s(p,n,d,h)
#endif
#ifndef sk
#define sk(p,n,d,h)
#endif
#ifndef sd
#define sd(p,n,d,h)
#endif
#ifndef b
#define b(p,n,d,h)
#endif
#ifndef i
#define i(p,n,d,h)
#endif

sk(ca, key,, MQTT TLS CA private key PEM);
s(ca, cert,, MQTT TLS CA certificate PEM);
s(mqtt, host,, MQTT hostname);
sk(mqtt, key,, MQTT CA key PEM);
s(mqtt, cert,, MQTT CA cert PEM);
s(mqtt, port, 8883, MQTT port name / number);
sd(sql, config,, SQL connection config file);
sd(sql, database, SS, Database name);
//b(sql,debug,0,Debug); // clashes, added manually
s(ota, host,, OTA hostname);

#undef s
#undef sd
#undef sk
#undef b
#undef i
