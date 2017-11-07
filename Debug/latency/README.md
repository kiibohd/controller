# Latency Measurement Module

This module tracks the latency between two specific points.
Generally used for repeated functions such as scan loops.
Each latency must have an allocated resource to it.

Instead of keeping a list of past latencies, a sort of moving average (not actually, but something that's fast to calculate) is used giving higher weighting to the most recent values (exponentially).

This information is also available using the `latency` cli command from the debug shell.


## Initialization

```c
#include <latency.h>
```

```c
uint8_t resource_index = Latency_add_resource("MyMeasurement");
```


## Tracking Latency

Starting time.
```c
Latency_start_time( resource_index );
```

Ending time + average calculation.
```c
Latency_end_time( resource_index );
```


## Querying

Querying latency information.
```c
uint32_t min = Latency_query( LatencyQuery_Min, resource_index );
uint32_t max = Latency_query( LatencyQuery_Max, resource_index );
uint32_t average = Latency_query( LatencyQuery_Average, resource_index );
uint32_t last = Latency_query( LatencyQuery_Last, resource_index );
```

