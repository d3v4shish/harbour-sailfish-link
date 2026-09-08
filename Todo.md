# harbour-sailfish-link TODO

## 0. Tracking Rules
- [x] Keep this file focused on Sailfish-side discovery, device identity, and mother-PC announcement.
- [ ] Mark objectives done only after the service survives real Wi-Fi reconnects, IP changes, and restarts.
- [x] Keep v1 scoped to local Wi-Fi/LAN discovery with no pretend support for a fully offline phone.
- [x] Keep service-grade expectations visible here: logging, visible config, user control, and documented recovery.

## 1. Device Identity And Service Presence
- [ ] Define one stable device identity that survives IP changes and normal reboots.
- [ ] Package the link component as a real Sailfish service with an intentional user-facing control surface.
- [ ] Make the service status inspectable from the device without reading raw logs.
- [ ] Keep the identity model simple enough to debug and rotate if needed.

Acceptance criteria:
- The service has a stable name or identifier that the mother PC can trust across sessions.
- The user can tell whether the service is enabled, disabled, or currently inactive.
- Identity details are not hidden in code-only constants.

## 2. Wi-Fi And LAN Discovery Eligibility
- [ ] Announce only when the phone has a usable LAN address and the relevant interface is actually up.
- [ ] Detect IP changes, Wi-Fi reconnects, and interface loss without requiring manual restarts.
- [ ] Handle multiple addresses or interfaces predictably instead of advertising nonsense.
- [ ] Report honestly when the phone cannot currently be discovered.

Acceptance criteria:
- The service does not announce stale or unreachable addresses after a network change.
- Rejoining Wi-Fi causes the next discovery cycle to reflect the new address.
- If Wi-Fi is absent, the device surface says discovery is unavailable rather than pretending otherwise.

## 3. Mother-PC Announcement
- [ ] Publish the current IP, SSH target, and service endpoints the mother PC needs.
- [x] Keep the announcement payload versioned and small enough to parse without guesswork.
- [ ] Define a predictable announce cadence with sane retry behavior.
- [x] Keep the announcement path local-first and easy to inspect during development.

Acceptance criteria:
- The mother PC can learn enough from one fresh announcement to reach the phone again.
- The discovery payload has stable field names and a visible version marker.
- Lost packets or short outages recover without manual state edits.

## 4. Persistence And Recovery
- [ ] Start the service automatically after the appropriate boot or login point.
- [ ] Recover cleanly from service restart, process crash, or transient network loss.
- [ ] Preserve small, useful last-known state such as last success time and last published address.
- [ ] Avoid leaving behind stale runtime artifacts that confuse the next launch.

Acceptance criteria:
- Normal reboot restores the service without manual shell work.
- A killed service can be restarted and resumes publishing current state.
- Last-known state helps diagnosis but does not override current network truth.

## 5. Security And Trust Boundaries
- [ ] Require an explicit pairing token or equivalent trust gate between phone and mother PC.
- [ ] Keep the service bound to the local trust model instead of exposing unauthenticated control data.
- [ ] Support token rotation or pairing reset without reinstalling the app.
- [ ] Keep secrets, identifiers, and logs separate enough that accidental disclosure is obvious.

Acceptance criteria:
- The user can tell whether the phone is paired and can revoke that trust.
- Untrusted listeners do not get privileged device details by default.
- Pairing state can be reset cleanly from the device side.

## 6. User Operability And Control
- [ ] Provide an on-device surface to enable, disable, inspect, and troubleshoot the link service.
- [x] Add a first-run explanation of what the service does and what it does not do.
- [x] Make config paths, log paths, and pairing state visible to the user.
- [x] Keep the operational UI small and explicit rather than burying control in hidden settings.

Acceptance criteria:
- A user can intentionally enable or disable discovery without SSH.
- The user can see current reachability state, last successful announce, and pairing state.
- A new user can understand the mother-PC model from the app or included docs.

## 7. Mother-PC Compatibility Contract
- [x] Define the exact fields the Linux helper is allowed to depend on.
- [x] Keep endpoint naming aligned with the actual webcam and LLs remote projects.
- [x] Version the payload so future changes do not silently break older helpers.
- [ ] Surface compatibility problems as visible errors instead of quiet failure.

Acceptance criteria:
- The Linux helper can parse the payload without scraping human-readable logs.
- A mismatch between helper and phone surfaces a useful version or capability error.
- Discovery data stays stable enough to script against.

## 8. Service Reliability And Background Behavior
- [ ] Keep the service useful when the UI is closed.
- [ ] Keep idle CPU, network, and battery overhead low enough for background use.
- [ ] Handle suspend, resume, and intermittent Wi-Fi without entering a wedged state.
- [ ] Keep the runtime design small enough that background bugs can be diagnosed.

Acceptance criteria:
- The service continues to function after the user leaves the foreground UI.
- Long-running operation does not require periodic manual restarts.
- Resume or reconnect events do not duplicate announcements indefinitely.

## 9. Observability And Supportability
- [ ] Write logs to a known user-visible path.
- [ ] Make the service expose last announce time, last error, and current eligibility state.
- [ ] If cache or state files accumulate, show their size and let the user clear them intentionally.
- [x] Document the service files and cleanup behavior in a README.

Acceptance criteria:
- A user or developer can find logs and current state quickly.
- Support artifacts are visible and deletable instead of hidden forever.
- The service state is inspectable without attaching a debugger.

## 10. Validation And Release Readiness
- [ ] Package the service as an installable Sailfish RPM with the right service metadata and user surface.
- [ ] Validate discovery from a Linux mother PC after reboot, Wi-Fi reconnect, and IP change.
- [ ] Keep developer deployment and packaged deployment behavior aligned.
- [x] Document install, pairing, recovery, and uninstall paths.

Acceptance criteria:
- The packaged service starts and behaves like the development build.
- The mother PC can rediscover the phone after address changes without manual subnet scanning.
- A developer can install, test, remove, and reinstall the service with explicit steps.
