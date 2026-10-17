# STORM MODS 2.0

## Install the matching pair
This Android project and the STORM_KURO_PANEL_2.0.zip panel are a matched release. The new app verifies RSA-signed responses using a public key embedded in LicenseClient.java. Its matching private key lives only in the panel's app/Config/StormSigning.php. Do not use an older unsigned endpoint with this app, and do not copy the private signing file into the Android project.

1. Deploy the panel to stormxp999.com's document root and finish its setup page. Use the setup code in the panel ZIP's INSTALLATION.txt.
2. In your panel, generate an ACTIVE key for FreeFire. Keep device limits and duration valid.
3. Extract this app project into a NEW folder and open its STORM_MODS directory in AndroidIDE.
4. Build and install the new APK. The version is 2.0 (versionCode 2). Use the existing working SDK/NDK and Gradle environment. Both foreground-service permissions from the verified startup fix are included.
5. Enable automatic date and time on the phone; use the license in the opening screen. After a successful response, choose root/version and grant overlay permission.

## UI changes
Original transparent STORM MODS mascot logo, navy/cyan/violet theme, textured static background, two-step access/setup card, short transitions, horizontal floating tabs, larger touch targets, draggable title, adjustable panel opacity, and reset-position button in the Display tab.
The root and version options remain those supported by your existing project. This release adds no new game or daemon features.

## License behavior
HTTPS with normal certificate validation, bounded responses, network timeouts, nonce-bound RSA signatures, response device/key/game checks, a short response validity window, expiry/blocked/device-limit checks on the server, and no stored license key. Reopening the app requires a new online verification; the floating UI uses the already verified session.
Response signatures make forged responses harder to use against the unmodified app. They do not make an APK uncrackable. An owner of a device can inspect or modify client code. This package includes no hiding, anti-cheat bypass, or detection-evasion features.
The signing response window checks freshness at login. It is not a live daemon revocation/expiry mechanism; the daemon is unchanged.

## Build and release
SDK 34 / NDK 24.0.8215888 / Gradle 8.2 remain as supplied. Existing R8 release shrinking and JNI keep rules remain. The inherited project signs release builds with the debug signing configuration: use your OWN release keystore before public distribution. No keystore or signing password has been invented for you.

## Verified here
15 Java files parsed by the JDK compiler parser; all source XML parsed; signature accepted with the matching public key and tampering rejected using Java's SHA256withRSA implementation; every JNI/native file plus Initialize.java and RootServices.java compared byte-for-byte to your upload; complete ZIP extraction and CRC checks.

## Still needs your device/hosting
No Android SDK/NDK or PHP runtime is available in this environment. Full APK compilation, Android rendering, overlay interactions, PHP execution, MySQL integration and live domain login were NOT tested. No error-free or untraceable claim is made.

For first testing: valid key, invalid key, expired key, blocked key, second device beyond limit, offline, incorrect phone time, reopening app and opening the floating Display tab. Send the exact screen message if a check fails.
