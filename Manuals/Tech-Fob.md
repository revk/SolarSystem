# NXP MIFARE DESFire NFC

The SolarSystem uses DESFire EV1 cards or fobs to control access (EV2, etc are compatible, but only EV1 features are used). These fobs are formatted with specific data for use with the system.

## Type of fob

The system is designed to work with a DESFire EV1 fob or card, and can work within only 2k - however this only allows one site/AID access control to be stored. A 4k or 8k is recommended if more than one site/AID is required. EV2 cards are compatible and can be used if necessary.

## Master Key

The fobs are intended to be used with one SolarSystem cloud management system, but could be used for multiple sites or organisations within that.

The fob must be provisioned for the system on a secure station - this involves setting the fob to AES mode and setting a master AES key for the fob. Once provisioned, other operations can be done securely at any station connected to the system.

The fact we know / set the master AES key and fix the fob to AES mode does limit its use as shared with other system. The main design concept is that it is usable for multiple sites and organisations under a common system, e.g. https://access.me.uk/

But this means that you cannot simply buy blank fobs and adopt them on to the system as a user - you need administrator access to provision the fob. For https://access.me.uk/ you can order printed DESFire 8k EV1 cards which are provisioned on the system from https://cards.aa.net.uk/

The master key settings are set to `0x09`, but could be locked down further if required.

## AID

Each site may have one (or more) AID defining an application on the fob. Each application has a master AES key (key `0`) which is unique to the card and AID - this is used to control files in the application.

The application key settings are `0xEB`, but could be locked down further if required.

The application has a second key (key `1`) which is a site wide key for the AID, and is also store in corresponding door controllers. This allows the fob to be authenticated and some specific read and write operations to be performed.

All files are created with comms mode `0x01` (CMAC communications required).

The card is expected to have an access control file.

|File|Details|
|----|-------|
|`0A`|This is a fixed size 256 byte file, configured in backup mode to ensure consistency. The actual content is variable length with the first byte defining the number of bytes that follow. This allows the file to be changed in-situ and change content length as needed. It also allows the file to be read without first finding its length, making operations faster - for example, to read the file an initial read of a smaller amount can be done, and then only if that was not the whole file is more read as a second operation.|

Additional files may be present. The access file defines if these files are used or not - if set in the access file then they should be present in the application. As the access file can be changed later, it is recommended that these are created.

|File|Details|
|----|-------|
|`00`|Fob holder name, variable length|
|`01`|Log, a cyclic record file with record size of 13. Records hold 6 byte device chip-id followed by BCD coded `YYYMMDDHHMMSS` UTC access time. Needs to be writable.|
|`02`|Usage counter. Needs to be writable (allow credit 1).|

## Access file

The access file is a single file that contains access control data. This would more logically be a series of files for different purposes, but by making a single file it reduces the number of card accesses.

Logically the file is variable length, however DESFire cards do not lend themselves to such files. To read the file means checking the size first, and changing the file would mean deleting and re-creating the file, which uses up EEPROM. To address this the file format is a 256 byte backup type file which has as the first byte a length of following data. This allows an initial block, say 40 bytes, to be read, and then read more (the rest of the actual file) to be read only if needed, without first checking file size. It also allows the content to be overwritten as needed without wasting EEPROM - but done as an atomic operation (as a *backup* format file).

After the one byte length of following data, the file then contains a number of tagged fields, each has one byte with top 4 bits as type and bottom 4 bits as length of following data, allowing up to 15 bytes for each tag. The exception of `FX` which are always one byte flags.

This file is type *backup* to ensure integrity, Read and write by key `1` to allow reader access, and set for CMAC mode to ensure writes are CMAC authenticated to avoid any man-in-the-middle attacks.

### Areas

Area codes are normally letters `A` to `Z`. Each letter has a bit starting with the top bit of each byte, and any trailing `00` bytes omitted. Area `A` is the top/first bit. So 1 byte `80` means just area `A` set, and 2 bytes `80 0A` means areas `A`, `N` and `O` set. The area list starts `A` to `Z` but may have other characters added after `Z` in future.

### Time ranges

Time ranges are defined by setting set of *from* times and a set of *to* times. Logically for a whole week this means 7 sets of 2 bytes (14 bytes total). However, as this is commonly the same every day, or the same weekdays, etc, there is a compressed storage of these values. The *from* and *to* can be separately compressed (they don't have to be the same length coding as each other). The length of the *from* or *to* field specifies how it is coded as follows.

|Type/length|Meaning|
|-----------|-------|
|`X2`|All days same time|
|`X4`|Weekend + Weekdays|
|`X6`|Sun + Weekday + Sat|
|`X8`|Sun + Mon-Tru + Fri + Sat|
|`XE`|Sun + Mon + Tue + Wed + Thu + Fri + Sat|

The *from* time is from the beginning of the `HHMM` specified for th current day. The *to* time is up to the moment before the specified `HHMM` for the current day. A value of `2400` is valid as *end of day*.

If *to* is before *from* on any day, then access is valid on, or after, the *from* time or before the *to* time, otherwise access is only valid on, or after, the *from* time and before the *to* time.

Set *from* and *to* to `0000` for no access on the specified day(s).

### Access file tagged fields

The full list of tagged fields in the access file is as follows. Note that `FX` fields are always one byte flags and have no additional data. All others have the length in the low 4 bits, shown as `X` where allowed.

|Tag|Meaning|
|---|-------|
|`00`|**End**: No further bytes considered|
|`0X`|**Zero**: Padding, ignore, but go on to next record after padding.|
|`1X`|**From**: Times of day BCD coded as `HHMM`. This is the start time allowed.|
|`2X`|**To**: Times of day BCD coded as `HHMM`. This is the end time allowed.|
|`31`|**Expiry**: Folled by one byte number of days (1-255) to auto update expiry date when access allowed. The `3X` expiry should also be set, ideally as just a date (`34`), and this is updated in-situ with new date when used.|
|`3X`|**Expires**: BCD coded `YYYYMMDDHHMMSS` expiry, or truncated part thereof. Expiry is end of specific time, e.g. `201906` means end of Jun 2019. Cannot just be `YY` (as would clash with `31` meaning). This is UTC.|
|`40`|**Name**: Indicates a separate *name* file exists (file `00`). This file will be the length of the name, and not writable.|
|`4X`|**Name**: This is the users name, no name file exists. Allows up to 15 characters of name in the access file, thus meaning it can be updated from server if needed.|
|`9X`|**Number**: BCD coded, high nibble first, `F` padded, mobile number E.123 formatted.|
|`AX`|**Arm**: A bit pattern of areas which can be armed.|
|`BX`|**Strong**: A bit pattern of areas which can be forced arm, ignoring `TAMPER` and `ACCESS`.|
|`CX`|**Prop**: A bit pattern of areas which can be propped open using this fob.|
|`DX`|**Disarm**: A bit pattern of areas which can be disarmed.|
|`EX`|**Enter**: A bit pattern of areas which can be entered (if disarmed).|
|`F0`|Ensure card updates are done before allowing access, including log and count if needed. This makes card use slower.|
|`F1`|Log access to the card in the log file (file `01`)|
|`F2`|Count access to the card in count file (file `02`)|
|`FA`|Arm outside times - allow arming as per `DX`, even if outside time periods|
|`FB`|Block - do not allow this card (this is more severe than simply having `A0` as that could allow access if no door areas are set).|
|`FC`|Ignore time controls if clock not set - use with care|
|`FF`|Override any other controls and unlock the door regardless|
