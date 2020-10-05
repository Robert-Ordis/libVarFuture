#include "./include/varfuture/private/body.h"
#include "./include/varfuture/varfuture_attr.h"
#include "./include/varfuture/primitives.h"
#include "./include/varfuture/type_future_implements.h"
#include "./include/varfuture/private/engine/concurrent_api.h"
#include "./include/varfuture/private/queue_api.h"
#include "./include/varfuture/private/inline_processes.h"
#include <errno.h>
#include <stdlib.h>


varfuture_define_implements(int, int, 0);

varfuture_define_implements(double, double, 0.0);
