// 4way operation on ints
// split num_bytes == 128*m + rem
// num_ints = 32*m + rem

#ifdef OP
  register int r0, r1, r2, r3, r4, r5, r6, r7;
  register int res1=0, res2=0;
  register unsigned n,i,j;

  register unsigned m  = num_ints >> 5;
  i = 0;          //first stream beginning
  j = (m << 4);   //second stream start from half-way 

  if (m)
  { 
    r0  = src0[i];
    r1  = src0[j];
    r2  = src1[i];
    r3  = src1[j];
    r4  = src2[i];
    r5  = src2[j];
    r6  = src3[i];
    r7  = src3[j];

    i++;
    j++;
    for (n=2; n < (m << 5); n+=2,i++,j++)
    {
      res1 = OP(r0, res1);
      r0  = src0[i];

      res2 = OP(r1, res2);
      r1  = src0[j];

      res1 = OP(r2, res1);
      r2  = src1[i];

      res2 = OP(r3, res2);
      r3  = src1[j];

      res1 = OP(r0, res1);
      r4  = src2[i];

      res2 = OP(r1, res2);
      r5  = src2[j];

      res1 = OP(r0, res1);
      r6  = src3[i];

      res2 = OP(r1, res2);
      r7  = src3[j];

      dst[i-1]  = res1;
      dst[j-1]  = res2;
      res1 = res2 = 0;
    }
  }

  //remainder of the ints
  for (n=(m<<5); n < num_ints; n++)
  {
    res1  = OP(src0[n], src1[n]);
    res1  = OP(res1, src2[n]);
    res1  = OP(res1, src3[n]);
    dst[n]  = res1;
  }
#endif	/* OP */
