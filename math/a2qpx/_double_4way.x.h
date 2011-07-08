// 4way operation on doubles
// split bytes == 128*m + rem
// num_dbls = 16*m + rem

#ifdef OP
  register double f0, f1, f2, f3, f4, f5, f6, f7;
  register double res1=0.0, res2=0.0;
  register unsigned n,i,j;

  register unsigned m  = num_dbls >> 4;
  i = 0;          //first stream beginning
  j = (m << 3);   //second stream start from half-way 

  if (m)
  { 
    f0  = src0[i];
    f1  = src0[j];
    f2  = src1[i];
    f3  = src1[j];
    f4  = src2[i];
    f5  = src2[j];
    f6  = src3[i];
    f7  = src3[j];

    i++;
    j++;
    for (n=2; n < (m << 4); n+=2,i++,j++)
    {
      res1 = OP(f0, res1);
      f0  = src0[i];

      res2 = OP(f1, res2);
      f1  = src0[j];

      res1 = OP(f2, res1);
      f2  = src1[i];

      res2 = OP(f3, res2);
      f3  = src1[j];

      res1 = OP(f0, res1);
      f4  = src2[i];

      res2 = OP(f1, res2);
      f5  = src2[j];

      res1 = OP(f0, res1);
      f6  = src3[i];

      res2 = OP(f1, res2);
      f7  = src3[j];

      dst[i-1]  = res1;
      dst[j-1]  = res2;
      res1 = res2 = 0.0;
    }
  }

  //remainder of the doubles
  for (n=(m<<4); n < num_dbls; n++)
  {
    res1  = OP(src0[n], src1[n]);
    res1  = OP(res1, src2[n]);
    res1  = OP(res1, src3[n]);
    dst[n]  = res1;
  }
#endif	/* OP */
