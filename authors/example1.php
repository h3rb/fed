<?php

/*
 * Example that shows how to process incoming data
 * and then export it back to fed
 */

$data=base64_decode($INCOMING['data']);
$x=$INCOMING['row'];
$y=$INCOMING['col'];

$data=str_replace( "A", "B", $data);

$options="";

echo base64_encode($options);
echo '--fedboundary--';
echo base64_encode( $data );
