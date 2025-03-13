<?php

include 'operations/mediaBrowser/httpImageProvider.php';

/*
$file =$_GET['file'];
$prop =$_GET['prop'];

$filemarker = substr_replace($file, "", 0, 2);

# $ref is the Picture_Attribute.FK_Picture value
$ref="";

#magic sql to determine ref go here
$sql = "SELECT File_Attribute.FK_File, File_Attribute.FK_Attribute, Attribute.PK_Attribute, Attribute.FK_AttributeType, Picture_Attribute.FK_Attribute, Picture_Attribute.FK_Picture FROM Picture_Attribute, Attribute, File_Attribute WHERE File_Attribute.FK_File = \"$filemarker\" AND Attribute.FK_AttributeType=\"$prop\" AND File_Attribute.FK_Attribute = Attribute.PK_Attribute AND Attribute.PK_Attribute = Picture_Attribute.FK_Attribute";

// Connect to database
$conn = mysqli_connect("localhost", "root", "", "pluto_media");
if (!$conn) {
    die("Connection failed: " . mysqli_connect_error());
}

$result = mysqli_query($conn, $sql) or die(mysqli_error($conn)); 
$row = mysqli_fetch_assoc($result);
$ref.=$row['FK_Picture'];

$img =file_get_contents("mediapics/".$ref.".jpg");
echo $img;
*/

?>