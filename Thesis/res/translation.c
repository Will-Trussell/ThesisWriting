/* append trailing NUL byte to array, but do not count it. */
bool array_cat0_temporary(array * nonnull a) {
  char * nonnull endpos = (char * nonnull) array_alloc(a, 1, array_bytes(a));
  *endpos = '\0';
  return true;
}

_Bool  array_cat0_temporary(array  * a) {
  ;
  {
    char  *endpos = ((char *)({
      void  *_tmp10 = ((array_alloc)((a), 1, ((((a))->used))));
      ({
        if (((_tmp10) == 0)) {
          ((fprintf)((stderr),
            "array.xc:194:25:ERROR: attempted cast of NULL to nonnull\\n"));
          ((exit)(255));
        } else {
          ;
        }
        (_tmp10); })
        ; })
      );
      if ((!(endpos)))
      {
        return 0;
      } else {
        ;
      }
      ((*(endpos)) = '\0');
      return 1;
    }
}
