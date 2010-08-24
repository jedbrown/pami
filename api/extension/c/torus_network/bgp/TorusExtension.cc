
#include "api/extension/c/torus_network/TorusExtension.h"

#include "Global.h"

PAMI::TorusExtension::torus_info_t PAMI::TorusExtension::info;
size_t PAMI::TorusExtension::coords[1024];


PAMI::TorusExtension::TorusExtension ()
{
  size_t * ptr = coords;

  info.dims = 4;
  info.coord = ptr;
  ptr += info.dims;
  info.size  = ptr;
  ptr += info.dims;
  info.torus = ptr;
  ptr += info.dims;

  info.size[0] = __global.personality.xSize();
  info.size[1] = __global.personality.ySize();
  info.size[2] = __global.personality.zSize();
  info.size[3] = __global.personality.tSize();

  info.coord[0] = __global.personality.xCoord();
  info.coord[1] = __global.personality.yCoord();
  info.coord[2] = __global.personality.zCoord();
  info.coord[3] = __global.personality.tCoord();

  info.torus[0] = __global.personality.isTorusX();
  info.torus[1] = __global.personality.isTorusY();
  info.torus[2] = __global.personality.isTorusZ();
  info.torus[3] = 1; // t coordinate is always a torus ?
}

const PAMI::TorusExtension::torus_info_t * PAMI::TorusExtension::information ()
{
  return & PAMI::TorusExtension::info;
};

pami_result_t PAMI::TorusExtension::task2torus (pami_task_t task, size_t addr[])
{
  array_t<size_t, 4> * resized = (array_t<size_t, 4> *) addr;
  return __global.mapping.task2torus_impl ((size_t) task, resized->array);
};

pami_result_t PAMI::TorusExtension::torus2task (size_t addr[], pami_task_t * task)
{
  size_t t = (size_t) - 1;
  array_t<size_t, 4> * resized = (array_t<size_t, 4> *) addr;
  pami_result_t result =
    __global.mapping.torus2task_impl (resized->array, t);
  *task = t;

  return result;
};



