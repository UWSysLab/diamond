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
* `pyscrabble-diamond-old` is Niel's original Diamond port of PyScrabble.
It doesn't work with the latest versions of Diamond.

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
