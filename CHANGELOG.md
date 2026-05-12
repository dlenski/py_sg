## Changelog

### 0.17

- I, Daniel Lenski, rediscovered this project in the form of `py3_sg` after having started it in 2008
- Renamed it back to `py_sg`
- Re-renamed the `read` function back to `read`, but dropped the option to
  use a buffer object as destination.
- Added output `buf` to the `SCSIError` contents
- Added `read(..., force_size=True)` option to work around SCSI bugs
- Cleanup and packaging

### 0.16

- Use static configuration file

### 0.14

- Missing PY_SSIZE_T_CLEAN macro added, fix for Python 3.10 version

### 0.13

- **breaking** change in API - split read function into 2. Get rid of deprecated function.

### 0.12

- migrate to Python3, one deprecated function used

### 0.11

- original Dan Lenski [code](http://tonquil.homeip.net/~dlenski/py_sg) (inactive link)
