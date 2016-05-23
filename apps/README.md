# Applications
This directory contains several apps either built from scratch for Diamond or ported
to Diamond. Most apps also have a "baseline" version that does not use Diamond
for data management (in the case of ported apps, the original version of the app).
See the README in each app's directory for more information about the different
versions of that app.

## Porting Android apps to Android Studio
We originally started working with the Android apps in Eclipse, but Google has
since dropped support for Eclipse in favor of Android Studio. As a result, we've
moved most of the Android apps over to Android Studio.

These Android apps are set up for Android Studio:

* `chat/AndroidChatBaseline`
* `chat/AndroidChatBaselineBenchmark`
* `chat/DiamondChatBenchmark`
* `chat/DiamondChat`
* `parse/DiamondParse`
* `parse/Parse-Starter-Project-1.13.0/` (the baseline Parse app)
* `test-apps/DiamondAndroidTest`
* `twitter/twimight-hatchet`

These Android apps are still set up for Eclipse:

* `notes/NotesReactive`
* `notes/NotesUnreactive`
* `twitter/twimight_ng`
* `twitter/twimight-diamond-old`

Here are some instructions for moving apps from Eclipse to
Android Studio, using Android Studio's import feature:

1. On the Android Studio splash screen, click "Import project (Eclipse ADT, Gradle, etc.)."
2. Go through the wizard, selecting the Eclipse project directory and then a location
for the new Android Studio project directory (the import wizard leaves the Eclipse project
untouched and makes a new directory).
3. Copy over the gitignore file from `test-apps/DiamondAndroidTest` (or any of the other
Android apps already ported to Android Studio) into the new Android Studio project directory.
4. Add the new project directory to Git.
