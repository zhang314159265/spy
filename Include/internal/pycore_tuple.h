#pragma once

#include "tupleobject.h"

#define _PyTuple_ITEMS(op) (_PyTuple_CAST(op)->ob_item)
