# Directory contents
This directory contains a bunch of different versions of PyScrabble.

* `pyscrabble-1.6.2` contains the original PyScrabble source code, with
a few small bug fixes.
* `pyscrabble-hatchet-original` is a version of PyScrabble with several extraneous
features (chat rooms, customization of various kinds, etc.) cut out.
* `pyscrabble-hatchet` is a version of PyScrabble with all of the cuts described
above as well as additional changes (mostly refactoring the tile/letter code)
designed to make the Diamond port easier.
* `pyscrabble-hatchet-diamond` is a complete Diamond port of `pyscrabble-hatchet`.

If you want to do experiments or make comparisions between PyScrabble with and without
Diamond, use `pyscrabble-hatchet` and `pyscrabble-hatchet-diamond` for a fair
comparision.

# Running the Diamond version of PyScrabble
As mentioned above, `pyscrabble-hatchet-diamond` is the definitive Diamond
version of PyScrabble. The original PyScrabble included an interface for
adding and logging onto different servers, but the Diamond version of
PyScrabble simply connects to the Diamond frontend server specified in the
`DiamondInit()` call in `pyscrabble-main.py`. To create an account:

1. Press the "Find Public Servers" button on the PyScrabble login screen.
2. Click the "Register" button (ignore any server names shown in the listing).
3. Fill out the registration dialog.
4. Click the "Create" button.
5. The registration dialog does not disappear after successfully creating an
account, due to a bug. Click the "Cancel" button to exit the registration dialog.
6. You can now use your username and password to log into PyScrabble running on Diamond.

# Work still to be done on PyScrabble
The only work that really needs to be done on the Diamond version of PyScrabble is to
add callbacks to all of the read/write transactions to re-run them if they abort.
Aborts should be very rare in practice, though, since the app handles concurrency control
during the game, and the only points of contention are in joining games and creating new
games. As long as you're not running PyScrabble at scales where multiple players doing
these actions simultaneously is likely, the game should work fine without the retry callbacks.

There may also be a few features from `pyscrabble-hatchet` that still have not been
ported to `pyscrabble-hatchet-diamond` (such as some game statistics and options). Niel
believes that the port is virtually complete, and that any features that were not ported would
not significantly change the LoC of the Diamond version if they were added.
