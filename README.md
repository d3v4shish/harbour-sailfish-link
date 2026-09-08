# Sailfish Link

Sailfish Link announces a trusted Sailfish phone to a Linux computer on the
local network. It combines a Sailfish UI with a small user service that emits
versioned UDP discovery metadata for SSH, Webcam, and LLs Remote handoffs.

## Use it

1. Open **Sailfish Link** on the phone.
2. Confirm the device name, SSH user/port, and announcement port.
3. Review the pairing token and enable **Discovery enabled** only on a trusted
   LAN.
4. Run the Linux-side `sailfish-mother-pc-helper` to receive the announcement.
5. Verify the shown device ID on Linux and explicitly trust it before using an
   advertised endpoint.

The UI shows the selected interface, current IP address, last announcement,
and any service error. Use **Rotate token** when a pairing secret must be
replaced.

## Discovery contract

Announcements use UDP port `45177` by default and multicast group
`239.255.77.77`. The service selects a private IPv4 address, prefers Wi-Fi,
and sends both directed broadcast and interface-pinned multicast with TTL 1.

The v1 JSON payload includes:

- a stable phone ID and display name;
- the current network address and SSH handoff details;
- Webcam MJPEG/status URLs;
- LLs Remote control/status URLs; and
- a token hint only.

Discovery messages are not authenticated control traffic. Treat them as
untrusted until the receiving computer has verified and pinned the device ID.
The full pairing token is never broadcast.

## Service and files

The installed user service is `harbour-sailfish-link.service`. Configuration,
status, and logs use standard Sailfish writable locations under the owning
user's config and data directories. They are created with owner-only
permissions because the configuration contains the pairing token.

## Build

Build with the matching Sailfish SDK target, for example:

```sh
sfdk -c target=SailfishOS-3.2.0.12-armv7hl build
```

The armv7hl RPM is written to `RPMS/`.

## License

MIT. See [LICENSE](LICENSE).
