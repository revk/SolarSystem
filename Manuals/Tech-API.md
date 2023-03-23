# API access

The Solar System control server allows access via an API.

API access is configured as a user setting (per organisation), and there there are individual API functions which can be enabled. There may be more fine tuned controls in future as more API functions are defined.

## API using curl

The URL for the API is `/api.cgi/organisation`, e.g. `https://access.me.uk/api.cgi/2` for organisation `2`.

It is necessary to POST an application/json object to the API. The object needs to contain a `“command”` field.

The user authentication is HTTP (over https).

The response will be `2XX` if valid.

Using curl, this could be achieve by, for example :-

```
echo '{"command":"expires","fob":"041981FA186280","expires":"2021-08-21"}' | curl -u test@nojunk.uk:SillyPassword https://dev.access.me.uk/api.cgi/1 -v -HContent-Type:application/json --data-binary @-
```

## Defined APIs

### Expiring a fob

The `expires` command can be used to expire a fob at a specified date/time.

|parameter|Meaning|
|---------|-------|
|`command`|`expires`|
|`fob`|The fob ID, e.g. the hex ID string, scuh as `041981FA186280`|
|`expires`|The date to set the expiry of the fob|

Note that this command sets the expiry in the management system. The fob itself may briefly work until updated to the new expiry, so this works best in advance where the fob can learn its new future expiry date before that date is reached.

## Other ideas

We have other ideas, and welcome feedback. E.g.

- blacklist a fob
- arm/disarm
- Get status

