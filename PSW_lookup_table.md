# Program Status Word (PSW) Lookup Table

This table details the meaning of each bit and bit-field within the 32-bit `g_programStatusWord`. The `g_programStatusWord` provides a concise summary of the program's execution flow and critical events.

---

## Status Word: `uint32_t g_programStatusWord`

| Bit Position (N) | Flag Macro / Field Name            | Mask (Hex)     | Description                                                          |
|------------------|------------------------------------|----------------|----------------------------------------------------------------------|
| **Bits 0-4: Critical & General Errors**                                                                                                            |
| 0                | `STATUS_CRITICAL_ERROR`            | `0x00000001`   | General critical error, usually leads to shutdown.                   |
| 1                | `STATUS_FILE_IO_ERROR`             | `0x00000002`   | Error during file read/write.                                        |
| **Bits 2-4: EGDB Initialization**                                                                                                                  |
| 2                | `STATUS_EGDB_INIT_START`           | `0x00000004`   | Endgame Database (EGDB) initialization process began.                |
| 3                | `STATUS_EGDB_INIT_OK`              | `0x00000008`   | EGDB initialized successfully.                                       |
| 4                | `STATUS_EGDB_INIT_FAIL`            | `0x00000010`   | EGDB initialization failed.                                          |
| **Bits 5-19: EGDB Lookup & Results**                                                                                                               |
| 5                | `STATUS_EGDB_LOOKUP_ATTEMPT`       | `0x00000020`   | An EGDB lookup was attempted.                                        |
| 6                | `STATUS_EGDB_LOOKUP_HIT`           | `0x00000040`   | An EGDB lookup resulted in a hit (found a value).                    |
| 7                | `STATUS_EGDB_LOOKUP_MISS`          | `0x00000080`   | An EGDB lookup performed, but position not found in cache/file.      |
| 8                | `STATUS_EGDB_UNEXPECTED_VALUE`     | `0x00000100`   | EGDB returned an unexpected numerical value (e.g., raw 3 for WLD).   |
| 9                | `STATUS_EGDB_LOOKUP_OUT_OF_BOUNDS` | `0x00000200`   | EGDB lookup failed due to piece count being out of database bounds.  |
| 10               | `STATUS_EGDB_LOOKUP_NOT_PRESENT`   | `0x00000400`   | EGDB lookup failed because the specific sub-database was not loaded. |
| 11               | `STATUS_EGDB_LOOKUP_INVALID_INDEX` | `0x00000800`   | EGDB lookup failed due to an invalid index calculation.              |
| 12               | `STATUS_EGDB_SINGLE_VALUE_HIT`     | `0x00001000`   | EGDB lookup returned a pre-calculated single value (e.g., always draw). |
| 13               | `STATUS_EGDB_WIN_RESULT`           | `0x00002000`   | EGDB lookup explicitly resulted in a Win for the side to move.       |
| 14               | `STATUS_EGDB_LOSS_RESULT`          | `0x00004000`   | EGDB lookup explicitly resulted in a Loss for the side to move.      |
| 15               | `STATUS_EGDB_DRAW_RESULT`          | `0x00008000`   | EGDB lookup explicitly resulted in a Draw.                           |
| 16               | `STATUS_EGDB_UNKNOWN_RESULT`       | `0x00010000`   | EGDB lookup finished, but the result is still unknown/unclassified.  |
| 17               | `STATUS_EGDB_DISK_READ_ERROR`      | `0x00020000`   | Error reading a disk block for EGDB data.                            |
| 18               | `STATUS_EGDB_DECODE_ERROR`         | `0x00040000`   | Error during EGDB data decoding.                                     |
| 19               | `STATUS_APP_START`                 | `0x00080000`   | Application has started.                                             |