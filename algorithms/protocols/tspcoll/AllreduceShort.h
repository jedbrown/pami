/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/tspcoll/AllreduceShort.h
 * \brief ???
 */

class ShortAllreduce
{
};


ShortAllreduce::ShortAllreduce ()
{
  /* --------------------------------------------------- */
  /*  set source and destination buffers and node ids    */
  /* We deal with non-power-of-2 nodes                   */
  /* --------------------------------------------------- */

  int nonBF, maxBF, logMaxBF, logN;
  //  logN = -1; for (int n=2*_comm->size()-1; n>0; n>>=1) logN++;
  for (logMaxBF = 0; (1<<(logMaxBF+1)) <= _comm->size(); logMaxBF++) ;
  maxBF = 1<<logMaxBF;   /* largest power of 2 that fits into comm->size() */
  nonBF = _comm->size() - maxBF;      /* comm->size() - largest power of 2 */

  int phase=0;

  /* phase 0: gather buffers from ranks > n2prev */

  if (nonBF > 0)
    {
      unsigned rdest = _comm->absrankof (_comm->rank() - maxBF);
      _dest    [phase] = (_comm->rank() >= maxBF) ? rdest : -1;
      _sbuf    [phase] = (_comm->rank() >= maxBF) ? dbuf  : NULL;
      _rbuf    [phase] = (_comm->rank() < nonBF)  ? _phasebuf[phase] : NULL;
      _cb_recv [phase] = (_comm->rank() < nonBF)  ? cb_allreduce : NULL;
      _sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  for (int i=0; i<logMaxBF; i++)   /* middle phases: butterfly pattern */
    {
      unsigned rdest   = _comm->absrankof (_comm->rank() ^ (1<<i));
      _dest    [phase] = (_comm->rank() < maxBF) ? rdest : -1;
      _sbuf    [phase] = (_comm->rank() < maxBF) ? dbuf  : NULL;
      _rbuf    [phase] = (_comm->rank() < maxBF) ? _phasebuf[phase] : NULL;
      _cb_recv [phase] = (_comm->rank() < maxBF) ? cb_allreduce : NULL;
      _sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  if (nonBF > 0)   /*  last phase: collect results */
    {
      unsigned rdest   = _comm->absrankof (_comm->rank() + maxBF);
      _dest    [phase] = (_comm->rank() < nonBF)  ? rdest : -1;
      _sbuf    [phase] = (_comm->rank() < nonBF)  ? dbuf  : NULL;
      _rbuf    [phase] = (_comm->rank() >= maxBF) ? dbuf  : NULL;
      _cb_recv [phase] = NULL;
      _sbufln  [phase] = nelems * datawidth;
      phase ++;
    }

  for (; phase < _numphases; phase++)
    {
      _dest    [phase] = -1;
      _sbuf    [phase] = NULL;
      _rbuf    [phase] = NULL;
      _cb_recv [phase] = NULL;
      _sbufln  [phase] = 0;
    }

  _numphases = 2*(logMaxBF + 2 * (nonBF > 0));


}
