--TEST--
Spans are automatically flushed when auto-flushing enabled
--ENV--
DD_TRACE_AUTO_FLUSH_ENABLED=1
DD_TRACE_TRACED_INTERNAL_FUNCTIONS=array_sum
--FILE--
<?php
use DDTrace\SpanData;

require __DIR__ . '/../includes/fake_tracer.inc';
require __DIR__ . '/../includes/fake_global_tracer.inc';

DDTrace\trace_function('array_sum', function (SpanData $span, $args, $retval) {
    $span->name = 'array_sum';
    $span->resource = $retval;
});

DDTrace\trace_function('main', function (SpanData $span) {
    $span->name = 'main';
});

function main($max) {
    echo array_sum(range(0, $max)) . PHP_EOL;
    echo array_sum(range(0, $max + 1)) . PHP_EOL;
}

main(2);
echo PHP_EOL;
main(4);
echo PHP_EOL;
main(6);
echo PHP_EOL;
?>
--EXPECT--
3
6
Flushing tracer...
main (main)
array_sum (6)
array_sum (3)
Tracer reset

10
15
Flushing tracer...
main (main)
array_sum (15)
array_sum (10)
Tracer reset

21
28
Flushing tracer...
main (main)
array_sum (28)
array_sum (21)
Tracer reset
