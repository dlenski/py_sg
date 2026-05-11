from typing import Union, Optional
from io import RawIOBase

class SCSIError(Exception):
    """SCSI operation failed.

    The args are a 5-tuple containing the masked_status,
    host_status, driver_status, sense buffer, and data buffer fields
    from the failed operation."""

    ...

def write(
    fd: Union[int, RawIOBase],
    cmd: bytes,
    buf: Optional[bytes] = None,
    timeout_ms: int = 20_000,
    flags: int = 0,
) -> None:
    "Issue a command and write data.  Returns nothing."
    ...

def read(
    fd: Union[int, RawIOBase],
    cmd: bytes,
    bufLen: int,
    timeout_ms: int = 20_000,
    flags: int = 0,
) -> bytes:
    """Issue a command and read a response.
    Response is returned as bytes."""
    ...
