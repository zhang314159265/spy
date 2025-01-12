#if STRINGLIB_SIZEOF_CHAR == 1

Py_UCS4
STRINGLIB(find_max_char)(const STRINGLIB_CHAR *begin, const STRINGLIB_CHAR *end) {
  const unsigned char *p = (const unsigned char *) begin;

  while (p < end) {
    if (*p++ & 0x80)
      return 255;
  }
  return 127;
}

#else
#error STRINGLIB_SIZEOF_CHAR != 1
#endif
