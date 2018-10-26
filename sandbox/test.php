<?php
//
// Include the GeSHi library//
include_once '../geshi/geshi.php'; 
//// Define some source to highlight, a language to use
// and the path to the language files//
 $source = '  $foo = 45;
for ( $i = 1; $i < $foo; $i++ ){
  echo "$foo\n";  --$foo;
}';$language = 'php';
 //
// Create a GeSHi object//
 $geshi = new GeSHi($source, $language);
 $geshi->set_header_type(GESHI_HEADER_NONE);
 //
// And echo the result!//
echo $geshi->parse_code();
