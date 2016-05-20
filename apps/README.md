# Applications
This directory contains several apps either built from scratch for Diamond or ported
to Diamond. Most apps also have a "baseline" version that does not use Diamond
for data management (in the case of ported apps, the original version of the app).
See the README in each app's directory for more information about the different
versions of that app.

## Porting apps to Android Studio
Niel originally started working with the Android apps in Eclipse, but Google has
since dropped support for Eclipse in favor of Android Studio. As a result, we'd
like to move all of the Android apps over to Android Studio. Moving apps to
Android Studio is mainly a matter of changing the project directory structure
to fit Android Studio's requirements, but you might also have to add some metadata
files.

Android apps still set up for Eclipse:

* `chat/AndroidChatBaseline`
* `chat/AndroidChatBaselineBenchmark`
* `chat/AndroidChatBenchmark`
* `chat/DiMessage`
* `chat/DiMessage-reactive`
* `notes/NotesReactive`
* `notes/NotesUnreactive`
* `test-apps/DiamondAndroidTest`
* `twitter/twimight-hatchet`
* `twitter/twimight_ng`
* `twitter/twimight-diamond`

Android apps set up for Android Studio:
