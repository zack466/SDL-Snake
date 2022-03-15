#include <SDL_stdinc.h>

// typedef struct Ringbuf {
// } Ringbuf;
typedef struct Ringbuf {
  Uint32 *data;
  Uint32 length;
  Uint32 *head;
  Uint32 *tail;
  Uint64 times_appended;
  Uint64 times_popped;
} Ringbuf;

Ringbuf *Ringbuf_alloc(Uint32 length);

void Ringbuf_free(Ringbuf *buf);

void Ringbuf_append(Ringbuf *buf, Uint32 x);

Uint32 Ringbuf_pop(Ringbuf *buf);

Uint32 Ringbuf_head(Ringbuf *buf);

Uint32 Ringbuf_tail(Ringbuf *buf);

void Ringbuf_foreach(Ringbuf *buf, int fn(Uint32));

void Ringbuf_foreach2(Ringbuf *buf1, Ringbuf *buf2, int fn(Uint32, Uint32));
