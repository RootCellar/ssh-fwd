# NOTES
---

Send data from src to dst socket:

```
r = read(src, buf, 1024 * 4);

    while (r > 0) {
        i = 0;

        while (i < r) {
            j = write(dst, buf + i, r - i);

            if (j == -1) {
                DIE("write"); // TODO is errno EPIPE
            }

            i += j;
        }

        r = read(src, buf, 1024 * 4);
    }
```
