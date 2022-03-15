#include <SDL_stdinc.h>

typedef struct Ringbuf {
  Uint32 *data;
  Uint32 length;
  Uint32 *head;
  Uint32 *tail;
  Uint64 times_appended;
  Uint64 times_popped;
} Ringbuf;

Ringbuf *
Ringbuf_alloc(Uint32 length)
{
  Ringbuf *buf = malloc(sizeof(Ringbuf));
  buf->data = calloc(length, sizeof(Uint32));
  buf->length = length;
  buf->head = buf->data;
  buf->tail = buf->data;
  buf->times_appended = 0;
  buf->times_popped = 0;
  return buf;
}

void
Ringbuf_free(Ringbuf *buf)
{
  free(buf->data);
  free(buf);
}

void
Ringbuf_append(Ringbuf *buf, Uint32 x)
{
  *buf->head++ = x;
  buf->times_appended += 1;
  if (buf->head > buf->data + buf->length - 1) {
    buf->head = buf->data;
  }
  if (buf->times_appended > buf->times_popped + buf->length) {
    printf("Ringbuf Error: appended too many times\n");
  }
}

Uint32
Ringbuf_pop(Ringbuf *buf)
{
  Uint32 ret = *buf->tail++;
  buf->times_popped += 1;
  if (buf->tail > buf->data + buf->length - 1) {
    buf->tail = buf->data;
  }
  if (buf->times_popped > buf->times_appended) {
    printf("Ringbuf Error: popped more times than appended\n");
  }
  return ret;
}

Uint32
Ringbuf_head(Ringbuf *buf)
{
  if (buf->times_appended == 0) {
    printf("Ringbuf Error: buffer is empty\n");
  } else if (buf->head == buf->data) {
    // buffer just wrapped around
    return *(buf->data + buf->length - 1);
  }
  return *(buf->head - 1);
}

Uint32
Ringbuf_tail(Ringbuf *buf)
{
  return *buf->tail;
}

void
Ringbuf_foreach(Ringbuf *buf, int fn(Uint32))
{
  Uint32 *curr = buf->head;
  while (1) {
    if (curr == buf->tail)
      break;
    if (fn(*curr++) == 1)
      break;
    if (curr > buf->data + buf->length) {
      curr = buf->data;
    }
  }
}

// iterates two ringbuffers in parallel
void
Ringbuf_foreach2(Ringbuf *buf1, Ringbuf *buf2, int fn(Uint32, Uint32))
{
  Uint32 *curr1 = buf1->tail;
  Uint32 *curr2 = buf2->tail;
  while (1) {
    if (curr1 == buf1->head)
      break;
    if (curr2 == buf2->head)
      break;
    if (fn(*curr1, *curr2) == 1)
      break;
    curr1++;
    curr2++;
    if (curr1 >= buf1->data + buf1->length) {
      curr1 = buf1->data;
    }
    if (curr2 >= buf2->data + buf2->length) {
      curr2 = buf2->data;
    }
  }
}
