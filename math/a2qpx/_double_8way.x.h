// 8way operation on doubles

#ifdef OP
register double f0, f1, f2, f3, f4, f5, f6, f7;
register double res = 0.0;
register  unsigned  n;

  f0 = src0[0];
  f1 = src1[0];
  f2 = src2[0];
  f3 = src3[0];
  f4 = src4[0];
  f5 = src5[0];
  f6 = src6[0];
  f7 = src7[0];

  for (n =1; n < num_dbls; n++)
  {
    res = OP(f0, f1);

    f0  = src0[n];
    f1  = src1[n];

    res = OP(f2, res);
    f2  = src2[n];

    res = OP(f3, res);
    f3  = src3[n];

    res = OP(f4, res);
    f4  = src4[n];

    res = OP(f5, res);
    f5  = src5[n];

    res = OP(f6, res);
    f6  = src6[n];

    res = OP(f7, res);
    f7  = src7[n];
  
    dst[n-1]  = res;
    res = 0.0;
  }

  res = OP(f0, f1);
  res = OP(f2, res);
  res = OP(f3, res);
  res = OP(f4, res);
  res = OP(f5, res);
  res = OP(f6, res);
  res = OP(f7, res);
  dst[n-1]  = res;
#endif	/* OP */
