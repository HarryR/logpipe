<?php
$stdin = fopen('php://stdin', 'r');
$stderr = fopen('php://stderr', 'w');
while( ($line = fgets($stdin)) ) {
	$x = json_decode($line);
	if( ! $x ) {
		fwrite($stderr, $line);
	}
}