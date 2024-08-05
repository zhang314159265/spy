#pragma once

// The PyThreadState typedef is in Include/pystate.h
struct _ts {
  PyInterpreterState *interp;
};
