// Config settings

#ifndef s
#define s(p,n,d,h)
#endif
#ifndef i
#define i(p,n,d,h)
#endif

s(ca,key,cakey.pem,MQTT TLS client CA file);
s(ca,cert,cacert.pem,MQTT TLS client CA file);
s(mqtt,key,mqttkey.pem,CA key file);
s(mqtt,cert,mqttcert.pem,CA cert file);
i(mqtt,port,8883,MQTT port);
s(sql,config,,SQL connection config file);
s(sql,database,SS,Database name);

#undef s
#undef i

