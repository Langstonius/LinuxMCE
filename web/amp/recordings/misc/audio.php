<?php

/**
 * @file
 * plays recording file
 */

if (isset($_GET['recording'])) {

  $path = $_GET['recording'];
  
  // See if the file exists
  if (!is_file($path)) { die("<b>404 File not found!</b>"); }

  // Gather relevent info about file
  $size = filesize($path);
  $name = basename($path);
  $extension = strtolower(substr(strrchr($name,"."),1));

  // This will set the Content-Type to the appropriate setting for the file
  switch( $extension ) {
    case "mp3": $ctype="audio/mpeg"; break;
    case "wav": $ctype="audio/x-wav"; break;

    // not downloadable
    case "php":
    case "htm":
    case "html":
    case "txt": die("<b>Cannot be used for ". $file_extension ." files!</b>"); break;

    default: $ctype="application/force-download";
  }

// need to check if file is mis labeled or a liar.

  $fp=fopen($path, "rb");
  if ($size && $fp) {
    header("Pragma: public");
    header("Expires: 0");
    header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
    header("Cache-Control: public");
    header("Content-Description: wav file");
    header("Content-Type: " . $ctype);
    header("Content-Disposition: attachment; filename=" . $name);
    header("Content-Transfer-Encoding: binary");
    header("Content-length: " . $size);
    fpassthru($fp);
  } 
}

?>