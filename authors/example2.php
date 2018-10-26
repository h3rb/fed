<?php

/*
 * Example that shows how to process incoming data
 * and then export it back to fed with multiple options
 */

$data=base64_decode($INCOMING['data']);
$x=$INCOMING['row'];
$y=$INCOMING['col'];

$results=array();
$results[0]=str_replace( "A", "B", $data );
$results[1]=str_replace( "C", "D", $data );
$option_count=count($results);

$options="
1) Change As to Bs:1:0
2) Change Cs to Ds:2:0
";

echo base64_encode($options);
echo '--fedboundary--';
foreach ( $results as $result )
 echo base64_encode( $result )
    . ( $option_count-1 != $i ? '--fedboundary--' : '' );
