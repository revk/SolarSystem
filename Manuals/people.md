# People

Access control is managed using fobs or cards, these are DESFire EV1 (but works with EV2).

These are no intended to be shared with other systems, but can be used for multiple sites on the same management system, e.g. one fob would work multiple doors/sites under [access.me.uk](https://access.me.uk).

## Getting fobs

Fobs and cards are availabel from many sources, including Amazon. Obviously you ideall want a source you can trust, though we are not really aware of *dodgy* DESFire card being on the market, it could theoretially happen.

Whilst EV2 cards also work, the EV1 cards are cheaper. We use these in a more secure mode (AES), not the default (DES).

## Provisioning fobs

**However:** Blank cards you buy won't *just work*. They have to be known to the system, which means the card has a unique random key set and stored in the management system.

This means you need to get cards from the person that manages the control system you are using. For [access.me.uk](https://access.me.uk) that is Andrews & Arnold Ltd. You can also order cards, printed even, and from [A&A](https://cards.aa.net.uk).

If you run your own management system, you, as administrator, have an option to "provision" a new card to set that master key.

You need to have set an NFC reader as *trusted*, once done the *Provision new fob* menu shows. Select this, pick the truisted reader and click provision. You hold the blank fob to the reader whilst the process happens. The page can be loaded on a mobile phone, and this is usually the most convenient way to do this.

### Why?

This may seem like just a scheme to sell cards, but it is not, You can run your own management system anyway. The reason for this step is security. Once a card has a master key set, it is secure, but until then it is possible for a *man in the middle* type attack, even at the card reader itself, or with something that fakes a card, to intercept what is being done, including setting site wide security keys that are used. So compromising one card provisioning process could get access to a whole site.

This means tha this initial setting a master key needs to be done at a known and secure station of some sort.

Once that is done, the cards can be posted to customers, and safely adopted on to the system - which can be remotely talking to the person on site setting up cards, etc. The communications to/from the card is secure using that master key and even intercepting at the radio level would not get the site keys.

However, this does mean you can have a stock of provisioned fobs that are not assigned to specific people, and an admin person on site could then set them up.

## Access levels

Before you can set up any people you need to create access levels. These define a number of features relating to card users. You can make them specific to specific people or classes of people *staff*, *night-shift*, *cleaner*, etc.

|Setting|Meaning|
|-------|-------|
|Name|This is the name of the access class|
|Ignore time restrictions|Nomally the system knows the time, and there is hardware to set this from GPS if no internet, but in the unlikely event of everything breaking, e.g. after a power cut, and no internet, then doors will not know the time. This means no time restricted access is allowed. Setting this can allow access when the time is not known.|
|Tro to log access to fob|The fobs have storage internally which can how a history of access - i.e. which doors and what date/time. At present there is not way to read this, but it could be done if needed for any sort of investigation. Thsi setting tries to log that access, but carries on even if there is a problem (e.g. fob removed too quickly)|
|Try to count access|The fobs also have a simple count of access which can be stored, this tries to update that but carries on if it fails.|
|Commit changes|This is a more serious setting - it requires that access or count updates (as above) must be complete on the card before access is allowed. It can add an noticable delay to opening doors. If this is expected by all users it is not normally a problem, but it could be up to a second even.|
|Allow arming out of hours|This is a really useful setting where people have time restricted access. It allows them to *arm* the system if they are leaving late, when they would be outside normal access times. If not set then the card is not allowed to do anything (including *arm*) outside time profile.|
|Open door in all cases|This is the get out of jail free card for system admins - **use with care** - this means the door is opened if this cards is used regardless of access restrictions. It can mean that the alarm will then be set off, etc. It only makes sense for system admin / director use as a last resort override.|
|Auto expire|This sets teh automatic expiry in days (see below)|
|Days|Each day can have a start and end time in hours and minutes, inclusive. If start is after end then this is from Midnight to end, and from start to midnight. `00:00` to `23:59` is all day.|
|`enter`|This defines the areas for which this card is allowed to enter (open door)|
|`arm`|This defines the areas for which this card can arm (holding 3 seconds)|
|`strong`|This defines the areas for which this card can strong-arm (holding 10 seconds)|
|`disarm`|This defines the areas for which this card can disarm|
|`prop`|This defines the areas for which this card can record a door validly propped open|

### Expiry

The cards are set with an automatic expiry. If set (not `0`) the card is updates every time it is used to be expiring hat many days ahead. If the card is removed too quickly to update then it is likely to be caught on next use, etc - it does not wait for the update (unless Commit is set). The idea is a card could be set for say `7` days expiry meaning only if a card has not been used for 7 days would the system have to *phone home* to confirm it is allowed. Within those 7 days a use would be allowed based on the data on the card even if no internet connection. This makes the usage much quicker.

This has the disadvanage that a door could allow access based on the fob even if access has been restricted on the management system. In practice all access permissions are updated if they have changed on the next use of the card, but again this is normally something the system waits for (unless commit is set), so could take a couple of card uses to manage to udpate it in practice. But this could mean a change in access is not immediate - even if it is it could mee a door unlocking and quickly re-locking if permissions have been changed.

The idea of the expiry is that a lost card cannot be used after a number of days, even in this small window whilst permissions are checked. WHowever, see below re blocking a card.

## Adopting a fob

A provisioned fob can be *adopted* on to a site/AID. There is normally one AID per site but there can be more if some doors need to be more secure. The card can be adopted to more than one site/AID depending on memory (8K cards recommended).

You need a reader set to *admin* to adopt a fob, and once set you see *Adopt a fob* menu (if you have that access permision).

The process is in two steps. First pick the reader and click *Read fob ID*, tap the fob and it shows its ID. If already adopted it shows some known details.

You then enter the persons Name, and select the site/AID and access class to apply. The access class can be changed later. Click *Adopt fob*. Hold the fob for the whole process then remove.

This is best done using a mobile phone to access the page while you stand at the reader/door for setting up.

### Fob settings

Once a fob is adopted you can edit the details related to your site, including changing the name, adding a mobile number, and changing access class per AID.

You can also set a hard expiry date/time if you need. Or remove it.

All of these apply once the card is next used and updated. It means previous access settings may apply until updated, so could allow some access not later to be allowed.

The user list also shows the last use of each fob.

## Blocking a fob

You can block a fob in the fob settings.

This is a special case - the idea is that if you need to block a fob you mean it - i.e. it is lost, and you do not want the small window where the fob is trusted and a door opens, even if it closes a moment later, as that could allow access.

When you block a fob, all the door controls are told the fob ID as part of a small list of blocked fobs. These are then immediately recognised by the door controllers without the need to *phone home*. The fob edit shows that the fob has been seen and blocked so it can then be deleted from the system, as this means the fob itself has been updated to stop further use.

Bear in mind that using the auto expiry also means after a period of time even a lost fob will have to *phone home* before access is allowed.