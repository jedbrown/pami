// 4way operation on ints
// split bytes == 256*m + rem
// num_ints ==  64*m + rem

#ifdef OP
#ifdef  DTYPE
  register DTYPE  r0, r1, r2, r3, r4, r5, r6, r7;
  register DTYPE res1=0, res2=0, res3=0, res4=0;
  register unsigned n,i,j,k,l;

  register unsigned m  = num_ints >> 6;
  i = 0;                     //first stream beginning
  j = (m << 4);              //second stream start from 1st quarter
  k = (m << 5);              //third stream start from half
  l = (m << 6) -  (m << 4);  //fourth stream starts from 3rd quarter

  if (m)
  { 
    r0  = src0[i];
    r1  = src0[j];
    r2  = src0[k];
    r3  = src0[l];
    r4  = src1[i];
    r5  = src1[j];
    r6  = src1[k];
    r7  = src1[l];
    i++;
    j++;
    k++;
    l++;

    for (n=4; n < (m << 6); n+=4,i++,j++,k++,l++)
    {
      res1 = OP(r0, res1);
      r0  = src0[i];

      res2 = OP(r1, res2);
      r1  = src0[j];

      res3 = OP(r2, res3);
      r2  = src0[k];

      res4 = OP(r3, res4);
      r3  = src0[l];

      res1 = OP(r4, res1);
      r4  = src1[i];

      res2 = OP(r5, res2);
      r5  = src1[j];

      res3 = OP(r6, res3);
      r6  = src1[i];

      res4 = OP(r7, res4);
      r7  = src1[j];

      dst[i-1]  = res1;
      dst[j-1]  = res2;
      dst[k-1]  = res3;
      dst[l-1]  = res4;
      res1 = res2 = res3  = res4  = 0;
    }
  }

  //remainder of the ints
  for (n=(m<<6); n < num_ints; n++)
  {
    dst[n]  = OP(src0[n], src1[n]);
  }
#endif	/* DTYPE */
#endif	/* OP */
